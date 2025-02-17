
#include <stdint.h>

#include "debug.h"
#include "x64emu.h"
#include "x64instr.h"
#include "x64modrm.h"
#include "x64regs_private.h"

SET_DEBUG_CHANNEL("X64MODRM")

void x64modrm_fetch(x64emu_t *emu, x64instr_t *ins) {
    ins->modrm.byte = fetch_8(emu, ins);

    /* 11 - Register-direct addressing mode. */
    if (ins->modrm.mod == 3) return;

    if (ins->modrm.rm == 4)
        ins->sib.byte = fetch_8(emu, ins);

    switch (ins->modrm.mod) {
        case 0x0: /* 00 */
            if (ins->modrm.rm == 5 || (ins->modrm.rm == 4 && ins->sib.base == 5))
                ins->displ.dword[0] = fetch_32(emu, ins);
            break;
        case 0x1: /* 01 */
            ins->displ.byte[0] = fetch_8(emu, ins);
            break;
        case 0x2: /* 10 */
            ins->displ.dword[0] = fetch_32(emu, ins);
            break;
    }
}

void *x64modrm_get_reg(x64emu_t *emu, x64instr_t *ins) {
    return emu->regs + (ins->modrm.reg | (ins->rex.r << 3));
}

/* https://wiki.osdev.org/X86-64_Instruction_Encoding#32/64-bit_addressing_2 */

static inline uint64_t sib_get_scaled_index(x64emu_t *emu, x64instr_t *ins) {
    return (!ins->rex.x && ins->sib.index == 4) ? 0 :
            (r_reg64(ins->sib.index | (ins->rex.x << 3)) << ins->sib.scale);
}

static inline uint64_t sib_get_base(x64emu_t *emu, x64instr_t *ins) {
    return r_reg64(ins->sib.base | (ins->rex.b << 3));
}

static inline uint64_t sib_get_00(x64emu_t *emu, x64instr_t *ins) {
    return sib_get_scaled_index(emu, ins) + ((ins->sib.base == 5) ? ins->displ.sdword[0] : sib_get_base(emu, ins));
}

static inline uint64_t sib_get_01_10(x64emu_t *emu, x64instr_t *ins) {
    return sib_get_scaled_index(emu, ins) + sib_get_base(emu, ins);
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
                return (void *)sib_get_00(emu, ins);
            else                              /* [r/m] */
                return (void *)get_raw_rm(emu, ins);

        case 0x1:     /* 01 */                /* [(r/m or SIB) + disp8] */
            return (void *)(((ins->modrm.rm == 4) ? sib_get_01_10(emu, ins) :
                    get_raw_rm(emu, ins)) + ins->displ.sbyte[0]);

        case 0x2:     /* 10 */                /* [(r/m or SIB) + disp32] */
            return (void *)(((ins->modrm.rm == 4) ? sib_get_01_10(emu, ins) :
                    get_raw_rm(emu, ins)) + ins->displ.sdword[0]);

        case 0x3:     /* 11 */                /* r/m */
            return emu->regs + (ins->modrm.rm | (ins->rex.b << 3));
    }
    return NULL;
}
