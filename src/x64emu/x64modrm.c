#include <stdbool.h>

#include "debug.h"
#include "x64emu.h"
#include "x64instr.h"
#include "x64modrm.h"
#include "x64regs_private.h"

SET_DEBUG_CHANNEL("X64MODRM")

void  x64modrm_fetch(x64emu_t *emu, x64instr_t *ins) {
    ins->modrm.byte = fetch_8();
}

void *x64modrm_get_reg(x64emu_t *emu, x64instr_t *ins) {
    switch (ins->modrm.mod) {
        case 0x3:
            return emu->regs + (ins->modrm.reg | (ins->rex.b << 3));
        default:
            log_err("Unimplemented mod %02X", ins->modrm.byte);
            break;
    }
    return NULL;
}

void *x64modrm_get_rm(x64emu_t *emu, x64instr_t *ins) {
    switch (ins->modrm.mod) {
        case 0x3:
            return emu->regs + (ins->modrm.rm | (ins->rex.b << 3));
        default:
            log_err("Unimplemented mod %02X", ins->modrm.byte);
            break;
    }
    return NULL;
}
