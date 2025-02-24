#ifndef __X64MODRM_H_
#define __X64MODRM_H_

#include <stdint.h>

#include "x64emu.h"
#include "x64instr.h"

/**
 * Decode ModR/M byte of the current instruction.
 */
void x64modrm_fetch(x64emu_t *emu, x64instr_t *ins);

/** Address of general purpose register encoded using ModR/M byte. */
void *x64modrm_get_reg(x64emu_t *emu, x64instr_t *ins);

/** Direct or indirect addressing using general purpose registers. */
void *x64modrm_get_r_m(x64emu_t *emu, x64instr_t *ins);

/** Address of XMM register encoded using ModR/M byte. */
void *x64modrm_get_xmm(x64emu_t *emu, x64instr_t *ins);

/** Direct or indirect addressing using XMM registers. */
void *x64modrm_get_xmm_m(x64emu_t *emu, x64instr_t *ins);

#endif /* __X64MODRM_H_ */