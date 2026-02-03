sigtool_src = files('src/sigtool.cpp')

sigtool = executable('sigtool',
    sigtool_src,
    native: true,
    cpp_args: ['-O2'],
)

sigtool_dep = declare_dependency(
    sources: sigtool_src,
)
