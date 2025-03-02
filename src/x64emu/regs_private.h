#ifndef __X64REGS_PRIVATE_H_
#define __X64REGS_PRIVATE_H_

enum {
    _rax, _rcx, _rdx, _rbx,
    _rsp, _rbp, _rsi, _rdi,
    _r8,  _r9,  _r10, _r11,
    _r12, _r13, _r14, _r15
};

/* macros used to directly access registers internally. */

/* unsigned */

/* 8-byte registers */
#define r_rax  emu->regs[_rax].uq[0]
#define r_rcx  emu->regs[_rcx].uq[0]
#define r_rdx  emu->regs[_rdx].uq[0]
#define r_rbx  emu->regs[_rbx].uq[0]
#define r_rsp  emu->regs[_rsp].uq[0]
#define r_rbp  emu->regs[_rbp].uq[0]
#define r_rsi  emu->regs[_rsi].uq[0]
#define r_rdi  emu->regs[_rdi].uq[0]

#define r_r8   emu->regs[_r8 ].uq[0]
#define r_r9   emu->regs[_r9 ].uq[0]
#define r_r10  emu->regs[_r10].uq[0]
#define r_r11  emu->regs[_r11].uq[0]
#define r_r12  emu->regs[_r12].uq[0]
#define r_r13  emu->regs[_r13].uq[0]
#define r_r14  emu->regs[_r14].uq[0]
#define r_r15  emu->regs[_r15].uq[0]

#define r_rip  emu->rip.uq[0]
#define r_flags emu->flags.uq[0]

/* bytes 0-3 */
#define r_eax  emu->regs[_rax].ud[0]
#define r_ecx  emu->regs[_rcx].ud[0]
#define r_edx  emu->regs[_rdx].ud[0]
#define r_ebx  emu->regs[_rbx].ud[0]
#define r_esp  emu->regs[_rsp].ud[0]
#define r_ebp  emu->regs[_rbp].ud[0]
#define r_esi  emu->regs[_rsi].ud[0]
#define r_edi  emu->regs[_rdi].ud[0]

#define r_r8d  emu->regs[_r8 ].ud[0]
#define r_r9d  emu->regs[_r9 ].ud[0]
#define r_r10d emu->regs[_r10].ud[0]
#define r_r11d emu->regs[_r11].ud[0]
#define r_r12d emu->regs[_r12].ud[0]
#define r_r13d emu->regs[_r13].ud[0]
#define r_r14d emu->regs[_r14].ud[0]
#define r_r15d emu->regs[_r15].ud[0]

#define r_eip  emu->rip.ud[0]
#define r_eflags emu->flags.ud[0]

/* bytes 0-1 */
#define r_ax   emu->regs[_rax].uw[0]
#define r_cx   emu->regs[_rcx].uw[0]
#define r_dx   emu->regs[_rdx].uw[0]
#define r_bx   emu->regs[_rbx].uw[0]
#define r_sp   emu->regs[_rsp].uw[0]
#define r_bp   emu->regs[_rbp].uw[0]
#define r_si   emu->regs[_rsi].uw[0]
#define r_di   emu->regs[_rdi].uw[0]

#define r_r8w  emu->regs[_r8 ].uw[0]
#define r_r9w  emu->regs[_r9 ].uw[0]
#define r_r10w emu->regs[_r10].uw[0]
#define r_r11w emu->regs[_r11].uw[0]
#define r_r12w emu->regs[_r12].uw[0]
#define r_r13w emu->regs[_r13].uw[0]
#define r_r14w emu->regs[_r14].uw[0]
#define r_r15w emu->regs[_r15].uw[0]

/* byte 0 */
#define r_al   emu->regs[_rax].ub[0]
#define r_cl   emu->regs[_rcx].ub[0]
#define r_dl   emu->regs[_rdx].ub[0]
#define r_bl   emu->regs[_rbx].ub[0]
#define r_spl  emu->regs[_rsp].ub[0]
#define r_bpl  emu->regs[_rbp].ub[0]
#define r_sil  emu->regs[_rsi].ub[0]
#define r_dil  emu->regs[_rdi].ub[0]

#define r_r8b  emu->regs[_r8 ].ub[0]
#define r_r9b  emu->regs[_r9 ].ub[0]
#define r_r10b emu->regs[_r10].ub[0]
#define r_r11b emu->regs[_r11].ub[0]
#define r_r12b emu->regs[_r12].ub[0]
#define r_r13b emu->regs[_r13].ub[0]
#define r_r14b emu->regs[_r14].ub[0]
#define r_r15b emu->regs[_r15].ub[0]

/* byte 1 */
#define r_ah   emu->regs[_rax].ub[1]
#define r_ch   emu->regs[_rcx].ub[1]
#define r_dh   emu->regs[_rdx].ub[1]
#define r_bh   emu->regs[_rbx].ub[1]
#define r_sph  emu->regs[_rsp].ub[1]
#define r_bph  emu->regs[_rbp].ub[1]
#define r_sih  emu->regs[_rsi].ub[1]
#define r_dih  emu->regs[_rdi].ub[1]

