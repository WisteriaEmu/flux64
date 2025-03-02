
x64emu_src = [
    'context.c',
    'decode_0f.c',
    'decode.c',
    'emu.c',
    'execute_0f.c',
    'execute.c',
    'modrm.c',
    'stack.c',
    'syscall.c'
]

libx64emu = static_library(
    'x64emu',
    sources: x64emu_src,
    include_directories: [
        inc
    ]
)
