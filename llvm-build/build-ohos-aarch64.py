import os

from build import BuildConfig, BuildUtils

def main():
    print('Start building LLVM toolchain for OHOS AArch64 host')
    build_config = BuildConfig()
    build_utils = BuildUtils(build_config)

    llvm_project_path = os.path.abspath(os.path.join(build_config.LLVM_PROJECT_DIR, 'llvm'))
    llvm_path = build_utils.merge_out_path('ohos-aarch64')
    llvm_install = build_utils.merge_out_path('ohos-aarch64-install')
    llvm_root = build_utils.merge_out_path('llvm-install')
    sysroot = build_utils.merge_out_path('sysroot')

    llvm_triple = 'aarch64-linux-ohos'

    cflags = [ '-v',
               '-fstack-protector-strong',
               '-fdata-sections',
               '-ffunction-sections',
               '-funwind-tables',
               '-no-canonical-prefixes',
               '-D__MUSL__',
               '-target %s' % llvm_triple,
             ]

    cflags_debug = '-O0 -g -fno-limit-debug-info'
    cflags_release = '-O2 -DNDEBUG'

    ldflags = [ '-fuse-ld=lld',
                '--rtlib=compiler-rt',
                '-lunwind',
                '-stdlib=libc++',
                '-static-libstdc++',
                '-pie',
                '-Wl,--build-id=sha1',
                '-Wl,-z,relro,-z,now',
              ]

    if build_config.strip:
        ldflags.append('-s')

    llvm_extra_env = {}
    llvm_extra_env['LD_LIBRARY_PATH'] = os.path.join(llvm_root, 'lib')
    env = dict(build_config.ORIG_ENV)
    if llvm_extra_env is not None:
        env.update(llvm_extra_env)

    llvm_defines = {}
    llvm_defines['CMAKE_SYSTEM_NAME'] = 'OHOS'
    llvm_defines['CMAKE_CROSSCOMPILING'] = 'True'
    llvm_defines['CMAKE_INSTALL_PREFIX'] = llvm_install
    llvm_defines['CMAKE_SYSROOT'] = sysroot
    llvm_defines['CMAKE_LIBRARY_ARCHITECTURE'] = llvm_triple
    llvm_defines['LLVM_TARGETS_TO_BUILD'] = 'AArch64'
    llvm_defines['LLVM_TARGET_ARCH'] = 'AArch64'
    llvm_defines['LLVM_DEFAULT_TARGET_TRIPLE'] = llvm_triple
    llvm_defines['LLVM_BUILD_LLVM_DYLIB'] = 'ON'
    llvm_defines['LLVM_ENABLE_FFI'] = 'OFF'
    llvm_defines['LLVM_ENABLE_TERMINFO'] = 'OFF'
    llvm_defines['LLVM_INCLUDE_BENCHMARKS'] = 'OFF'
    llvm_defines['LLVM_INCLUDE_EXAMPLES'] = 'OFF'
    llvm_defines['LLVM_INCLUDE_TESTS'] = 'OFF'
    llvm_defines['LLVM_BUILD_TOOLS'] = 'ON'
    llvm_defines['LLVM_ENABLE_ZLIB'] = 'OFF'
    llvm_defines['LLVM_ENABLE_PROJECTS'] = 'clang;lld;clang-tools-extra'
    llvm_defines['LLVM_CONFIG_PATH'] = os.path.join(llvm_root, 'bin', 'llvm-config')
    llvm_defines['LLVM_TABLEGEN'] = os.path.join(llvm_root, 'bin', 'llvm-tblgen')
    llvm_defines['CMAKE_C_COMPILER_EXTERNAL_TOOLCHAIN'] = llvm_root
    llvm_defines['CMAKE_CXX_COMPILER_EXTERNAL_TOOLCHAIN'] = llvm_root
    llvm_defines['CMAKE_ASM_COMPILER_EXTERNAL_TOOLCHAIN'] = llvm_root
    llvm_defines['CMAKE_C_COMPILER'] = os.path.join(llvm_root, 'bin', 'clang')
    llvm_defines['CMAKE_CXX_COMPILER'] = os.path.join(llvm_root, 'bin', 'clang++')
    llvm_defines['CMAKE_AR'] = os.path.join(llvm_root, 'bin', 'llvm-ar')
    llvm_defines['CMAKE_NM'] = os.path.join(llvm_root, 'bin', 'llvm-nm')
    llvm_defines['CMAKE_RANLIB'] = os.path.join(llvm_root, 'bin', 'llvm-ranlib')
    llvm_defines['CMAKE_OBJCOPY'] = os.path.join(llvm_root, 'bin', 'llvm-objcopy')
    llvm_defines['CMAKE_OBJDUMP'] = os.path.join(llvm_root, 'bin', 'llvm-objdump')
    llvm_defines['CMAKE_READELF'] = os.path.join(llvm_root, 'bin', 'llvm-readelf')
    llvm_defines['CMAKE_STRIP'] = os.path.join(llvm_root, 'bin', 'llvm-strip')
    llvm_defines['CMAKE_LINKER'] = os.path.join(llvm_root, 'bin', 'ld.lld')
    llvm_defines['CMAKE_ASM_FLAGS'] = ' '.join(cflags)
    llvm_defines['CMAKE_C_FLAGS'] = ' '.join(cflags)
    llvm_defines['CMAKE_CXX_FLAGS'] = ' '.join(cflags) + ' -stdlib=libc++'
    llvm_defines['CMAKE_EXE_LINKER_FLAGS'] = ' '.join(ldflags) + ' -Wl,--gc-sections'
    llvm_defines['CMAKE_SHARED_LINKER_FLAGS'] = ' '.join(ldflags)
    llvm_defines['CMAKE_MODULE_LINKER_FLAGS'] = ' '.join(ldflags)
    llvm_defines['CMAKE_FIND_ROOT_PATH_MODE_INCLUDE'] = 'ONLY'
    llvm_defines['CMAKE_FIND_ROOT_PATH_MODE_LIBRARY'] = 'ONLY'
    llvm_defines['CMAKE_FIND_ROOT_PATH_MODE_PACKAGE'] = 'ONLY'
    llvm_defines['CMAKE_FIND_ROOT_PATH_MODE_PROGRAM'] = 'NEVER'
    llvm_defines['CMAKE_POSITION_INDEPENDENT_CODE'] = 'True'
    llvm_defines['Python3_EXECUTABLE'] = os.path.join(build_utils.get_python_dir(), 'bin', build_config.LLDB_PYTHON)
    llvm_defines['CMAKE_C_FLAGS_DEBUG'] = cflags_debug
    llvm_defines['CMAKE_CXX_FLAGS_DEBUG'] = cflags_debug
    llvm_defines['CMAKE_ASM_FLAGS_DEBUG'] = cflags_debug
    llvm_defines['CMAKE_C_FLAGS_RELEASE'] = cflags_release
    llvm_defines['CMAKE_CXX_FLAGS_RELEASE'] = cflags_release
    llvm_defines['CMAKE_ASM_FLAGS_RELEASE'] = cflags_release

    if build_config.enable_assertions:
        llvm_defines['LLVM_ENABLE_ASSERTIONS'] = 'ON'

    if build_config.debug:
        llvm_defines['CMAKE_BUILD_TYPE'] = 'Debug'
    else:
        llvm_defines['CMAKE_BUILD_TYPE'] = 'Release'

    build_utils.invoke_cmake(llvm_project_path, llvm_path, llvm_defines, env=env)

    build_utils.invoke_ninja(out_path=llvm_path, env=env, target=None, install=True)

    # Copy required aarch64-linux-ohos libs from main toolchain build.
    build_utils.check_copy_tree(os.path.join(llvm_root, 'lib', llvm_triple),
                                os.path.join(llvm_install, 'lib', llvm_triple))
    build_utils.check_copy_tree(os.path.join(llvm_root, 'lib', 'clang', '15.0.4', 'lib', llvm_triple),
                                os.path.join(llvm_install, 'lib', 'clang', '15.0.4', 'lib', llvm_triple))

if __name__ == '__main__':
    main()
