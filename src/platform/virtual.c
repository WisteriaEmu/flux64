#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "debug.h"
#include "virtual.h"

SET_DEBUG_CHANNEL("VIRTUAL")

static inline FILE *open_maps(void) {
    FILE *fd = fopen("/proc/self/maps", "r");
    if (!fd) {
        log_err("Failed to open /proc/self/maps. Are you playing games?");
    }
    return fd;
}

bool detect_48bit_va(void) {
    FILE *maps = open_maps();
    if (!maps) return false;

    uintptr_t max_address = 0;
    char line[256];

    while (fgets(line, sizeof(line), maps)) {
        uintptr_t start, end;
        if (sscanf(line, "%lx-%lx", &start, &end) == 2) {
            max_address = end;
        }
    }
    fclose(maps);

    /* not using bitwise logic: harder to read, same instructions */
    if (max_address > (1UL << 39) - 1) {
        /* does not fit in 39 bits */
        log_debug("Detected 48 bit virtual address space");
        return true;
    } else {
        log_debug("Did not detect 48 bit virtual address space, considering it is 39 bits");
    }
    return false;
}

bool dump_self_maps(void) {
    FILE *maps = open_maps();
    if (!maps) return false;

    log_dump("Dumped /proc/self/maps:");

    char line[256];

    while (fgets(line, sizeof(line), maps)) {
        int len = strlen(line);
        if (len) line[len - 1] = 0;
        log_dump("%s", line);
    }
    fclose(maps);

    return true;
}
