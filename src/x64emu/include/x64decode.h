#ifndef __X64DECODE_H_
#define __X64DECODE_H_

#include <stdbool.h>

#include "x64emu.h"
#include "x64instr.h"

/**
 * Decode the next x86_64 instruction into `x64instr_t` fields.
 */
bool x64decode(x64emu_t *emu, x64instr_t *ins);

bool x64decode_0f(x64emu_t *emu, x64instr_t *ins);

#endif /* __X64DECODE_H_ */