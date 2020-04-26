nemolib_sources = [
    'Graph.cpp',
    'NautyLink.cpp',
    'RandomGraphAnalysis.cpp',
    'RandomGraphGenerator.cpp',
    'SubgraphProfile.cpp'
]

nemolib_inc = [
    inc, 
    inc_tp
]

nemolib_dependencies = [
    thread_dep,
    loguru_dep
]

nemolib_lib = library(
    'nemolib',
    nemolib_sources,
    include_directories: nemolib_inc,
    install: true,
    dependencies: nemolib_dependencies
)

# dependency for use with other meson builds
nemolib_dep = declare_dependency(
    link_with : nemolib_lib, 
    include_directories : nemolib_inc
)
