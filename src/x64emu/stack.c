#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/auxv.h>
#include <stdint.h>

#include "debug.h"
#include "x64context.h"
#include "x64emu.h"
#include "x64stack.h"
#include "x64regs_private.h"
#include "virtual.h"

SET_DEBUG_CHANNEL("X64STACK");

/* stack operations */

void push_16(x64emu_t *emu, uint16_t v) {
    *(uint16_t *)(r_rsp -= 2) = v;
}

void push_32(x64emu_t *emu, uint32_t v) {
    *(uint32_t *)(r_rsp -= 4) = v;
}

void push_64(x64emu_t *emu, uint64_t v) {
    *(uint64_t *)(r_rsp -= 8) = v;
}

void push_auxv(x64emu_t *emu, uint64_t v, uint64_t t) {
    push_64(emu, v);
    push_64(emu, t);
}

void push_real_auxv(x64emu_t *emu, uint64_t t) {
    uint64_t v = getauxval(t);
    if (errno != ENOENT) push_auxv(emu, v, t);
}

void push_align(x64emu_t *emu) {
    uint64_t aligned = r_rsp & ~(emu->ctx->stack.align - 1);
    memset((void *)aligned, 0, r_rsp - aligned);
    r_rsp = aligned;
}

void push_string(x64emu_t *emu, const char *str) {
    int size = strlen(str) + 1; // NULL-terminated size
    r_rsp -= size;
    memcpy((void *)r_rsp, str, size);
}

uint16_t pop_16(x64emu_t *emu) {
    return *(uint16_t *)(r_rsp += 2, r_rsp - 2);
}

uint32_t pop_32(x64emu_t *emu) {
    return *(uint32_t *)(r_rsp += 4, r_rsp - 4);
}

uint64_t pop_64(x64emu_t *emu) {
    return *(uint64_t *)(r_rsp += 8, r_rsp - 8);
}

// address such that the native side has more than enough space above
// (so that overlaps and overflows do not happen).
static const uintptr_t stack_addr_48bit = 7UL << 44UL;
static const uintptr_t stack_addr_39bit = 7UL << 36UL;
/* 1 GB, try using big enough stack for entire program execution */
static const size_t    stack_init_size  = 1024UL * 1024UL * 1024UL * 1UL;

bool x64stack_init(x64context_t *ctx) {
    if (!ctx) return false;

    dump_self_maps();

    // try mapping stack at higher addresses
    uintptr_t addr_hint = detect_48bit_va() ? stack_addr_48bit : stack_addr_39bit;

    ctx->stack.size = stack_init_size;
    addr_hint -= ctx->stack.size; /* do not go above hint. */

    ctx->stack.base = mmap((void *)addr_hint, ctx->stack.size,
        PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (ctx->stack.base == MAP_FAILED) {
        log_err("Failed to map the initial stack: %s", strerror(errno));
        ctx->stack.size = 0;
        return false;
    }

    ctx->stack.align = 16;

    log_dump("Mapped initial stack at 0x%lx-0x%lx, with alignment 0x%lx",
             (uintptr_t)ctx->stack.base, (uintptr_t)ctx->stack.base + ctx->stack.size, ctx->stack.align);

    return true;
}

bool x64stack_free(x64context_t *ctx) {
    if (!ctx || !ctx->stack.size) return true;

    if (munmap(ctx->stack.base, ctx->stack.size) != 0) {
        log_err("Failed to unmap stack: %s", strerror(errno));
        return false;
    }
    ctx->stack.size = 0;
    return true;
}

void x64stack_setup(x64emu_t *emu) {
    if (!emu || !emu->ctx) return;
    x64context_t *ctx = emu->ctx;

    /* push strings and other data */

    uintptr_t p_argv[ctx->argc];
    uintptr_t p_envv[ctx->envc];

    for (int i = 0; i < ctx->argc; i++) {
        push_string(emu, ctx->argv[i]);
        p_argv[i] = r_rsp;
    }
    for (int i = 0; i < ctx->envc; i++) {
        push_string(emu, ctx->envv[i]);
        p_envv[i] = r_rsp;
    }

    push_string(emu, "x86_64");
    uintptr_t platform_string = r_rsp;

    /* 16 random bytes */
    for (int i = 0; i < 4; i++)
        push_32(emu, (uint32_t)random());
    uintptr_t random_16 = r_rsp;

    push_align(emu);

// List of types to consider
// AT_PAGESZ         6      /* System page size */
// AT_FLAGS          8      /* Flags */
// AT_ENTRY          9      /* Entry point of program */
// AT_UID           11      /* Real uid */
// AT_EUID          12      /* Effective uid */
// AT_GID           13      /* Real gid */
// AT_EGID          14      /* Effective gid */
// AT_PLATFORM      15      /* Platform.  */
// AT_HWCAP         16      /* Processor capabilities.  */
// AT_CLKTCK        17      /* Frequency of times() */
// AT_SECURE        23      /* Boolean, was exec setuid-like?  */
// AT_BASE_PLATFORM 24      /* String identifying real platforms.*/
// AT_RANDOM        25      /* Address of 16 random bytes.  */
// AT_HWCAP2        26      /* extension of AT_HWCAP.  */
// AT_HWCAP3        29      /* extension of AT_HWCAP.  */
// AT_HWCAP4        30      /* extension of AT_HWCAP.  */
// AT_EXECFN        31      /* Filename of executable.  */
// AT_SYSINFO       32      /* vSyscalls */
// AT_SYSINFO_EHDR  33      /* vDSO */

    /* push NULL-terminated auxv */

    push_auxv(emu, 0, 0);
    push_real_auxv(emu, AT_PAGESZ);
    push_real_auxv(emu, AT_FLAGS);
    push_auxv(emu, ctx->entry, AT_ENTRY);
    push_real_auxv(emu, AT_UID);
    push_real_auxv(emu, AT_EUID);
    push_real_auxv(emu, AT_GID);
    push_real_auxv(emu, AT_EGID);
    push_auxv(emu, platform_string, AT_PLATFORM);


    push_real_auxv(emu, AT_SECURE);

    push_auxv(emu, random_16, AT_RANDOM);



    push_auxv(emu, p_argv[0], AT_EXECFN);

    /* order should be in reverse, since it is a stack. */
    push_64(emu, 0);
    for (int i = ctx->envc - 1; i >= 0; i--) {
        push_64(emu, p_envv[i]);
    }

    push_64(emu, 0);
    for (int i = ctx->argc - 1; i >= 0; i--) {
        push_64(emu, p_argv[i]);
    }

    push_64(emu, ctx->argc);
}
