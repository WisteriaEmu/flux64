
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
        case 0x05:           /* SYSCALL */
            if (!x64syscall(emu))
                return false;
            break;

        case 0x18 ... 0x1F:  /* HINT_NOP */
            break;

        case 0x80 ... 0x8F:  /* Jcc rel16/32 */
            if (x64execute_jmp_cond(emu, ins, op)) {
                if (ins->operand_sz)
                    r_eip += (int32_t)ins->imm.sword[0];
                else
                    r_eip += ins->imm.sdword[0];
                log_dump("Jump taken");
            } else log_dump("Jump not taken");
            break;

        case 0xB6: {         /* MOVZX r16/32/64,r/m8 */
            void *src  = x64modrm_get_rm(emu, ins);
            void *dest = x64modrm_get_reg(emu, ins);
            DEST_OPERATION(OP_UNSIGNED_MOV, (uint64_t)(*(uint8_t *)src),
                           (uint16_t)(*(uint8_t *)src), (uint32_t)(*(uint8_t *)src))
            break;
        }

        case 0xB7: {         /* MOVZX r16/32/64,r/m16 */
            void *src  = x64modrm_get_rm(emu, ins);
            void *dest = x64modrm_get_reg(emu, ins);
            DEST_OPERATION(OP_UNSIGNED_MOV, (uint64_t)(*(uint16_t *)src),
                           *(uint16_t *)src, (uint32_t)(*(uint16_t *)src))
            break;
        }

        default:
            log_err("Unimplemented opcode 0F %02X", op);
            return false;
    }
    return true;
}