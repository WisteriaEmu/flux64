
#include <stdbool.h>

#include "debug.h"
#include "x64emu.h"
#include "x64instr.h"
#include "x64decode.h"
#include "x64modrm.h"
#include "x64regs_private.h"

SET_DEBUG_CHANNEL("X64DECODE_0F")

bool x64decode_0f(x64emu_t *emu, x64instr_t *ins) {
    ins->opcode[1] = fetch_8(emu, ins);

    switch (ins->opcode[1]) {
        case 0x05:           /* SYSCALL */
            break;

        case 0x18 ... 0x1F:  /* HINT_NOP */
            x64modrm_fetch(emu, ins);
            break;

        default:
            log_err("Unhandled opcode 0F %02X", ins->opcode[1]);
            return false;
    }
    return true;
}