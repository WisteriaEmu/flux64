#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>

#include "debug.h"
#include "x64emu.h"

#include "regs_private.h"

SET_DEBUG_CHANNEL("X64SYSCALL")

bool x64syscall(x64emu_t *emu) {
    /* The syscall is determined by rax, arguments are passed through
       rdi, rsi, rdx, r10, r8 and r9 registers accordingly. */

    log_dump("Syscall %lX", r_rax);

    r_rcx = r_rip;
    r_r11 = r_flags;
    r_flags = 0; /* FIXME: Maybe implement IA32_FMASK? */

    switch (r_rax) {
        case 0x01:            /* SYS_write */
            s_rax = write(s_edi, (void *)r_rsi, (size_t)r_rdx);
            if (s_rax == -1)
                s_rax = -errno;
            break;

        case 0x3C: {          /* SYS_exit */
            int status = s_edi;
            x64emu_free(emu);
            _exit(status);
        }

        default:
            log_err("Unimplemented syscall 0x%lx", r_rax);
            return false;
    }
    return true;
}
