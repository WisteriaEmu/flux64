
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
        case 0x05:            /* SYSCALL */
            break;

        case 0x18 ... 0x1F:   /* HINT_NOP */
            x64modrm_fetch(emu, ins);
            break;

        case 0x40 ... 0x4F:   /* CMOVcc r16/32/64,r/m16/32/64 */
            x64modrm_fetch(emu, ins);
            break;

        case 0x80 ... 0x8F:   /* Jcc rel16/32 */
            if (ins->operand_sz)
                ins->imm.word[0] = fetch_16(emu, ins);
            else
                ins->imm.dword[0] = fetch_32(emu, ins);
            break;

        case 0x90 ... 0x9F:   /* SETcc r/m8 */
            x64modrm_fetch(emu, ins);
            break;

        case 0xA2:            /* CPUID */
            break;

        case 0xB6 ... 0xB7:   /* MOVZX r16/32/64,r/m8 / r16/32/64,r/m16 */
            x64modrm_fetch(emu, ins);
            break;

        default:
            log_err("Unhandled opcode 0F %02X", ins->opcode[1]);
            return false;
    }
    return true;
}