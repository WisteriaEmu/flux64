
#include <stdbool.h>
#include <stdint.h>

#include "debug.h"
#include "x64emu.h"
#include "x64instr.h"
#include "x64decode.h"
#include "x64modrm.h"
#include "x64regs_private.h"
#include "x64decode_private.h"

SET_DEBUG_CHANNEL("X64DECODE_0F")

bool x64decode_0f(x64emu_t *emu, x64instr_t *ins) {
    ins->opcode[1] = fetch_8(emu, ins);

    switch (ins->opcode[1]) {
        case 0x05:            /* SYSCALL */
            break;

        case 0x0D:            /* NOP r/m16/32 */
            x64modrm_fetch(emu, ins);
            break;

        case 0x10 ... 0x17:   /* sse1/2/3 opcodes. */
        case 0x18 ... 0x1F:   /* HINT_NOP */
            x64modrm_fetch(emu, ins);
            break;

        case 0x28 ... 0x2F:   /* sse1/sse2 opcodes. */
            x64modrm_fetch(emu, ins);
            break;

        case 0x40 ... 0x4F:   /* CMOVcc r16/32/64,r/m16/32/64 */
        case 0x50 ... 0x6F:   /* sse1/sse2/mmx opcodes. */
            x64modrm_fetch(emu, ins);
            break;

        case 0x70 ... 0x73:   /* sse1/sse2/mmx opcodes. */
            x64modrm_fetch(emu, ins);
            FETCH_IMM_8()
            break;

        case 0x74 ... 0x77:   /* sse2/mmx opcodes. */
            x64modrm_fetch(emu, ins);
            break;

        case 0x7C ... 0x7F:   /* sse2/sse3/mmx opcodes. */
            x64modrm_fetch(emu, ins);
            break;

        case 0x80 ... 0x8F:   /* Jcc rel16/32 */
            FETCH_IMM_16_32()
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