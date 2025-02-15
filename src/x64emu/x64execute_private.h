
#ifndef __EXECUTE_PRIVATE_H_
#define __EXECUTE_PRIVATE_H_

/* Set SF, ZF, PF. */
#define SET_RESULT_FLAGS(x) \
    f_SF = (x) < 0; \
    f_ZF = (x) == 0; \
    f_PF = !__builtin_parity((uint8_t)(x));

/* Perform bitwise operation and update flags. */
/* NOTE: state of AF is undefined. */
#define OP_BITWISE(oper, s_type, u_type, operand) { \
    *(s_type *)dest oper ## = (operand); \
    SET_RESULT_FLAGS(*(s_type *)dest) \
    f_CF = f_OF = 0; \
}

/* `*dest ^= operand`, update flags. */
#define OP_BITWISE_XOR(s_type, u_type, operand) OP_BITWISE(^, s_type, u_type, operand)
/* `*dest &= operand`, update flags. */
#define OP_BITWISE_AND(s_type, u_type, operand) OP_BITWISE(&, s_type, u_type, operand)
/* `*dest |= operand`, update flags. */
#define OP_BITWISE_OR(s_type, u_type, operand)  OP_BITWISE(|, s_type, u_type, operand)

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

/* Same as SUB, but does throws away the result. */
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

/* Perform specified operation on dest using src operands. */
#define DEST_OPERATION(operation, src64, src16, src32) \
    if (!dest) return false; \
    if      (ins->rex.w)      operation(int64_t, uint64_t, src64) \
    else if (ins->operand_sz) operation(int16_t, uint16_t, src16) \
    else                      operation(int32_t, uint32_t, src32)

/* Perform specified operation on dest using byte or word sign-extended operand. */
#define DEST_OPERATION_SX(operation, src) \
    DEST_OPERATION(operation, (int64_t)src, (int16_t)src, (int32_t)src)

#endif /* __EXECUTE_PRIVATE_H_ */