#ifndef __X64EMU_H_
#define __X64EMU_H_

#include <stdbool.h>
#include <stdint.h>

#include "x64regs.h"
#include "x64context.h"

/**
 * Current state of the cpu.
 */
typedef struct x64emu_s {
    x64context_t *ctx;
    reg64_t       regs[16]; /* 16 general-purpose registers. */
    reg64_t       rip;      /* Instruction pointer. */
} x64emu_t;

/**
 * Bind context, inititalize the stack and registers.
 */
bool x64emu_init(x64emu_t *emu, x64context_t *ctx);

/**
 * Push initial data to the stack.
 */
void x64stack_setup(x64emu_t *emu);

/**
 * Execute instructions.
 */
void x64emu_run(x64emu_t *emu);


#if 0
// set/get registers and instruction pointer

#define ADD(r) \
    void     set_##r(x64emu_t *emu, uint64_t v); \
    uint64_t get_##r(x64emu_t *emu);

ADD(rax) ADD(rcx) ADD(rdx) ADD(rbx)
ADD(rsi) ADD(rdi) ADD(rsp) ADD(rbp)
ADD(r8 ) ADD(r9 ) ADD(r10) ADD(r11)
ADD(r12) ADD(r13) ADD(r14) ADD(r15)
ADD(rip)

#undef ADD
#endif

#endif /* __X64EMU_H_ */
