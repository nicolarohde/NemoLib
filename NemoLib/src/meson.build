nemolib_sources = [
    'Graph.cpp',
    'NautyLink.cpp',
    #'RandESU.cpp',
    'RandomGraphAnalysis.cpp',
    'RandomGraphGenerator.cpp',
    'SubgraphProfile.cpp'
]

nemolib_lib = shared_library('nemolib',
                             nemolib_sources,
                             # only use this if the cuda RT is installed and you want to utilize CUDA:
                             #include_directories : [inc, inc_cuda],
                             # use this in general:
                             include_directories : [inc, inc_tp],
                             install : true,
#                             link_with : nauty_lib,
                             dependencies : thread_dep
)

