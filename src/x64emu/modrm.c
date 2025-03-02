#include <stdint.h>
#include <stdbool.h>

#include "debug.h"
#include "x64emu.h"
#include "x64instr.h"
#include "x64modrm.h"

#include "regs_private.h"

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
                ins->displ.ud[0] = fetch_32(emu, ins);
            break;
        case 0x1: /* 01 */
            ins->displ.ub[0] = fetch_8(emu, ins);
            break;
        case 0x2: /* 10 */
            ins->displ.ud[0] = fetch_32(emu, ins);
            break;
    }
}

void *x64modrm_get_reg(x64emu_t *emu, x64instr_t *ins) {
    return emu->regs + (ins->modrm.reg | (ins->rex.r << 3));
}

void *x64modrm_get_xmm(x64emu_t *emu, x64instr_t *ins) {
    return emu->xmm + (ins->modrm.reg | (ins->rex.r << 3));
}

void *x64modrm_get_mmx(x64emu_t *emu, x64instr_t *ins) {
    return emu->mmx + (ins->modrm.reg | (ins->rex.r << 3));
}

/* https://wiki.osdev.org/X86-64_Instruction_Encoding#32/64-bit_addressing_2 */

static inline uint64_t x64sib_get_scaled_index(x64emu_t *emu, x64instr_t *ins) {
    return (!ins->rex.x && ins->sib.index == 4) ? 0 :
            (emu->regs[ins->sib.index | (ins->rex.x << 3)].uq[0] << ins->sib.scale);
}

static inline uint64_t x64sib_get_base(x64emu_t *emu, x64instr_t *ins) {
    return emu->regs[ins->sib.base | (ins->rex.b << 3)].uq[0];
}

static inline uint64_t x64sib_get_00(x64emu_t *emu, x64instr_t *ins) {
    return x64sib_get_scaled_index(emu, ins) + ((ins->sib.base == 5) ? ins->displ.sd[0] : x64sib_get_base(emu, ins));
}

static inline uint64_t x64sib_get_01_10(x64emu_t *emu, x64instr_t *ins) {
    return x64sib_get_scaled_index(emu, ins) + x64sib_get_base(emu, ins);
}

static inline uint64_t x64modrm_get_base_rm(x64emu_t *emu, x64instr_t *ins) {
    return emu->regs[ins->modrm.rm | (ins->rex.b << 3)].uq[0];
}

void *x64modrm_get_indirect(x64emu_t *emu, x64instr_t *ins) {
    if (ins->address_sz) {
        log_err("32 bit addressing ModR/M still not covered.");
        return NULL;
    }

    /* https://wiki.osdev.org/X86-64_Instruction_Encoding#32/64-bit_addressing */

    switch (ins->modrm.mod) {
        case 0x0:     /* 00 */
            if (ins->modrm.rm == 5)           /* [RIP/EIP + disp32] */
                return (void *)(r_rip + ins->displ.sd[0]);
            else if (ins->modrm.rm == 4)      /* [SIB] */
                return (void *)x64sib_get_00(emu, ins);
            else                              /* [r/m] */
                return (void *)x64modrm_get_base_rm(emu, ins);

        case 0x1:     /* 01 */                /* [(r/m or SIB) + disp8] */
            return (void *)(((ins->modrm.rm == 4) ? x64sib_get_01_10(emu, ins) :
                x64modrm_get_base_rm(emu, ins)) + ins->displ.sb[0]);

        case 0x2:     /* 10 */                /* [(r/m or SIB) + disp32] */
            return (void *)(((ins->modrm.rm == 4) ? x64sib_get_01_10(emu, ins) :
                x64modrm_get_base_rm(emu, ins)) + ins->displ.sd[0]);
    }
    return NULL;
}

void *x64modrm_get_r_m(x64emu_t *emu, x64instr_t *ins) {
    if (ins->modrm.mod == 3)
        return emu->regs + (ins->modrm.rm | (ins->rex.b << 3));

    return x64modrm_get_indirect(emu, ins);
}

void *x64modrm_get_xmm_m(x64emu_t *emu, x64instr_t *ins) {
    if (ins->modrm.mod == 3)
        return emu->xmm + (ins->modrm.rm | (ins->rex.b << 3));

    return x64modrm_get_indirect(emu, ins);
}

void *x64modrm_get_mmx_m(x64emu_t *emu, x64instr_t *ins) {
    if (ins->modrm.mod == 3)
        return emu->mmx + (ins->modrm.rm | (ins->rex.b << 3));

    return x64modrm_get_indirect(emu, ins);
}
