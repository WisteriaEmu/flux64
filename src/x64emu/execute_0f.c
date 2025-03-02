
#include <stdbool.h>
#include <stdint.h>

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

        case 0x0D:            /* NOP r/m16/32 */
            break;

        case 0x10: {          /* xmm,xmm/m */
            reg128_t *dest = (reg128_t *)x64modrm_get_xmm(emu, ins);
            reg128_t *src = (reg128_t *)x64modrm_get_xmm_m(emu, ins);
            if (ins->rep == 0xF3)           /* MOVSS xmm,xmm/m32 */
                if (ins->modrm.mod == 3) dest->ud[0] = src->ud[0];
                else                     dest->uo[0] = (uint128_t)src->ud[0];
            else if (ins->rep == 0xF2)      /* MOVSD xmm,xmm/m64 */
                if (ins->modrm.mod == 3) dest->uq[0] = src->uq[0];
                else                     dest->uo[0] = (uint128_t)src->uq[0];
            else                            /* MOVUPD/MOVUPS xmm,xmm/m128 */
                dest->uo[0] = src->uo[0];
            break;
        }

        case 0x11: {          /* xmm/m,xmm */
            reg128_t *dest = (reg128_t *)x64modrm_get_xmm_m(emu, ins);
            reg128_t *src = (reg128_t *)x64modrm_get_xmm(emu, ins);
            if (ins->rep == 0xF3)           /* MOVSS xmm/m32,xmm */
                dest->ud[0] = src->ud[0];
            else if (ins->rep == 0xF2)      /* MOVSD xmm/m64,xmm */
                dest->uq[0] = src->uq[0];
            else                            /* MOVUPD/MOVUPS xmm/m128,xmm */
                dest->uo[0] = src->uo[0];
            break;
        }

        case 0x12: {          /* xmm,xmm/m */
            reg128_t *dest = (reg128_t *)x64modrm_get_xmm(emu, ins);
            reg128_t *src = (reg128_t *)x64modrm_get_xmm_m(emu, ins);
            if (ins->rep == 0xF3) {         /* MOVSLDUP xmm,xmm/m64 */
                dest->ud[0] = dest->ud[1] = src->ud[0];
                dest->ud[2] = dest->ud[3] = src->ud[1];
            } else if (ins->rep == 0xF2) {  /* MOVDDUP xmm,xmm/m64 */
                dest->uq[0] = dest->uq[1] = src->uq[0];
            } else if (ins->operand_sz) {   /* MOVLPD xmm,m64 */
                /* FIXME: This is not xmm/m64. */
                dest->uq[0] = src->uq[0];
            } else                          /* MOVLPS/MOVHLPS xmm,xmm/m64 */
                dest->uq[0] = src->uq[ins->modrm.mod == 3];
            break;
        }

        case 0x13: {          /* MOVLPD/MOVLPS m64,xmm */
            /* FIXME: this is not xmm/m64. */
            reg128_t *dest = (reg128_t *)x64modrm_get_xmm_m(emu, ins);
            reg128_t *src = (reg128_t *)x64modrm_get_xmm(emu, ins);
            dest->uq[0] = src->uq[0];
            break;
        }

        case 0x14: {          /* xmm,xmm/m */
            reg128_t *dest = (reg128_t *)x64modrm_get_xmm(emu, ins);
            reg128_t *src = (reg128_t *)x64modrm_get_xmm_m(emu, ins);
            if (ins->operand_sz) {          /* UNPCKLPD xmm,xmm/m128 */
                dest->uq[1] = src->uq[0];
            } else {                        /* UNPCKLPS xmm,xmm/m128 */
                dest->ud[2] = dest->ud[1];
                dest->ud[1] = src->ud[0];
                dest->ud[3] = src->ud[1];
            }
            break;
        }

        case 0x15: {          /* xmm,xmm/m */
            reg128_t *dest = (reg128_t *)x64modrm_get_xmm(emu, ins);
            reg128_t *src = (reg128_t *)x64modrm_get_xmm_m(emu, ins);
            if (ins->operand_sz) {          /* UNPCKHPD xmm,xmm/m128 */
                dest->uq[0] = dest->uq[1];
                dest->uq[1] = src->uq[1];
            } else {                        /* UNPCKHPS xmm,xmm/m128 */
                dest->ud[0] = dest->ud[2];
                dest->ud[1] = src->ud[2];
                dest->ud[2] = dest->ud[3];
                dest->ud[3] = src->ud[3];
            }
            break;
        }



        case 0x18 ... 0x1F:   /* HINT_NOP */
            break;

        case 0x40 ... 0x4F:   /* CMOVcc r16/32/64,r/m16/32/64 */
            if (x64execute_jmp_cond(emu, ins, op))
                OP2_16_32_64(REG, R_M, OP_U_MOV, U_64)
            break;

        case 0x6E:            /* MOVD/MOVQ mm/xmm,r/m32/64 */
            if (ins->operand_sz) {    /* xmm */
                reg128_t *dest = (reg128_t *)x64modrm_get_xmm(emu, ins);
                reg128_t *src = (reg128_t *)x64modrm_get_xmm_m(emu, ins);
                dest->uo[0] = (uint128_t)((ins->rex.w) ? src->uq[0] : src->ud[0]);
            } else {
                log_fixme("mmx");
                return false;
            }
            break;

        case 0x6F:            /* MOVQ/MOVDQA/MOVDQU mm/xmm,mm/m64/xmm/m128 */
            if (ins->operand_sz) {
                reg128_t *dest = (reg128_t *)x64modrm_get_xmm(emu, ins);
                reg128_t *src = (reg128_t *)x64modrm_get_xmm_m(emu, ins);
                dest->uo[0] = src->uo[0];
            } else {
                log_fixme("mmx");
                return false;
            }
            break;

        case 0x80 ... 0x8F:   /* Jcc rel16/32 */
            if (x64execute_jmp_cond(emu, ins, op))
                r_rip += (ins->operand_sz) ? (int64_t)ins->imm.sw[0] : (int64_t)ins->imm.sd[0];
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
            OP2_16_32_64(REG, R_M, OP_U_MOV, U_8)
            break;

        case 0xB7:            /* MOVZX r16/32/64,r/m16 */
            OP2_16_32_64(REG, R_M, OP_U_MOV, U_16)
            break;

        default:
            log_err("Unimplemented opcode 0F %02X", op);
            return false;
    }
    return true;
}