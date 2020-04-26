project(
    'Nemo Library', 
    'cpp', 
    version: '1.2.0', 
    license: 'MIT', 
    default_options: [
        'cpp_std=c++17'
    ]
)

add_global_arguments('-DNDEBUG=1', language: ['c', 'cpp'])

thread_dep = dependency('threads')

inc = include_directories('include')
inc_tp = include_directories('ThreadPool')


loguru_project = subproject('loguru')
loguru_dep = loguru_project.get_variable('loguru_dep')

subdir('ThreadPool')
subdir('include')
subdir('src')
subdir('main')
