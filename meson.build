project('mixa', 'c', 'cpp'
    , default_options: ['c_std=c11'
                       , 'cpp_std=c++20'
                       , 'buildtype=debugoptimized'
                       , 'warning_level=3']
    , license : 'GPL-3.0'
    , license_files : 'LICENSE'
    , meson_version: '>=1.3.0'
    , version: '0.1')
project_description = 'Audio mixer application Maintained: RolandPf67@googlemail.com'
fs = import('fs')

pkgdatadir = join_paths(get_option('prefix'), get_option('datadir'), meson.project_name())
pkglocaledir = join_paths(get_option('prefix'), get_option('datadir'), 'locale')
pkgsrcdir = fs.relative_to( meson.global_source_root(), meson.current_build_dir()) + '/src'
add_project_arguments('-DPACKAGE_DATA_DIR="' + pkgdatadir + '"', language: ['cpp','c'])
add_project_arguments('-DPACKAGE="' + meson.project_name() + '"', language: ['cpp','c'])
# this is to recreate the conventions from autotools
add_project_arguments('-DPACKAGE_SRC_DIR="' + pkgsrcdir + '"', language: ['cpp','c'])

compiler_add_flags = [  # limit the compiler noisiness
     '-Wno-unused-parameter'    # limit the compiler noisiness
    ,'-Wno-overlength-strings'  # this matters only if you are pedantic
    ]
cc = meson.get_compiler('c')
foreach ccflag : compiler_add_flags
    if cc.has_argument(ccflag)
        add_project_arguments(ccflag, language: ['c', 'cpp'])
    endif
endforeach

libharu_deps = declare_dependency(dependencies : cc.find_library('hpdf', required: false))


conf = configuration_data()
conf.set_quoted('MIXA_VERSION', meson.project_version())     # Surround the version in quotes to make it a C string
conf.set       ('KEYBINDER', get_option('keybind'))
conf_file = configure_file(output : meson.project_name() + '_config.h'
               , configuration : conf)

gtkmm3_deps     = dependency('gtkmm-3.0')
glibmm2_deps    = dependency('glibmm-2.4 giomm-2.4')
alsa_deps       = dependency('alsa')
thread_deps     = dependency('threads')
keybind_deps    = declare_dependency()
if get_option('keybind')
   keybind_deps = dependency('keybinder-3.0')
endif
deps = [
      thread_deps
    , gtkmm3_deps
    , glibmm2_deps
    , alsa_deps
    , keybind_deps
    ]
subdir('src')
subdir('res')
sources += app_resources

inc_dir = include_directories(fs.parent(conf_file))
project_target = executable(meson.project_name()
    , sources
    , dependencies: deps
    , include_directories: inc_dir
    , install: true
    , win_subsystem: 'windows'  # disable this to see console output for windows
    )
