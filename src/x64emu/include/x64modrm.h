#ifndef __X64MODRM_H_
#define __X64MODRM_H_

#include "x64emu.h"
#include "x64instr.h"

/**
 * Decode ModR/M byte of the current instruction.
 */
void x64modrm_fetch(x64emu_t *emu, x64instr_t *ins);

/* Get memory address. */
void *x64modrm_get_indirect(x64emu_t *emu, x64instr_t *ins);

/** GPR. */
void *x64modrm_get_reg(x64emu_t *emu, x64instr_t *ins);

/** GPR or memory address. */
void *x64modrm_get_r_m(x64emu_t *emu, x64instr_t *ins);

/** XMM register. */
void *x64modrm_get_xmm(x64emu_t *emu, x64instr_t *ins);

/** XMM register or memory address. */
void *x64modrm_get_xmm_m(x64emu_t *emu, x64instr_t *ins);

/** MMX register. */
void *x64modrm_get_mmx(x64emu_t *emu, x64instr_t *ins);

/** MMX register or memory address. */
void *x64modrm_get_mmx_m(x64emu_t *emu, x64instr_t *ins);

#endif /* __X64MODRM_H_ */