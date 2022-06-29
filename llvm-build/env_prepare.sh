llvm_name=llvm
cmake_name=linux-x86
clang_name=clang+llvm-10.0.1-x86_64-linux-gnu-ubuntu-16.04
llvm_dir=clang/ohos/linux-x86_64
cmake_dir=cmake
cd prebuilts
mkdir -p $llvm_dir
mkdir -p $cmake_dir

wget https://mirrors.huaweicloud.com/harmonyos/compiler/clang/10.0.1-62608/linux/llvm.tar.gz
wget https://mirrors.huaweicloud.com/harmonyos/compiler/cmake/3.16.5/linux/cmake-linux-x86-3.16.5.tar.gz
#wget https://github.com/llvm/llvm-project/releases/download/llvmorg-10.0.1/clang+llvm-10.0.1-x86_64-linux-gnu-ubuntu-16.04.tar.xz
tar -xvf llvm.tar.gz
tar -xvf cmake-linux-x86-3.16.5.tar.gz
tar -xvf clang+llvm-10.0.1-x86_64-linux-gnu-ubuntu-16.04.tar.xz
mv $llvm_name $llvm_dir/llvm
mv $cmake_name $cmake_dir/linux-x86
mv $clang_name $llvm_dir/clang-10.0.1
