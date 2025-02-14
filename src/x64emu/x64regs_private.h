#ifndef __X64REGS_PRIVATE_H_
#define __X64REGS_PRIVATE_H_

enum {
    _rax, _rcx, _rdx, _rbx,
    _rsp, _rbp, _rsi, _rdi,
    _r8,  _r9,  _r10, _r11,
    _r12, _r13, _r14, _r15,
    _rip
};

/* macros used to directly access registers internally. */

/* indexed registers */
#define r_reg64(x) emu->regs[x].qword[0]

/* unsigned */

/* 8-byte registers */
#define r_rax  emu->regs[_rax].qword[0]
#define r_rcx  emu->regs[_rcx].qword[0]
#define r_rdx  emu->regs[_rdx].qword[0]
#define r_rbx  emu->regs[_rbx].qword[0]
#define r_rsp  emu->regs[_rsp].qword[0]
#define r_rbp  emu->regs[_rbp].qword[0]
#define r_rsi  emu->regs[_rsi].qword[0]
#define r_rdi  emu->regs[_rdi].qword[0]

#define r_r8   emu->regs[_r8 ].qword[0]
#define r_r9   emu->regs[_r9 ].qword[0]
#define r_r10  emu->regs[_r10].qword[0]
#define r_r11  emu->regs[_r11].qword[0]
#define r_r12  emu->regs[_r12].qword[0]
#define r_r13  emu->regs[_r13].qword[0]
#define r_r14  emu->regs[_r14].qword[0]
#define r_r15  emu->regs[_r15].qword[0]

#define r_rip  emu->rip.qword[0]

/* bytes 0-3 */
#define r_eax  emu->regs[_rax].dword[0]
#define r_ecx  emu->regs[_rcx].dword[0]
#define r_edx  emu->regs[_rdx].dword[0]
#define r_ebx  emu->regs[_rbx].dword[0]
#define r_esp  emu->regs[_rsp].dword[0]
#define r_ebp  emu->regs[_rbp].dword[0]
#define r_esi  emu->regs[_rsi].dword[0]
#define r_edi  emu->regs[_rdi].dword[0]

#define r_r8d  emu->regs[_r8 ].dword[0]
#define r_r9d  emu->regs[_r9 ].dword[0]
#define r_r10d emu->regs[_r10].dword[0]
#define r_r11d emu->regs[_r11].dword[0]
#define r_r12d emu->regs[_r12].dword[0]
#define r_r13d emu->regs[_r13].dword[0]
#define r_r14d emu->regs[_r14].dword[0]
#define r_r15d emu->regs[_r15].dword[0]

#define r_eip  emu->rip.dword[0]

#define r_flags emu->flags.raw

/* bytes 0-1 */
#define r_ax   emu->regs[_rax].word[0]
#define r_cx   emu->regs[_rcx].word[0]
#define r_dx   emu->regs[_rdx].word[0]
#define r_bx   emu->regs[_rbx].word[0]
#define r_sp   emu->regs[_rsp].word[0]
#define r_bp   emu->regs[_rbp].word[0]
#define r_si   emu->regs[_rsi].word[0]
#define r_di   emu->regs[_rdi].word[0]

#define r_r8w  emu->regs[_r8 ].word[0]
#define r_r9w  emu->regs[_r9 ].word[0]
#define r_r10w emu->regs[_r10].word[0]
#define r_r11w emu->regs[_r11].word[0]
#define r_r12w emu->regs[_r12].word[0]
#define r_r13w emu->regs[_r13].word[0]
#define r_r14w emu->regs[_r14].word[0]
#define r_r15w emu->regs[_r15].word[0]

/* byte 0 */
#define r_al   emu->regs[_rax].byte[0]
#define r_cl   emu->regs[_rcx].byte[0]
#define r_dl   emu->regs[_rdx].byte[0]
#define r_bl   emu->regs[_rbx].byte[0]
#define r_spl  emu->regs[_rsp].byte[0]
#define r_bpl  emu->regs[_rbp].byte[0]
#define r_sil  emu->regs[_rsi].byte[0]
#define r_dil  emu->regs[_rdi].byte[0]

#define r_r8b  emu->regs[_r8 ].byte[0]
#define r_r9b  emu->regs[_r9 ].byte[0]
#define r_r10b emu->regs[_r10].byte[0]
#define r_r11b emu->regs[_r11].byte[0]
#define r_r12b emu->regs[_r12].byte[0]
#define r_r13b emu->regs[_r13].byte[0]
#define r_r14b emu->regs[_r14].byte[0]
#define r_r15b emu->regs[_r15].byte[0]


