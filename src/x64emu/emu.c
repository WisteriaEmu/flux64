#include <stdbool.h>
#include <stdint.h>
#include <string.h>

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

static inline void print_emu_state(x64emu_t *emu, x64instr_t *ins, uint64_t rip) {
    static x64emu_t emu_saved = { 0 };

    char changes[256] = { 0 };
    char *target = changes;

    if (emu->flags.raw != emu_saved.flags.raw) {
        target += sprintf(target, "rflags: [ ");
#define ADD_FLAG(flag) if (f_ ## flag) target += sprintf(target, "%s ", #flag);
        ADD_FLAG(CF)   ADD_FLAG(PF)   ADD_FLAG(AF)   ADD_FLAG(ZF)
        ADD_FLAG(SF)   ADD_FLAG(TF)   ADD_FLAG(IF)   ADD_FLAG(DF)
        ADD_FLAG(OF)   ADD_FLAG(IOPL) ADD_FLAG(NT)   ADD_FLAG(RF)
        ADD_FLAG(VM)   ADD_FLAG(AC)   ADD_FLAG(VIF)  ADD_FLAG(VIP)
        ADD_FLAG(ID)
#undef ADD_FLAG
        target += sprintf(target, "]   ");
        emu_saved.flags.raw = emu->flags.raw;
    }

#define ADD_REG(reg) \
    if (r_ ## reg != emu_saved.regs[_ ## reg].qword[0]) { \
        target += sprintf(target, "%s: %lx   ", #reg, r_ ## reg); \
        emu_saved.regs[_ ## reg].qword[0] = r_ ## reg; \
    }
    ADD_REG(rax) ADD_REG(rcx) ADD_REG(rdx) ADD_REG(rbx)
    ADD_REG(rsp) ADD_REG(rbp) ADD_REG(rsi) ADD_REG(rdi)
    ADD_REG(r8)  ADD_REG(r9)  ADD_REG(r10) ADD_REG(r11)
    ADD_REG(r12) ADD_REG(r13) ADD_REG(r14) ADD_REG(r15)
#undef ADD_REG

    char instr_str[48] = { 0 };
    for (uint8_t i = 0; i < ins->desc.bytes_len; i++) {
        sprintf(instr_str + i * 3, "%02X ", ins->desc.bytes[i]);
    }

    log_dump("%lx: %-32s %s", rip, instr_str, changes);
}

void x64emu_run(x64emu_t *emu) {
    while (1) {
        x64instr_t instr = { 0 };
        uint64_t start_rip = r_rip;

        if (!x64decode(emu, &instr))
            return;

        print_emu_state(emu, &instr, start_rip);

        if (!x64execute(emu, &instr))
            return;
    }
}
