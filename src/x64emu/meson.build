
x64emu_src = [
    'x64context.c',
    'x64decode_0f.c',
    'x64decode.c',
    'x64emu.c',
    'x64execute_0f.c',
    'x64execute.c',
    'x64modrm.c',
    'x64stack.c',
    'x64syscall.c'
]

libx64emu = static_library(
    'x64emu',
    sources: x64emu_src,
    include_directories: [
        inc
    ]
)
