
#include <stdbool.h>

#include "debug.h"
#include "x64emu.h"
#include "x64instr.h"
#include "x64execute.h"
#include "x64regs_private.h"

SET_DEBUG_CHANNEL("X64EXECUTE_0F")

bool x64execute_0f(x64emu_t *emu, x64instr_t *ins) {
    switch (ins->opcode[1]) {
        case 0x05:           /* SYSCALL */
            if (!x64syscall(emu))
                return false;
            break;

        case 0x18 ... 0x1F:  /* HINT_NOP */
            break;

        default:
            log_err("Unimplemented opcode 0F %02X", ins->opcode[1]);
            return false;
    }
    return true;
}