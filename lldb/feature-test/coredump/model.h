#ifndef LLDB_FEATURE_TEST_COREDUMP_NTFILE_FALLBACK_SHARED_H
#define LLDB_FEATURE_TEST_COREDUMP_NTFILE_FALLBACK_SHARED_H

#include <cstdint>
#include <string>
#include <vector>

namespace ntfile_fixture {

enum class Category {
  Valid,
  EmptyPath,
  MalformedRange,
  DuplicatePath,
  PlaceholderOnly,
  StrictPath,
};

struct MappingRow {
  const char *CaseID;
  Category Kind;
  uint64_t Start;
  uint64_t End;
  uint64_t PageOffset;
  const char *Path;
  const char *Expected;
};

const char *categoryToString(Category c);
bool rowLooksMalformed(const MappingRow &r);
bool rowHasEmptyPath(const MappingRow &r);
bool categoryMatchesExpectation(const MappingRow &r);
std::vector<std::string> buildModuleView(const std::vector<MappingRow> &rows);
void printSummary(const std::vector<MappingRow> &rows);
bool validateRows(const std::vector<MappingRow> &rows);

void appendPart1(std::vector<MappingRow> &rows);
void appendPart2(std::vector<MappingRow> &rows);
void appendPart3(std::vector<MappingRow> &rows);
void appendPart4(std::vector<MappingRow> &rows);
std::vector<MappingRow> buildRows();

} // namespace ntfile_fixture

#endif
