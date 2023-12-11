import os

def get_ohos_flags(out_path, flag_type):
    oh_ninja_path = os.path.join(out_path, 'obj', 'toolchain', 'llvm-project', 'llvm-build', 'llvm_test.ninja')
    with open(oh_ninja_path, 'r') as file:
        lines = file.readlines()
    for line in lines:
        line = line.lstrip()
        if line.startswith(flag_type):
            flag_line = line.replace(flag_type, "")
            flag_list = list(tuple(flag_line.split( )))
            flag_list = sorted(set(flag_list), key=flag_list.index)
            return flag_list

def get_ohos_cflags(out_path, llvm_cflags):
    cflag_list = get_ohos_flags(out_path, 'cflags = ')
    filter_list = [
        '-Xclang',
        '-mllvm',
        '-instcombine-lower-dbg-declare=0',
        '-ffunction-sections',
        '-Werror',
        '-g2',
    ]

    filter_re = [
        '--target=',
        '-march=',
        '-mfloat-abi=',
        '-mtune=',
    ]
    for filter_str in filter_re:
        for cflag in cflag_list:
            if cflag.startswith(filter_str) or cflag in filter_list:
                cflag_list.remove(cflag)

    if llvm_cflags:
        cflag_list.extend(llvm_cflags)
    return sorted(set(cflag_list), key=cflag_list.index)
   
def get_ohos_ldflags(out_path, llvm_ldflags):
    ldflag_list = get_ohos_flags(out_path, 'ldflags = ')
    filter_list = [
        '-Wl,--exclude-libs=libunwind_llvm.a',
        '-Wl,--exclude-libs=libc++_static.a',
        '-Wl,--exclude-libs=libvpx_assembly_arm.a',
        '--target=arm-linux-ohos',
        '-Werror',
        '--sysroot=obj/third_party/musl',
    ]
    for ldflag in filter_list:
        if ldflag in ldflag_list:
            ldflag_list.remove(ldflag)

    if llvm_ldflags:
        ldflag_list.extend(llvm_ldflags)
    return sorted(set(ldflag_list), key=ldflag_list.index)
