//===-- Config.h -----------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_HOST_CONFIG_H
#define LLDB_HOST_CONFIG_H

#cmakedefine01 LLDB_EDITLINE_USE_WCHAR

#cmakedefine01 LLDB_HAVE_EL_RFUNC_T

#cmakedefine01 HAVE_SYS_EVENT_H

#cmakedefine01 HAVE_PPOLL

#cmakedefine01 HAVE_PTSNAME_R

#cmakedefine01 HAVE_PROCESS_VM_READV

#cmakedefine01 HAVE_NR_PROCESS_VM_READV

#ifndef HAVE_LIBCOMPRESSION
#cmakedefine HAVE_LIBCOMPRESSION
#endif

#cmakedefine01 LLDB_ENABLE_POSIX

#cmakedefine01 LLDB_ENABLE_TERMIOS

#cmakedefine01 LLDB_ENABLE_LZMA
// OHOS_LOCAL
#cmakedefine01 LLDB_ENABLE_LZMA_7ZIP

#cmakedefine01 LLDB_ENABLE_CURSES

#cmakedefine01 CURSES_HAVE_NCURSES_CURSES_H

#cmakedefine01 LLDB_ENABLE_LIBEDIT

#cmakedefine01 LLDB_ENABLE_LIBXML2

#cmakedefine01 LLDB_ENABLE_LUA

#cmakedefine01 LLDB_ENABLE_PYTHON

#cmakedefine01 LLDB_ENABLE_FBSDVMCORE

#cmakedefine01 LLDB_EMBED_PYTHON_HOME

// OHOS_LOCAL begin
/* Define to 1 If the tool timeout automatic exit option is enabled. */
#cmakedefine01 LLDB_ENABLE_TIMEOUT
// OHOS_LOCAL end

#cmakedefine LLDB_PYTHON_HOME R"(${LLDB_PYTHON_HOME})"

#define LLDB_LIBDIR_SUFFIX "${LLVM_LIBDIR_SUFFIX}"

#cmakedefine LLDB_GLOBAL_INIT_DIRECTORY R"(${LLDB_GLOBAL_INIT_DIRECTORY})"

#endif // #ifndef LLDB_HOST_CONFIG_H