/* signed */

/* 8-byte registers */
#define s_rax  emu->regs[_rax].sqword[0]
#define s_rcx  emu->regs[_rcx].sqword[0]
#define s_rdx  emu->regs[_rdx].sqword[0]
#define s_rbx  emu->regs[_rbx].sqword[0]
#define s_rsp  emu->regs[_rsp].sqword[0]
#define s_rbp  emu->regs[_rbp].sqword[0]
#define s_rsi  emu->regs[_rsi].sqword[0]
#define s_rdi  emu->regs[_rdi].sqword[0]

#define s_r8   emu->regs[_r8 ].sqword[0]
#define s_r9   emu->regs[_r9 ].sqword[0]
#define s_r10  emu->regs[_r10].sqword[0]
#define s_r11  emu->regs[_r11].sqword[0]
#define s_r12  emu->regs[_r12].sqword[0]
#define s_r13  emu->regs[_r13].sqword[0]
#define s_r14  emu->regs[_r14].sqword[0]
#define s_r15  emu->regs[_r15].sqword[0]

#define s_rip  emu->regs[_rip].sqword[0]

/* bytes 0-3 */
#define s_eax  emu->regs[_rax].sdword[0]
#define s_ecx  emu->regs[_rcx].sdword[0]
#define s_edx  emu->regs[_rdx].sdword[0]
#define s_ebx  emu->regs[_rbx].sdword[0]
#define s_esp  emu->regs[_rsp].sdword[0]
#define s_ebp  emu->regs[_rbp].sdword[0]
#define s_esi  emu->regs[_rsi].sdword[0]
#define s_edi  emu->regs[_rdi].sdword[0]

#define s_r8d  emu->regs[_r8 ].sdword[0]
#define s_r9d  emu->regs[_r9 ].sdword[0]
#define s_r10d emu->regs[_r10].sdword[0]
#define s_r11d emu->regs[_r11].sdword[0]
#define s_r12d emu->regs[_r12].sdword[0]
#define s_r13d emu->regs[_r13].sdword[0]
#define s_r14d emu->regs[_r14].sdword[0]
#define s_r15d emu->regs[_r15].sdword[0]

#define s_eip  emu->regs[_rip].sdword[0]

/* bytes 0-1 */
#define s_ax   emu->regs[_rax].sword[0]
#define s_cx   emu->regs[_rcx].sword[0]
#define s_dx   emu->regs[_rdx].sword[0]
#define s_bx   emu->regs[_rbx].sword[0]
#define s_sp   emu->regs[_rsp].sword[0]
#define s_bp   emu->regs[_rbp].sword[0]
#define s_si   emu->regs[_rsi].sword[0]
#define s_di   emu->regs[_rdi].sword[0]

#define s_r8w  emu->regs[_r8 ].sword[0]
#define s_r9w  emu->regs[_r9 ].sword[0]
#define s_r10w emu->regs[_r10].sword[0]
#define s_r11w emu->regs[_r11].sword[0]
#define s_r12w emu->regs[_r12].sword[0]
#define s_r13w emu->regs[_r13].sword[0]
#define s_r14w emu->regs[_r14].sword[0]
#define s_r15w emu->regs[_r15].sword[0]

/* byte 0 */
#define s_al   emu->regs[_rax].sbyte[0]
#define s_cl   emu->regs[_rcx].sbyte[0]
#define s_dl   emu->regs[_rdx].sbyte[0]
#define s_bl   emu->regs[_rbx].sbyte[0]
#define s_spl  emu->regs[_rsp].sbyte[0]
#define s_bpl  emu->regs[_rbp].sbyte[0]
#define s_sil  emu->regs[_rsi].sbyte[0]
#define s_dil  emu->regs[_rdi].sbyte[0]

#define s_r8b  emu->regs[_r8 ].sbyte[0]
#define s_r9b  emu->regs[_r9 ].sbyte[0]
#define s_r10b emu->regs[_r10].sbyte[0]
#define s_r11b emu->regs[_r11].sbyte[0]
#define s_r12b emu->regs[_r12].sbyte[0]
#define s_r13b emu->regs[_r13].sbyte[0]
#define s_r14b emu->regs[_r14].sbyte[0]
#define s_r15b emu->regs[_r15].sbyte[0]

#endif /* __X64REGS_PRIVATE_H_ */