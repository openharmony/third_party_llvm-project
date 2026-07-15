set(EXTRA_FEAT_LIST)

if (OHOS_LLVM)
  list(APPEND EXTRA_FEAT_LIST "-DOHOS_LLVM")
endif()

clang_tablegen(Checkers.inc -gen-clang-sa-checkers
  ${EXTRA_FEAT_LIST}
  SOURCE Checkers.td
  TARGET ClangSACheckers)
