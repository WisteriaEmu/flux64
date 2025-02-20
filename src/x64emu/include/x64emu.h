#ifndef __X64EMU_H_
#define __X64EMU_H_

#include <stdbool.h>

#include "x64flags.h"
#include "x64regs.h"
#include "x64context.h"

/**
 * Current state of the cpu.
 */
typedef struct {
    x64context_t *ctx;
    reg64_t       regs[16]; /* 16 general-purpose registers. */
    reg64_t       rip;      /* Instruction pointer. */
    x64flags_t    flags;    /* RFLAGS register. */
} x64emu_t;

/**
 * Bind context, inititalize the stack and registers.
 */
bool x64emu_init(x64emu_t *emu, x64context_t *ctx);

/**
 * Execute instructions.
 * Returns when a bad opcode is fetched.
 */
void x64emu_run(x64emu_t *emu);

/**
 * Free emu and context.
 */
bool x64emu_free(x64emu_t *emu);

bool x64syscall(x64emu_t *emu);

#endif /* __X64EMU_H_ */
