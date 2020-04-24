project(
    'Nemo Library', 
    'cpp', 
    version: '1.2.0', 
    license: 'MIT', 
    default_options: [
        'cpp_std=c++17'
    ]
)

thread_dep = dependency('threads')

inc = include_directories('include')
inc_tp = include_directories('ThreadPool')

subdir('ThreadPool')
subdir('include')
subdir('src')
subdir('main')
