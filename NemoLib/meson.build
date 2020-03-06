project(
        'Nemo Library', 
#        'c', 
        'cpp', 
        version : '1.2.0', 
        license : 'MIT', 
        default_options : [
                #'c_std=c11', 
                'cpp_std=c++17'
        ]
)

thread_dep = dependency('threads')

inc = include_directories('include')
inc_cuda = include_directories('../rand_graph')
inc_tp = include_directories('ThreadPool')

subdir('ThreadPool')
subdir('include')
subdir('src')
subdir('main')
