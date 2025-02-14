#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <linux/elf.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>

#include "elfloader.h"
#include "debug.h"
#include "x64context.h"

SET_DEBUG_CHANNEL("ELFLOADER")

/**
 * @return Error description or `NULL` if checks passed.
 */
static char *check_elf_header(Elf64_Ehdr *ehdr) {
    if (!ehdr) return "Meh";

    /* Check the magic number */
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0)
        return "Not a valid ELF file";

    if (ehdr->e_ident[EI_CLASS] != ELFCLASS64)
        return "Not a 64 bit elf";

    if (ehdr->e_ident[EI_DATA] != ELFDATA2LSB)
        return "Unsupported data encoding byte index, can only support little endian for now.";

    // if (ehdr->e_ident[EI_OSABI] != ELFOSABI_GNU &&
    //     ehdr->e_ident[EI_OSABI] != ELFOSABI_NONE)
    //     return "Unsupported OS ABI";

    if (ehdr->e_ident[EI_VERSION] != EV_CURRENT)
        return "Not a valid ELF ident version";

    // if (ehdr->e_type != )
    //     return "Not a valid ELF type";

    if (ehdr->e_machine != EM_X86_64)
        return "Not a x86_64 ELF";

    // if (ehdr->e_version != EV_CURRENT)
    //     return "Not a valid ELF version";

    log_dump("ph: num: %d, size: %d, off: 0x%llx, "
             "sh: num: %d, size: %d, off: 0x%llx, entry: 0x%llx",
             ehdr->e_phnum, ehdr->e_phentsize, ehdr->e_phoff,
             ehdr->e_shnum, ehdr->e_shentsize, ehdr->e_shoff, ehdr->e_entry);

    return NULL;
}

/**
 * @note Never closes fd.
 */
static bool read_elf_header(FILE *fd, Elf64_Ehdr *ehdr) {
    if (!fd) return false;

    if (fread(ehdr, sizeof(Elf64_Ehdr), 1, fd) != 1) {
        log_err("Failed to read ELF header");
        return false;
    }

    char *err = check_elf_header(ehdr);
    if (err) {
        log_err("%s", err);
        return false;
    }

    return true;
}

/**
 * Convert program header flags to page protection
 */
static inline int p_flags_to_prot(Elf64_Word p_flags) {
    return ((p_flags & PF_X) ? PROT_EXEC : 0) | ((p_flags & PF_R) ? PROT_READ : 0) | ((p_flags & PF_W) ? PROT_WRITE : 0);
}

/**
 * Check whether this a loadable segment, with non-zero size and valid flags.
 */
static inline bool need_to_load(Elf64_Phdr *phdr) {
    return phdr && phdr->p_type == PT_LOAD && phdr->p_memsz != 0 && (phdr->p_flags & 7U);
}

/**
 * @note Never closes fd.
 */
static bool load_program_headers(x64context_t *ctx, FILE *fd, Elf64_Ehdr *ehdr) {
    if (!fd || !ctx || !ehdr) return false;

    Elf64_Phdr phdrs[ehdr->e_phnum];

    if (fseek(fd, ehdr->e_phoff, SEEK_SET) == -1 ||
        fread(phdrs, ehdr->e_phentsize, ehdr->e_phnum, fd) != ehdr->e_phnum)
    {
        log_err("Failed to read program headers");
        return false;
    }

    if (ctx->segments_len) {
        log_err("Tried to load binary segments, but segments_len is already not 0");
        return false;
    }

    /* first count segments that we are going to load */
    for (Elf64_Half i = 0; i < ehdr->e_phnum; i++)
        if (need_to_load(phdrs + i))
            ctx->segments_len++;

    log_dump("Mapping %d segments", ctx->segments_len);

    ctx->segments = calloc(ctx->segments_len, sizeof(segment_t));

    uint32_t seg_idx = 0;

    for (Elf64_Half i = 0; i < ehdr->e_phnum; i++) {
        Elf64_Phdr *ph = phdrs + i;
        log_dump("Program header: type: 0x%x, flags: 0x%x, offset: 0x%llx, vaddr: 0x%llx, "
                 "paddr: 0x%llx\nfilesz: 0x%llx, memsz: 0x%llx, align: 0x%llx",
                 ph->p_type, ph->p_flags, ph->p_offset, ph->p_vaddr, ph->p_paddr,
                 ph->p_filesz, ph->p_memsz, ph->p_align);

        if (need_to_load(ph)) {
            /* FIXME: workaround would be simple, but not implemented yet... */
            if (ph->p_align & (ctx->page_size - 1)) {
                log_err("Mapping segment with 0x%llx alignment on 0x%lx page size is currently unsupported", ph->p_align, ctx->page_size);
                return false;
            }

            /* since segments must have congruent values for p_vaddr and p_offset, modulo the page size,
               we will load the segment directly at aligned offset to aligned address */

            Elf64_Addr  aligned_addr = ph->p_vaddr & ~(ph->p_align - 1);
            Elf64_Off   aligned_offs = ph->p_offset & ~(ph->p_align - 1);
            Elf64_Addr  align_offset = ph->p_vaddr - aligned_addr;
            Elf64_Xword aligned_size = ph->p_memsz + align_offset;

            int prot = p_flags_to_prot(ph->p_flags);

            log_dump("Mapping segment at 0x%llx, size 0x%llx, offset 0x%llx, prot 0x%x",
                        aligned_addr, aligned_size, aligned_offs, prot);

            void *ret = mmap((void *)aligned_addr, aligned_size,
                prot, MAP_PRIVATE | MAP_FIXED, fileno(fd), aligned_offs);

            if (ret == MAP_FAILED) {
                log_err("Failed to map segment: %s", strerror(errno));
                return false;
            }

            Elf64_Xword added = ph->p_memsz - ph->p_filesz;
            if (added) memset((void *)(ph->p_vaddr + ph->p_filesz), 0, added);

            ctx->segments[seg_idx].base = ret; /* used for unmapping segments */
            ctx->segments[seg_idx].size = aligned_size;
            seg_idx++;
        }
    }

    return true;
}

bool elfloader_load(x64context_t *ctx, char *path) {
    if (!ctx) return false;

    FILE *fd;

    if (!path || !(fd = fopen(path, "rb"))) {
        log_err("Failed to open elf binary");
        return false;
    }

    Elf64_Ehdr ehdr;
    if (!read_elf_header(fd, &ehdr)) {
        fclose(fd);
        return false;
    }

    if (!load_program_headers(ctx, fd, &ehdr)) {
        fclose(fd);
        return false;
    }

    ctx->entry = ehdr.e_entry; /* save entry point. */

    fclose(fd);
    return true;
}
