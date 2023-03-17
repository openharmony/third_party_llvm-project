// Make sure that for BareMetal toolchain, we able to
// disable the use of integrated assembler.
// RUN: %clang -Werror --target="arm-none-eabi" -fno-integrated-as -c %s
