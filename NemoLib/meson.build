project('Nemo Library', 
        'c', 
        'cpp', 
        version : '1.2.0', 
        license : 'MIT', 
        default_options : ['c_std=c11', 'cpp_std=c++17']
       )

thread_dep = dependency('threads')

inc = include_directories('include')
inc_cuda = include_directories('../rand_graph')
nauty_inc = include_directories('nauty/include')

subdir('nauty')
subdir('include')
subdir('src')
subdir('main')
