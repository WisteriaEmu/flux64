
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

#define DEST_XMM_SRC_XMM_M() \
    reg128_t *dest = (reg128_t *)x64modrm_get_xmm(emu, ins); \
    reg128_t *src = (reg128_t *)x64modrm_get_xmm_m(emu, ins);

#define DEST_XMM_M_SRC_XMM() \
    reg128_t *dest = (reg128_t *)x64modrm_get_xmm_m(emu, ins); \
    reg128_t *src = (reg128_t *)x64modrm_get_xmm(emu, ins);

#define DEST_MMX_SRC_MMX_M() \
    reg64_t *dest = (reg64_t *)x64modrm_get_mmx(emu, ins); \
    reg64_t *src = (reg64_t *)x64modrm_get_mmx_m(emu, ins);

#define DEST_MMX_M_SRC_MMX() \
    reg64_t *dest = (reg64_t *)x64modrm_get_mmx_m(emu, ins); \
    reg64_t *src = (reg64_t *)x64modrm_get_mmx(emu, ins);

#define OP_INTERLEAVE_LOW(op_type, times) \
    for (int i = 0; i < times; i++) { \
        dest->u ## op_type[i * 2] = dest->u ## op_type[i]; \
        dest->u ## op_type[i * 2 + 1] = src->u ## op_type[i]; \
    }

