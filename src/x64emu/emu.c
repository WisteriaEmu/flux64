#include <stdbool.h>
#include <stdint.h>

#include "x64emu.h"
#include "x64instr.h"
#include "x64decode.h"
#include "x64execute.h"
#include "x64regs_private.h"
#include "x64flags_private.h"
#include "x64stack.h"
#include "debug.h"
#include "x64flags.h"

SET_DEBUG_CHANNEL("X64EMU")

bool x64emu_init(x64emu_t *emu, x64context_t *ctx) {
    if (!emu || !ctx) return false;

    emu->ctx = ctx;

    r_rsp = (uintptr_t)ctx->stack.base + ctx->stack.size; /* top of the stack */
    x64stack_setup(emu);

    r_rdi = ctx->argc;
    r_rsi = r_rsp + 8; /* argv */
    r_rdx = r_rsi + 8 * (ctx->argc + 1); /* envp */
    /* NOTE: rbp (frame pointer) is initialized to 0. */
    r_rip = ctx->entry;

    return true;
}

void x64emu_run(x64emu_t *emu) {
    while (1) {
        x64instr_t instr = { 0 };
        uint64_t saved_rip = r_rip;

        if (!x64decode(emu, &instr))
            return;

        log_dump("%lx: %-48s C/P/A/Z/S/OF: %x%x%x%x%x%x "
                 "rax/cx/dx/bx: %lx %lx %lx %lx "
                 "rsp/bp/si/di: %lx %lx %lx %lx",
                 saved_rip, instr.desc.str,
                 f_CF, f_PF, f_AF, f_ZF, f_SF, f_OF,
                 r_rax, r_rcx, r_rdx, r_rbx,
                 r_rsp, r_rbp, r_rsi, r_rdi);

        if (!x64execute(emu, &instr))
            return;
    }
}
