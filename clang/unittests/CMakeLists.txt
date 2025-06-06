add_custom_target(ClangUnitTests)
set_target_properties(ClangUnitTests PROPERTIES FOLDER "Clang tests")

if(CLANG_BUILT_STANDALONE)
  # LLVMTestingSupport library is needed for some of the unittests.
  if (EXISTS ${LLVM_MAIN_SRC_DIR}/lib/Testing/Support
      AND NOT TARGET LLVMTestingSupport)
    add_subdirectory(${LLVM_MAIN_SRC_DIR}/lib/Testing/Support
      lib/Testing/Support)
  endif()
endif()

# add_clang_unittest(test_dirname file1.cpp file2.cpp)
#
# Will compile the list of files together and link against the clang
# Produces a binary named 'basename(test_dirname)'.
function(add_clang_unittest test_dirname)
  add_unittest(ClangUnitTests ${test_dirname} ${ARGN})
endfunction()

add_subdirectory(Basic)
add_subdirectory(Lex)
add_subdirectory(Driver)
if(CLANG_ENABLE_STATIC_ANALYZER)
  add_subdirectory(Analysis)
  add_subdirectory(StaticAnalyzer)
endif()
add_subdirectory(ASTMatchers)
add_subdirectory(AST)
add_subdirectory(CrossTU)
add_subdirectory(Tooling)
add_subdirectory(Introspection)
add_subdirectory(Format)
add_subdirectory(Frontend)
add_subdirectory(Rewrite)
add_subdirectory(Sema)
add_subdirectory(CodeGen)
add_subdirectory(Interpreter)
# FIXME: libclang unit tests are disabled on Windows due
# to failures, mostly in libclang.VirtualFileOverlay_*.
if(NOT WIN32 AND CLANG_TOOL_LIBCLANG_BUILD) 
  add_subdirectory(libclang)
endif()
if (NOT APPLE)
  add_subdirectory(DirectoryWatcher)
endif()
add_subdirectory(Rename)
add_subdirectory(Index)
add_subdirectory(Serialization)
