set(EXTRA_FEAT_LIST)

if (OHOS_LLVM)
  list(APPEND EXTRA_FEAT_LIST "-DOHOS_LLVM")
endif()

clang_tablegen(Attrs.inc -gen-clang-attr-classes
  ${EXTRA_FEAT_LIST}
  SOURCE ../Basic/Attr.td
  TARGET ClangAttrClasses)

clang_tablegen(AttrImpl.inc -gen-clang-attr-impl
  ${EXTRA_FEAT_LIST}
  SOURCE ../Basic/Attr.td
  TARGET ClangAttrImpl)

clang_tablegen(AttrTextNodeDump.inc -gen-clang-attr-text-node-dump
  ${EXTRA_FEAT_LIST}
  SOURCE ../Basic/Attr.td
  TARGET ClangAttrTextDump)

clang_tablegen(AttrNodeTraverse.inc -gen-clang-attr-node-traverse
  ${EXTRA_FEAT_LIST}
  SOURCE ../Basic/Attr.td
  TARGET ClangAttrTraverse)

clang_tablegen(AttrVisitor.inc -gen-clang-attr-ast-visitor
  ${EXTRA_FEAT_LIST}
  SOURCE ../Basic/Attr.td
  TARGET ClangAttrVisitor)

clang_tablegen(StmtNodes.inc -gen-clang-stmt-nodes
  ${EXTRA_FEAT_LIST}
  SOURCE ../Basic/StmtNodes.td
  TARGET ClangStmtNodes)

clang_tablegen(DeclNodes.inc -gen-clang-decl-nodes
  ${EXTRA_FEAT_LIST}
  SOURCE ../Basic/DeclNodes.td
  TARGET ClangDeclNodes)

clang_tablegen(TypeNodes.inc -gen-clang-type-nodes
  ${EXTRA_FEAT_LIST}
  SOURCE ../Basic/TypeNodes.td
  TARGET ClangTypeNodes)

clang_tablegen(AbstractBasicReader.inc -gen-clang-basic-reader
  ${EXTRA_FEAT_LIST}
  SOURCE PropertiesBase.td
  TARGET ClangAbstractBasicReader)

clang_tablegen(AbstractBasicWriter.inc -gen-clang-basic-writer
  ${EXTRA_FEAT_LIST}
  SOURCE PropertiesBase.td
  TARGET ClangAbstractBasicWriter)

clang_tablegen(AbstractTypeReader.inc -gen-clang-type-reader
  ${EXTRA_FEAT_LIST}
  SOURCE TypeProperties.td
  TARGET ClangAbstractTypeReader)

clang_tablegen(AbstractTypeWriter.inc -gen-clang-type-writer
  ${EXTRA_FEAT_LIST}
  SOURCE TypeProperties.td
  TARGET ClangAbstractTypeWriter)

clang_tablegen(CommentNodes.inc -gen-clang-comment-nodes
  ${EXTRA_FEAT_LIST}
  SOURCE ../Basic/CommentNodes.td
  TARGET ClangCommentNodes)

clang_tablegen(CommentHTMLTags.inc -gen-clang-comment-html-tags
  ${EXTRA_FEAT_LIST}
  SOURCE CommentHTMLTags.td
  TARGET ClangCommentHTMLTags)

clang_tablegen(CommentHTMLTagsProperties.inc -gen-clang-comment-html-tags-properties
  ${EXTRA_FEAT_LIST}
  SOURCE CommentHTMLTags.td
  TARGET ClangCommentHTMLTagsProperties)

clang_tablegen(CommentHTMLNamedCharacterReferences.inc -gen-clang-comment-html-named-character-references
  ${EXTRA_FEAT_LIST}
  SOURCE CommentHTMLNamedCharacterReferences.td
  TARGET ClangCommentHTMLNamedCharacterReferences)

clang_tablegen(CommentCommandInfo.inc -gen-clang-comment-command-info
  ${EXTRA_FEAT_LIST}
  SOURCE CommentCommands.td
  TARGET ClangCommentCommandInfo)

clang_tablegen(CommentCommandList.inc -gen-clang-comment-command-list
  ${EXTRA_FEAT_LIST}
  SOURCE CommentCommands.td
  TARGET ClangCommentCommandList)

clang_tablegen(StmtDataCollectors.inc -gen-clang-data-collectors
  ${EXTRA_FEAT_LIST}
  SOURCE StmtDataCollectors.td
  TARGET StmtDataCollectors)
