set(EXTRA_FEAT_LIST)

if (OHOS_LLVM)
  list(APPEND EXTRA_FEAT_LIST "-DOHOS_LLVM")
endif()

clang_tablegen(AttrPCHRead.inc -gen-clang-attr-pch-read
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../
  ${EXTRA_FEAT_LIST}
  SOURCE ../Basic/Attr.td
  TARGET ClangAttrPCHRead)

clang_tablegen(AttrPCHWrite.inc -gen-clang-attr-pch-write
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../
  ${EXTRA_FEAT_LIST}
  SOURCE ../Basic/Attr.td
  TARGET ClangAttrPCHWrite)
