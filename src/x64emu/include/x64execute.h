#ifndef __X64EXECUTE_H_
#define __X64EXECUTE_H_

#include <stdbool.h>

#include "x64emu.h"
#include "x64instr.h"

/**
 * Execute x86_64 instruction `ins`.
 */
bool x64execute(x64emu_t *emu, x64instr_t *ins);

bool x64execute_0f(x64emu_t *emu, x64instr_t *ins);

#endif /* __X64EXECUTE_H_ */