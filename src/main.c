#include <stdio.h>

#include "elfloader.h"
#include "x64context.h"
#include "x64emu.h"

int main(int argc, char *argv[], char *envp[]) {
    if (argc == 1) {
        printf("Usage: %s <path/to/binary> [args]\n", argv[0]);
        return 1;
    }

    x64context_t ctx = { 0 };
    if (!x64context_init(&ctx, argc, argv, envp)) {
        return 1;
    }

    if (!elfloader_load(&ctx, argv[1])) {
        return 1;
    }

    x64emu_t emu = { 0 };
    if (!x64emu_init(&emu, &ctx)) {
        return 1;
    }

    x64emu_run(&emu);

    int ret = 0;

    if (!x64emu_free(&emu)) ret = 1;

    return ret;
}
