
#include "debug.h"
#include "x64emu.h"
#include "x64instr.h"
#include "x64modrm.h"
#include "x64regs_private.h"

SET_DEBUG_CHANNEL("X64MODRM")

void x64modrm_fetch(x64emu_t *emu, x64instr_t *ins) {
    ins->modrm.byte = fetch_8();

    /* 11 - Register-direct addressing mode. */
    if (ins->modrm.mod == 3) return;

    if (ins->modrm.rm == 4)
        ins->sib.byte = fetch_8();

    switch (ins->modrm.mod) {
        case 0x0: /* 00 */
            if (ins->modrm.rm == 5 || (ins->modrm.rm == 4 && ins->sib.base == 5))
                ins->displ.dword[0] = fetch_32();
            break;
        case 0x1: /* 01 */
            ins->displ.byte[0] = fetch_8();
            break;
        case 0x2: /* 10 */
            ins->displ.dword[0] = fetch_32();
            break;
    }
}

/* https://wiki.osdev.org/X86-64_Instruction_Encoding#32/64-bit_addressing_2 */

/** Index register value or 0 for X.Index 0.100. */
static inline uint64_t x64sib_get_index(x64emu_t *emu, x64instr_t *ins) {
    return (!ins->rex.x && ins->sib.index == 4) ? 0 :
            r_reg64(ins->sib.index | (ins->rex.x << 3));
}

/** Base register value. */
static inline uint64_t x64sib_get_base(x64emu_t *emu, x64instr_t *ins) {
    return r_reg64(ins->sib.base | (ins->rex.b << 3));
}

/** The SIB address (mod 00). */
static inline uint64_t x64sib_calculate00(x64emu_t *emu, x64instr_t *ins) {
    return (x64sib_get_index(emu, ins) << ins->sib.scale) +
           ((ins->sib.base == 5) ? ins->displ.sdword[0] :
            x64sib_get_base(emu, ins));
}

/** The SIB address (mod 01 or 10). */
static inline uint64_t x64sib_calculate01(x64emu_t *emu, x64instr_t *ins) {
    /* displacement is added later. */
    return (x64sib_get_index(emu, ins) << ins->sib.scale) + x64sib_get_base(emu, ins);
}

void *x64modrm_get_reg(x64emu_t *emu, x64instr_t *ins) {
    return emu->regs + (ins->modrm.reg | (ins->rex.r << 3));
}

static inline uint64_t get_raw_rm(x64emu_t *emu, x64instr_t *ins) {
    return r_reg64(ins->modrm.rm | (ins->rex.b << 3));
}

void *x64modrm_get_rm(x64emu_t *emu, x64instr_t *ins) {
    if (ins->address_sz) {
        log_err("32 bit addressing ModR/M still not covered.");
        return NULL;
    }

    /* https://wiki.osdev.org/X86-64_Instruction_Encoding#32/64-bit_addressing */

    switch (ins->modrm.mod) {
        case 0x0:     /* 00 */
            if (ins->modrm.rm == 5)           /* [RIP/EIP + disp32] */
                return (void *)(r_rip + ins->displ.sdword[0]);
            else if (ins->modrm.rm == 4)      /* [SIB] */
                return (void *)x64sib_calculate00(emu, ins);
            else                              /* [r/m] */
                return (void *)get_raw_rm(emu, ins);

        case 0x1:     /* 01 */                /* [(r/m or SIB) + disp8] */
            return (void *)(((ins->modrm.rm == 4) ? x64sib_calculate01(emu, ins) :
                    get_raw_rm(emu, ins)) + ins->displ.sbyte[0]);

        case 0x2:     /* 10 */                /* [(r/m or SIB) + disp32] */
            return (void *)(((ins->modrm.rm == 4) ? x64sib_calculate01(emu, ins) :
                    get_raw_rm(emu, ins)) + ins->displ.sdword[0]);

        case 0x3:     /* 11 */                /* r/m */
            return emu->regs + (ins->modrm.rm | (ins->rex.b << 3));
    }
    return NULL;
}