/* signed */

/* 8-byte registers */
#define s_rax  emu->regs[_rax].sq[0]
#define s_rcx  emu->regs[_rcx].sq[0]
#define s_rdx  emu->regs[_rdx].sq[0]
#define s_rbx  emu->regs[_rbx].sq[0]
#define s_rsp  emu->regs[_rsp].sq[0]
#define s_rbp  emu->regs[_rbp].sq[0]
#define s_rsi  emu->regs[_rsi].sq[0]
#define s_rdi  emu->regs[_rdi].sq[0]

#define s_r8   emu->regs[_r8 ].sq[0]
#define s_r9   emu->regs[_r9 ].sq[0]
#define s_r10  emu->regs[_r10].sq[0]
#define s_r11  emu->regs[_r11].sq[0]
#define s_r12  emu->regs[_r12].sq[0]
#define s_r13  emu->regs[_r13].sq[0]
#define s_r14  emu->regs[_r14].sq[0]
#define s_r15  emu->regs[_r15].sq[0]

#define s_rip  emu->regs[_rip].sq[0]

/* bytes 0-3 */
#define s_eax  emu->regs[_rax].sd[0]
#define s_ecx  emu->regs[_rcx].sd[0]
#define s_edx  emu->regs[_rdx].sd[0]
#define s_ebx  emu->regs[_rbx].sd[0]
#define s_esp  emu->regs[_rsp].sd[0]
#define s_ebp  emu->regs[_rbp].sd[0]
#define s_esi  emu->regs[_rsi].sd[0]
#define s_edi  emu->regs[_rdi].sd[0]

#define s_r8d  emu->regs[_r8 ].sd[0]
#define s_r9d  emu->regs[_r9 ].sd[0]
#define s_r10d emu->regs[_r10].sd[0]
#define s_r11d emu->regs[_r11].sd[0]
#define s_r12d emu->regs[_r12].sd[0]
#define s_r13d emu->regs[_r13].sd[0]
#define s_r14d emu->regs[_r14].sd[0]
#define s_r15d emu->regs[_r15].sd[0]

#define s_eip  emu->regs[_rip].sd[0]

/* bytes 0-1 */
#define s_ax   emu->regs[_rax].sw[0]
#define s_cx   emu->regs[_rcx].sw[0]
#define s_dx   emu->regs[_rdx].sw[0]
#define s_bx   emu->regs[_rbx].sw[0]
#define s_sp   emu->regs[_rsp].sw[0]
#define s_bp   emu->regs[_rbp].sw[0]
#define s_si   emu->regs[_rsi].sw[0]
#define s_di   emu->regs[_rdi].sw[0]

#define s_r8w  emu->regs[_r8 ].sw[0]
#define s_r9w  emu->regs[_r9 ].sw[0]
#define s_r10w emu->regs[_r10].sw[0]
#define s_r11w emu->regs[_r11].sw[0]
#define s_r12w emu->regs[_r12].sw[0]
#define s_r13w emu->regs[_r13].sw[0]
#define s_r14w emu->regs[_r14].sw[0]
#define s_r15w emu->regs[_r15].sw[0]

/* byte 0 */
#define s_al   emu->regs[_rax].sb[0]
#define s_cl   emu->regs[_rcx].sb[0]
#define s_dl   emu->regs[_rdx].sb[0]
#define s_bl   emu->regs[_rbx].sb[0]
#define s_spl  emu->regs[_rsp].sb[0]
#define s_bpl  emu->regs[_rbp].sb[0]
#define s_sil  emu->regs[_rsi].sb[0]
#define s_dil  emu->regs[_rdi].sb[0]

#define s_r8b  emu->regs[_r8 ].sb[0]
#define s_r9b  emu->regs[_r9 ].sb[0]
#define s_r10b emu->regs[_r10].sb[0]
#define s_r11b emu->regs[_r11].sb[0]
#define s_r12b emu->regs[_r12].sb[0]
#define s_r13b emu->regs[_r13].sb[0]
#define s_r14b emu->regs[_r14].sb[0]
#define s_r15b emu->regs[_r15].sb[0]

/* byte 1 */
#define s_ah   emu->regs[_rax].sb[1]
#define s_ch   emu->regs[_rcx].sb[1]
#define s_dh   emu->regs[_rdx].sb[1]
#define s_bh   emu->regs[_rbx].sb[1]
#define s_sph  emu->regs[_rsp].sb[1]
#define s_bph  emu->regs[_rbp].sb[1]
#define s_sih  emu->regs[_rsi].sb[1]
#define s_dih  emu->regs[_rdi].sb[1]

#endif /* __X64REGS_PRIVATE_H_ */