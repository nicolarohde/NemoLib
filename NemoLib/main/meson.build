
test_bin = executable('nemolib', 'Source.cpp',
                     include_directories : [inc, inc_tp],
                     link_with : nemolib_lib,
                     dependencies : thread_dep
)

cli_bin = executable('nemocollection', 'nemocollection.cpp',
                     include_directories : [inc, inc_tp],
                     link_with : nemolib_lib,
                     dependencies : thread_dep
)
