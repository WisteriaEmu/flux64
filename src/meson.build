
inc = include_directories(
    'include',
    'elfloader/include',
    'x64emu/include',
    'platform/include'
)

subdir('platform')
subdir('x64emu')
subdir('wrapper')
subdir('elfloader')

flux64_src = [
    'main.c'
]

executable(
    'flux64',
    sources: flux64_src,
    include_directories: inc,
    link_with: [libelfloader, libx64emu, libplatform],
    # link_args: ['-Wl,-Ttext-segment=0x34800000']
)
