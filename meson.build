project('flux64', 'c',
    version: '0.1'
)

if get_option('buildtype') == 'debug'
    add_project_arguments('-DHAVE_TRACE', language : 'c')
endif

subdir('src')
