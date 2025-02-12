#include <stdbool.h>
#include <stdint.h>

#include "x64emu.h"
#include "x64instr.h"
#include "x64decode.h"
#include "x64execute.h"
#include "x64regs_private.h"
#include "debug.h"

SET_DEBUG_CHANNEL("X64EMU")

#if 0
// to set and get registers
#define ADD(r) \
    void     set_##r(x64emu_t *emu, uint64_t v) { emu->regs[_##r].qword[0] = v; } \
    uint64_t get_##r(x64emu_t *emu) { return emu->regs[_##r].qword[0]; }
ADD(rax) ADD(rcx) ADD(rdx) ADD(rbx)
ADD(rsi) ADD(rdi) ADD(rsp) ADD(rbp)
ADD(r8 ) ADD(r9 ) ADD(r10) ADD(r11)
ADD(r12) ADD(r13) ADD(r14) ADD(r15)
ADD(rip)
#undef ADD
#endif

//#define ADD(r) void set_e##r(x64emu_t *emu, uint64_t v) { emu->regs[_r##r].dword[0] = v; }

bool x64emu_init(x64emu_t *emu, x64context_t *ctx) {
    if (!emu || !ctx) return false;

    emu->ctx = ctx;

    r_rsp = (uintptr_t)ctx->stack.base + ctx->stack.size; /* top of the stack */
    x64stack_setup(emu);

    r_rdi = ctx->argc;
    r_rsi = r_rsp + 8; /* argv */
    r_rdx = r_rsi + 8 * (ctx->argc + 1); /* envp */
    r_rip = ctx->entry;

    return true;
}

void x64emu_run(x64emu_t *emu) {
    while (1) {
        x64instr_t instr = { 0 };
        if (!x64decode(emu, &instr))
            return;

        if (!x64execute(emu, &instr))
            return;
    }
}
