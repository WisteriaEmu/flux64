
#include <stdbool.h>

#include "debug.h"
#include "x64emu.h"
#include "x64instr.h"
#include "x64execute.h"
#include "x64regs_private.h"
#include "x64execute_private.h"
#include "x64modrm.h"

SET_DEBUG_CHANNEL("X64EXECUTE_0F")

bool x64execute_0f(x64emu_t *emu, x64instr_t *ins) {
    uint8_t op = ins->opcode[1];

    switch (op) {
        case 0x05:            /* SYSCALL */
            if (!x64syscall(emu))
                return false;
            break;

        case 0x18 ... 0x1F:   /* HINT_NOP */
            break;

        case 0x40 ... 0x4F:   /* CMOVcc r16/32/64,r/m16/32/64 */
            if (x64execute_jmp_cond(emu, ins, op))
                OPERATION_16_32_64(REG, R_M, OP_UNSIGNED_MOV, U_64)
            break;

        case 0x6E:            /* MOVD/MOVQ mm/xmm,r/m32/64 */
            if (ins->operand_sz) {    /* xmm */
                OPERATION_32_64(XMM, R_M, OP_UNSIGNED_MOV, U_64)
            } else {
                log_fixme("mmx");
                return false;
            }
            break;

        case 0x6F:            /* MOVQ/MOVDQA/MOVDQU mm/xmm,mm/m64/xmm/m128 */
            if (ins->operand_sz) {
                uint64_t *dest = (uint64_t *)x64modrm_get_xmm(emu, ins);
                uint64_t *src = (uint64_t *)x64modrm_get_xmm_m(emu, ins);
                *(dest++) = *(src++);
                *dest = *src;
            } else {
                log_fixme("mmx");
                return false;
            }
            break;

        case 0x80 ... 0x8F:   /* Jcc rel16/32 */
            if (x64execute_jmp_cond(emu, ins, op))
                r_rip += (ins->operand_sz) ? (int64_t)ins->imm.sword[0] : (int64_t)ins->imm.sdword[0];
            break;

        case 0x90 ... 0x9F: { /* SETcc r/m8 */
            void *dest = x64modrm_get_r_m(emu, ins);
            *(uint8_t *)dest = x64execute_jmp_cond(emu, ins, op) ? 1 : 0;
            break;
        }

        case 0xA2:            /* CPUID */
            /* FIXME: implement that. */
            r_eax = r_ebx = r_ecx = r_edx = 0;
            break;

        case 0xB6:            /* MOVZX r16/32/64,r/m8 */
            OPERATION_16_32_64(REG, R_M, OP_UNSIGNED_MOV, U_8)
            break;

        case 0xB7:            /* MOVZX r16/32/64,r/m16 */
            OPERATION_16_32_64(REG, R_M, OP_UNSIGNED_MOV, U_16)
            break;

        default:
            log_err("Unimplemented opcode 0F %02X", op);
            return false;
    }
    return true;
}