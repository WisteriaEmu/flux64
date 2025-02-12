#ifndef __X64MODRM_H_
#define __X64MODRM_H_

#include <stdint.h>

#include "x64emu.h"
#include "x64instr.h"

/**
 * Decode ModR/M byte of the current instruction.
 */
void x64modrm_fetch(x64emu_t *emu, x64instr_t *ins);

void *x64modrm_src_reg(x64emu_t *emu, x64instr_t *ins);

void *x64modrm_dest_rm(x64emu_t *emu, x64instr_t *ins);

#endif /* __X64MODRM_H_ */