#define OP_INTERLEAVE_HIGH(op_type, times) \
    for (int i = 0; i < times; i++) { \
        dest->u ## op_type[i * 2] = dest->u ## op_type[i + times]; \
        dest->u ## op_type[i * 2 + 1] = src->u ## op_type[i + times]; \
    }

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
            DEST_XMM_SRC_XMM_M()
            if (ins->rep == 0xF3)                      /* MOVSS xmm,xmm/m32 */
                if (ins->modrm.mod == 3) dest->ud[0] = src->ud[0];
                else                     dest->uo[0] = (uint128_t)src->ud[0];
            else if (ins->rep == 0xF2)                 /* MOVSD xmm,xmm/m64 */
                if (ins->modrm.mod == 3) dest->uq[0] = src->uq[0];
                else                     dest->uo[0] = (uint128_t)src->uq[0];
            else                                       /* MOVUPS/MOVUPD xmm,xmm/m128 */
                dest->uo[0] = src->uo[0];
            break;
        }

        case 0x11: {          /* xmm/m,xmm */
            DEST_XMM_M_SRC_XMM()
            if (ins->rep == 0xF3)                      /* MOVSS xmm/m32,xmm */
                dest->ud[0] = src->ud[0];
            else if (ins->rep == 0xF2)                 /* MOVSD xmm/m64,xmm */
                dest->uq[0] = src->uq[0];
            else                                       /* MOVUPS/MOVUPD xmm/m128,xmm */
                dest->uo[0] = src->uo[0];
            break;
        }

        case 0x12: {          /* xmm,xmm/m */
            DEST_XMM_SRC_XMM_M()
            if (ins->rep == 0xF3) {                    /* MOVSLDUP xmm,xmm/m64 */
                dest->ud[0] = dest->ud[1] = src->ud[0];
                dest->ud[2] = dest->ud[3] = src->ud[1];
            } else if (ins->rep == 0xF2) {             /* MOVDDUP xmm,xmm/m64 */
                dest->uq[0] = dest->uq[1] = src->uq[0];
            } else if (ins->operand_sz) {              /* MOVLPD xmm,m64 */
                /* FIXME: This is not xmm/m64. */
                dest->uq[0] = src->uq[0];
            } else                                     /* MOVLPS/MOVHLPS xmm,xmm/m64 */
                dest->uq[0] = src->uq[ins->modrm.mod == 3];
            break;
        }

        case 0x13: {          /* MOVLPS/MOVLPD m64,xmm */
            /* FIXME: this is not xmm/m64. */
            DEST_XMM_M_SRC_XMM()
            dest->uq[0] = src->uq[0];
            break;
        }

        case 0x14: {          /* xmm,xmm/m */
            DEST_XMM_SRC_XMM_M()
            if (ins->operand_sz) {                     /* UNPCKLPD xmm,xmm/m128 */
                OP_INTERLEAVE_LOW(q, 1)
            } else {                                   /* UNPCKLPS xmm,xmm/m128 */
                OP_INTERLEAVE_LOW(d, 2)
            }
            break;
        }

        case 0x15: {          /* xmm,xmm/m */
            DEST_XMM_SRC_XMM_M()
            if (ins->operand_sz) {                     /* UNPCKHPD xmm,xmm/m128 */
                OP_INTERLEAVE_HIGH(q, 1)
            } else {                                   /* UNPCKHPS xmm,xmm/m128 */
                OP_INTERLEAVE_HIGH(d, 2)
            }
            break;
        }

        case 0x16: {          /* xmm,xmm/m */
            DEST_XMM_SRC_XMM_M()
            if (ins->rep == 0xF3) {                    /* MOVSHDUP xmm,xmm/m64 */
                dest->ud[0] = dest->ud[1] = src->ud[1];
                dest->ud[2] = dest->ud[3] = src->ud[3];
            } else if (ins->operand_sz) {              /* MOVHPD xmm,m64 */
                /* FIXME: This is not xmm/m. */
                dest->uq[0] = src->uq[1];
            } else {                                   /* MOVLHPS/MOVHPS xmm,xmm/m64 */
                dest->uq[1] = src->uq[0];
            }
            break;
        }

        case 0x17: {          /* MOVHPS/MOVHPD m64,xmm */
            /* FIXME: this is not xmm/m. */
            DEST_XMM_M_SRC_XMM()
            dest->uq[1] = src->uq[0];
            break;
        }

        case 0x18 ... 0x1F:   /* HINT_NOP */
            break;

        case 0x28: {          /* MOVAPS/MOVAPD xmm,xmm/m128 */
            DEST_XMM_SRC_XMM_M()
            dest->uo[0] = src->uo[0];
            break;
        }

        case 0x29: {          /* MOVAPS/MOVAPD xmm/m128,xmm */
            DEST_XMM_M_SRC_XMM()
            dest->uo[0] = src->uo[0];
            break;
        }

        case 0x2B: {          /* MOVNTPS/MOVNTPD m128,xmm */
            /* FIXME: this is not xmm/m */
            DEST_XMM_M_SRC_XMM()
            dest->uo[0] = src->uo[0];
            break;
        }

        case 0x40 ... 0x4F:   /* CMOVcc r16/32/64,r/m16/32/64 */
            if (x64execute_jmp_cond(emu, ins, op))
                OP2_16_32_64(REG, R_M, OP_U_MOV, U_64)
            break;

        case 0x60: {          /* PUNPCKLBW */
            if (ins->operand_sz) {                     /* xmm,xmm/m */
                DEST_XMM_SRC_XMM_M()
                OP_INTERLEAVE_LOW(b, 8)
            } else {                                   /* mmx,mmx/m */
                DEST_MMX_SRC_MMX_M()
                OP_INTERLEAVE_LOW(b, 4)
            }
            break;
        }

        case 0x61: {          /* PUNPCKLWD */
            if (ins->operand_sz) {                     /* xmm,xmm/m */
                DEST_XMM_SRC_XMM_M()
                OP_INTERLEAVE_LOW(w, 4)
            } else {                                   /* mmx,mmx/m */
                DEST_MMX_SRC_MMX_M()
                OP_INTERLEAVE_LOW(w, 2)
            }
            break;
        }

        case 0x62: {          /* PUNPCKLDQ */
            if (ins->operand_sz) {                     /* xmm,xmm/m */
                DEST_XMM_SRC_XMM_M()
                OP_INTERLEAVE_LOW(d, 2)
            } else {                                   /* mmx,mmx/m */
                DEST_MMX_SRC_MMX_M()
                OP_INTERLEAVE_LOW(d, 1)
            }
            break;
        }

        case 0x68: {          /* PUNPCKHBW */
            if (ins->operand_sz) {                     /* xmm,xmm/m */
                DEST_XMM_SRC_XMM_M()
                OP_INTERLEAVE_HIGH(b, 8)
            } else {                                   /* mmx,mmx/m */
                DEST_MMX_SRC_MMX_M()
                OP_INTERLEAVE_HIGH(b, 4)
            }
            break;
        }

        case 0x69: {          /* PUNPCKHWD */
            if (ins->operand_sz) {                     /* xmm,xmm/m */
                DEST_XMM_SRC_XMM_M()
                OP_INTERLEAVE_HIGH(w, 4)
            } else {                                   /* mmx,mmx/m */
                DEST_MMX_SRC_MMX_M()
                OP_INTERLEAVE_HIGH(w, 2)
            }
            break;
        }

        case 0x6A: {          /* PUNPCKHDQ */
            if (ins->operand_sz) {                     /* xmm,xmm/m */
                DEST_XMM_SRC_XMM_M()
                OP_INTERLEAVE_HIGH(d, 2)
            } else {                                   /* mmx,mmx/m */
                DEST_MMX_SRC_MMX_M()
                OP_INTERLEAVE_HIGH(d, 1)
            }
            break;
        }

        case 0x6E:            /* MOVD/MOVQ */
            if (ins->operand_sz) {                     /* xmm,xmm/m */
                DEST_XMM_SRC_XMM_M()
                dest->uo[0] = (ins->rex.w) ? (uint128_t)src->uq[0] : (uint128_t)src->ud[0];
            } else {                                   /* mmx,mmx/m */
                DEST_MMX_SRC_MMX_M()
                dest->uq[0] = (ins->rex.w) ? src->uq[0] : (uint64_t)src->ud[0];
            }
            break;

        case 0x6F:            /*  */
            if (ins->operand_sz || ins->rep == 0xF3) { /* MOVDQA/MOVDQU xmm,xmm/m */
                DEST_XMM_SRC_XMM_M()
                dest->uo[0] = src->uo[0];
            } else {                                   /* MOVQ mmx,mmx/m */
                DEST_MMX_SRC_MMX_M()
                dest->uq[0] = src->uq[0];
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