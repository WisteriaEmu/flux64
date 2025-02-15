
#ifndef __EXECUTE_PRIVATE_H_
#define __EXECUTE_PRIVATE_H_

/* Set SF, ZF, PF. */
#define SET_RESULT_FLAGS(x) \
    f_SF = (x) < 0; \
    f_ZF = (x) == 0; \
    f_PF = !__builtin_parity((uint8_t)(x));

/* Perform bitwise operation and update flags. */
/* NOTE: state of AF is undefined. */
#define OP_BITWISE_IMPL(oper, s_type, u_type, operand) { \
    *(s_type *)dest oper ## = (operand); \
    SET_RESULT_FLAGS(*(s_type *)dest) \
    f_CF = f_OF = 0; \
}

#define OP_BITWISE_TEST_IMPL(oper, s_type, u_type, operand) { \
    s_type _res = (*(s_type *)dest) oper (operand); \
    SET_RESULT_FLAGS(_res) \
    f_CF = f_OF = 0; \
}

/* `*dest ^= operand`, update flags. */
#define OP_BITWISE_XOR(s_type, u_type, operand) \
    OP_BITWISE_IMPL(^, s_type, u_type, operand)

/* `*dest &= operand`, update flags. */
#define OP_BITWISE_AND(s_type, u_type, operand) \
    OP_BITWISE_IMPL(&, s_type, u_type, operand)

/* Same as AND, but discarding result. */
#define OP_BITWISE_TEST_AND(s_type, u_type, operand) \
    OP_BITWISE_TEST_IMPL(&, s_type, u_type, operand)

/* `*dest |= operand`, update flags. */
#define OP_BITWISE_OR(s_type, u_type, operand) \
    OP_BITWISE_IMPL(|, s_type, u_type, operand)

/* `*dest += operand`, update flags.
   CF, AF as unsigned, OF as signed.
   operand should be signed type. */
#define OP_SIGNED_ADD(s_type, u_type, operand) { \
    s_type _sav = *(s_type *)dest; \
    s_type _res = _sav + (operand); \
    SET_RESULT_FLAGS(_res) \
    f_CF = (u_type)_res < (u_type)_sav; \
    f_AF = (uint8_t)_res < (uint8_t)_sav; \
    f_OF = ((operand) <  0 && _sav <  0 && _res >= 0) || \
           ((operand) >= 0 && _sav >= 0 && _res <  0); \
    *(s_type *)dest = _res; \
}

#define OP_SIGNED_CMP_IMPL(s_type, u_type, operand) \
    s_type _sav = *(s_type *)dest; \
    s_type _res = _sav - (operand); \
    SET_RESULT_FLAGS(_res) \
    f_CF = (u_type)_res > (u_type)_sav; \
    f_AF = (uint8_t)_res > (uint8_t)_sav; \
    f_OF = ((operand) >= 0 && _sav <  0 && _res >= 0) || \
           ((operand) <  0 && _sav >= 0 && _res <  0);

/* Same as SUB, but discarding result. */
#define OP_SIGNED_CMP(s_type, u_type, operand) { \
    OP_SIGNED_CMP_IMPL(s_type, u_type, operand) \
}

/* `*dest -= operand`, update flags.
   CF, AF as unsigned, OF as signed.
   operand should be signed type. */
#define OP_SIGNED_SUB(s_type, u_type, operand) { \
    OP_SIGNED_CMP_IMPL(s_type, u_type, operand) \
    *(s_type *)dest = _res; \
}

/* `*dest = operand` */
#define OP_SIGNED_MOV(s_type, u_type, operand) { \
    *(s_type *)dest = operand; \
}

/* `*dest = operand`, RCX/ECX times if rep specified. */
#define OP_UNSIGNED_MOV_REP(s_type, u_type, operand) { \
    u_type *tdest = (u_type *)dest; \
    if (!ins->rep) *tdest = operand; \
    else if (ins->address_sz) \
        for (; r_ecx; r_ecx--) \
            if (f_DF) *(tdest--) = operand; \
            else      *(tdest++) = operand; \
    else \
        for (; r_rcx; r_rcx--) \
            if (f_DF) *(tdest--) = operand; \
            else      *(tdest++) = operand; \
}

/* Perform specified operation on dest using src operands. */
#define DEST_OPERATION(operation, src64, src16, src32) \
    if      (ins->rex.w)      operation(int64_t, uint64_t, src64) \
    else if (ins->operand_sz) operation(int16_t, uint16_t, src16) \
    else                      operation(int32_t, uint32_t, src32)

/* Perform specified operation on dest using byte or word sign-extended operand. */
#define DEST_OPERATION_SX(operation, src) \
    DEST_OPERATION(operation, (int64_t)src, (int16_t)src, (int32_t)src)

static inline bool x64execute_jmp_cond(x64emu_t *emu, x64instr_t *ins, uint8_t op) {
    bool ret = false;
    switch (op & 0xF) {
        case 0x0: ret =  f_OF;                 break;
        case 0x1: ret = !f_OF;                 break;
        case 0x2: ret =  f_CF;                 break;
        case 0x3: ret = !f_CF;                 break;
        case 0x4: ret =  f_ZF;                 break;
        case 0x5: ret = !f_ZF;                 break;
        case 0x6: ret =  f_CF ||  f_ZF;        break;
        case 0x7: ret = !f_CF && !f_ZF;        break;
        case 0x8: ret =  f_SF;                 break;
        case 0x9: ret = !f_SF;                 break;
        case 0xA: ret =  f_PF;                 break;
        case 0xB: ret = !f_PF;                 break;
        case 0xC: ret =  f_SF != f_OF;         break;
        case 0xD: ret =  f_SF == f_OF;         break;
        case 0xE: ret =  f_ZF || f_SF != f_OF; break;
        case 0xF: ret = !f_ZF && f_SF == f_OF; break;
    }
    if (ret) log_dump("Jump taken");
    else log_dump("Jump not taken");
    return ret;
}

#endif /* __EXECUTE_PRIVATE_H_ */