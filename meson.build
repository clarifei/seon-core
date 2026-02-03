project(
    'seon',
    'cpp',
    version: '0.1.0',
    default_options: [
        'cpp_std=c++20',
        'warning_level=2',
    ],
    meson_version: '>=0.58.0',
)

subdir('tools/sigtool')

# Build main application in output directory
seon_exe = executable('seon',
    files('src/main.cpp'),
    win_subsystem: 'console',
    install: false,
)

subdir('protector')

build_dir = meson.current_build_dir()

gensig_script = find_program('scripts/gensig.bat')

# Generate signature header
signature_header = custom_target(
    'signature_data_h',
    output: 'signature_data.h',
    input: seon_exe,
    command: [
        gensig_script,
        '@INPUT@',
        '@OUTPUT@',
        'seon',
    ],
    depends: [seon_exe, sigtool],
    build_by_default: true,
)

protector_incs = include_directories(
    'protector/include',
    'protector/src',
    '.',
)

# Build seon.dll
seon_dll = shared_library('seon',
    files(
        'protector/src/dllmain.cpp',
        'protector/src/checker.cpp',
    ),
    signature_header,
    include_directories: protector_incs,
    cpp_args: [
        '-D_CRT_SECURE_NO_WARNINGS',
        '-DSEON_HAS_SIGNATURE',
    ],
    link_args: [
        '-ladvapi32',
    ],
    build_by_default: true,
)

run_target('run',
    command: [seon_exe],
    depends: [seon_exe, seon_dll],
)

summary({
    'Version': meson.project_version(),
    'Build type': get_option('buildtype'),
    'C++ Standard': 'C++20',
}, section: 'Project')

summary({
    'seon.exe': 'Protected application',
    'sigtool.exe': 'Signature generator tool',
    'seon.dll': 'Integrity checker DLL',
    'signature_data.h': 'Generated hash header',
}, section: 'Build Outputs')
