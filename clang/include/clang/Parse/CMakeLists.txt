set(EXTRA_FEAT_LIST)

if (OHOS_LLVM)
  list(APPEND EXTRA_FEAT_LIST "-DOHOS_LLVM")
endif()

clang_tablegen(AttrParserStringSwitches.inc -gen-clang-attr-parser-string-switches
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../
  ${EXTRA_FEAT_LIST}
  SOURCE ../Basic/Attr.td
  TARGET ClangAttrParserStringSwitches)

clang_tablegen(AttrSubMatchRulesParserStringSwitches.inc
  -gen-clang-attr-subject-match-rules-parser-string-switches
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../
  ${EXTRA_FEAT_LIST}
  SOURCE ../Basic/Attr.td
  TARGET ClangAttrSubMatchRulesParserStringSwitches)
