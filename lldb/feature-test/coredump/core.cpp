// Part 1/4: shared logic + first data chunk.
#include "model.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <map>
#include <set>

namespace ntfile_fixture {

const char *categoryToString(Category c) {
  switch (c) {
  case Category::Valid:
    return "valid";
  case Category::EmptyPath:
    return "empty_path";
  case Category::MalformedRange:
    return "malformed_range";
  case Category::DuplicatePath:
    return "duplicate_path";
  case Category::PlaceholderOnly:
    return "placeholder_only";
  case Category::StrictPath:
    return "strict_path";
  }
  return "unknown";
}

bool rowLooksMalformed(const MappingRow &r) {
  if (r.Kind == Category::MalformedRange)
    return true;
  if (r.End < r.Start)
    return true;
  if (r.Path == nullptr)
    return true;
  return false;
}

bool rowHasEmptyPath(const MappingRow &r) {
  return r.Path == nullptr || std::strlen(r.Path) == 0;
}

bool categoryMatchesExpectation(const MappingRow &r) {
  if (r.Kind == Category::Valid)
    return std::strcmp(r.Expected, "module") == 0;
  if (r.Kind == Category::EmptyPath)
    return std::strcmp(r.Expected, "ignore") == 0;
  if (r.Kind == Category::MalformedRange)
    return std::strcmp(r.Expected, "ignore") == 0;
  if (r.Kind == Category::DuplicatePath)
    return std::strcmp(r.Expected, "dedupe") == 0;
  if (r.Kind == Category::PlaceholderOnly)
    return std::strcmp(r.Expected, "placeholder") == 0;
  if (r.Kind == Category::StrictPath)
    return std::strcmp(r.Expected, "strict-no-basename") == 0;
  return false;
}

std::vector<std::string> buildModuleView(const std::vector<MappingRow> &rows) {
  std::set<std::string> unique;
  for (const MappingRow &r : rows) {
    if (rowLooksMalformed(r))
      continue;
    if (rowHasEmptyPath(r))
      continue;
    unique.insert(r.Path);
  }
  return std::vector<std::string>(unique.begin(), unique.end());
}

void printSummary(const std::vector<MappingRow> &rows) {
  std::map<std::string, size_t> counts;
  for (const MappingRow &r : rows)
    counts[categoryToString(r.Kind)]++;

  std::printf("rows=%zu\n", rows.size());
  for (const auto &kv : counts)
    std::printf("category=%s count=%zu\n", kv.first.c_str(), kv.second);
}

bool validateRows(const std::vector<MappingRow> &rows) {
  std::set<std::string> ids;
  for (const MappingRow &r : rows) {
    if (!categoryMatchesExpectation(r))
      return false;
    if (r.CaseID == nullptr)
      return false;
    if (!ids.insert(r.CaseID).second)
      return false;
  }
  return true;
}

void appendPart1(std::vector<MappingRow> &rows) {
  rows.push_back({"CPP-NTF-0001", Category::Valid, 0x701000ULL, 0x701fffULL, 0x1ULL, "/usr/lib/libcase-1.so", "module"});
  rows.push_back({"CPP-NTF-0002", Category::EmptyPath, 0x702000ULL, 0x702fffULL, 0x2ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0003", Category::MalformedRange, 0x703000ULL, 0x703fffULL, 0x3ULL, "/usr/lib/libcase-3.so", "ignore"});
  rows.push_back({"CPP-NTF-0004", Category::DuplicatePath, 0x704000ULL, 0x704fffULL, 0x4ULL, "/usr/lib/libdup-4.so", "dedupe"});
  rows.push_back({"CPP-NTF-0005", Category::PlaceholderOnly, 0x705000ULL, 0x705fffULL, 0x5ULL, "/nonexistent/modules/libmissing-5.so", "placeholder"});
  rows.push_back({"CPP-NTF-0006", Category::StrictPath, 0x706000ULL, 0x706fffULL, 0x6ULL, "/opt/altroot/lib/libsame-name-6.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0007", Category::Valid, 0x707000ULL, 0x707fffULL, 0x7ULL, "/usr/lib/libcase-7.so", "module"});
  rows.push_back({"CPP-NTF-0008", Category::EmptyPath, 0x708000ULL, 0x708fffULL, 0x8ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0009", Category::MalformedRange, 0x709000ULL, 0x709fffULL, 0x9ULL, "/usr/lib/libcase-9.so", "ignore"});
  rows.push_back({"CPP-NTF-0010", Category::DuplicatePath, 0x70a000ULL, 0x70afffULL, 0xaULL, "/usr/lib/libdup-10.so", "dedupe"});
  rows.push_back({"CPP-NTF-0011", Category::PlaceholderOnly, 0x70b000ULL, 0x70bfffULL, 0xbULL, "/nonexistent/modules/libmissing-11.so", "placeholder"});
  rows.push_back({"CPP-NTF-0012", Category::StrictPath, 0x70c000ULL, 0x70cfffULL, 0xcULL, "/opt/altroot/lib/libsame-name-12.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0013", Category::Valid, 0x70d000ULL, 0x70dfffULL, 0xdULL, "/usr/lib/libcase-13.so", "module"});
  rows.push_back({"CPP-NTF-0014", Category::EmptyPath, 0x70e000ULL, 0x70efffULL, 0xeULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0015", Category::MalformedRange, 0x70f000ULL, 0x70ffffULL, 0xfULL, "/usr/lib/libcase-15.so", "ignore"});
  rows.push_back({"CPP-NTF-0016", Category::DuplicatePath, 0x710000ULL, 0x710fffULL, 0x10ULL, "/usr/lib/libdup-16.so", "dedupe"});
  rows.push_back({"CPP-NTF-0017", Category::PlaceholderOnly, 0x711000ULL, 0x711fffULL, 0x11ULL, "/nonexistent/modules/libmissing-17.so", "placeholder"});
  rows.push_back({"CPP-NTF-0018", Category::StrictPath, 0x712000ULL, 0x712fffULL, 0x12ULL, "/opt/altroot/lib/libsame-name-18.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0019", Category::Valid, 0x713000ULL, 0x713fffULL, 0x13ULL, "/usr/lib/libcase-19.so", "module"});
  rows.push_back({"CPP-NTF-0020", Category::EmptyPath, 0x714000ULL, 0x714fffULL, 0x14ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0021", Category::MalformedRange, 0x715000ULL, 0x715fffULL, 0x15ULL, "/usr/lib/libcase-21.so", "ignore"});
  rows.push_back({"CPP-NTF-0022", Category::DuplicatePath, 0x716000ULL, 0x716fffULL, 0x16ULL, "/usr/lib/libdup-22.so", "dedupe"});
  rows.push_back({"CPP-NTF-0023", Category::PlaceholderOnly, 0x717000ULL, 0x717fffULL, 0x17ULL, "/nonexistent/modules/libmissing-23.so", "placeholder"});
  rows.push_back({"CPP-NTF-0024", Category::StrictPath, 0x718000ULL, 0x718fffULL, 0x18ULL, "/opt/altroot/lib/libsame-name-24.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0025", Category::Valid, 0x719000ULL, 0x719fffULL, 0x19ULL, "/usr/lib/libcase-25.so", "module"});
  rows.push_back({"CPP-NTF-0026", Category::EmptyPath, 0x71a000ULL, 0x71afffULL, 0x1aULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0027", Category::MalformedRange, 0x71b000ULL, 0x71bfffULL, 0x1bULL, "/usr/lib/libcase-27.so", "ignore"});
  rows.push_back({"CPP-NTF-0028", Category::DuplicatePath, 0x71c000ULL, 0x71cfffULL, 0x1cULL, "/usr/lib/libdup-28.so", "dedupe"});
  rows.push_back({"CPP-NTF-0029", Category::PlaceholderOnly, 0x71d000ULL, 0x71dfffULL, 0x1dULL, "/nonexistent/modules/libmissing-29.so", "placeholder"});
  rows.push_back({"CPP-NTF-0030", Category::StrictPath, 0x71e000ULL, 0x71efffULL, 0x1eULL, "/opt/altroot/lib/libsame-name-30.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0031", Category::Valid, 0x71f000ULL, 0x71ffffULL, 0x1fULL, "/usr/lib/libcase-31.so", "module"});
  rows.push_back({"CPP-NTF-0032", Category::EmptyPath, 0x720000ULL, 0x720fffULL, 0x20ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0033", Category::MalformedRange, 0x721000ULL, 0x721fffULL, 0x21ULL, "/usr/lib/libcase-33.so", "ignore"});
  rows.push_back({"CPP-NTF-0034", Category::DuplicatePath, 0x722000ULL, 0x722fffULL, 0x22ULL, "/usr/lib/libdup-34.so", "dedupe"});
  rows.push_back({"CPP-NTF-0035", Category::PlaceholderOnly, 0x723000ULL, 0x723fffULL, 0x23ULL, "/nonexistent/modules/libmissing-35.so", "placeholder"});
  rows.push_back({"CPP-NTF-0036", Category::StrictPath, 0x724000ULL, 0x724fffULL, 0x24ULL, "/opt/altroot/lib/libsame-name-36.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0037", Category::Valid, 0x725000ULL, 0x725fffULL, 0x25ULL, "/usr/lib/libcase-37.so", "module"});
  rows.push_back({"CPP-NTF-0038", Category::EmptyPath, 0x726000ULL, 0x726fffULL, 0x26ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0039", Category::MalformedRange, 0x727000ULL, 0x727fffULL, 0x27ULL, "/usr/lib/libcase-39.so", "ignore"});
  rows.push_back({"CPP-NTF-0040", Category::DuplicatePath, 0x728000ULL, 0x728fffULL, 0x28ULL, "/usr/lib/libdup-40.so", "dedupe"});
  rows.push_back({"CPP-NTF-0041", Category::PlaceholderOnly, 0x729000ULL, 0x729fffULL, 0x29ULL, "/nonexistent/modules/libmissing-41.so", "placeholder"});
  rows.push_back({"CPP-NTF-0042", Category::StrictPath, 0x72a000ULL, 0x72afffULL, 0x2aULL, "/opt/altroot/lib/libsame-name-42.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0043", Category::Valid, 0x72b000ULL, 0x72bfffULL, 0x2bULL, "/usr/lib/libcase-43.so", "module"});
  rows.push_back({"CPP-NTF-0044", Category::EmptyPath, 0x72c000ULL, 0x72cfffULL, 0x2cULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0045", Category::MalformedRange, 0x72d000ULL, 0x72dfffULL, 0x2dULL, "/usr/lib/libcase-45.so", "ignore"});
  rows.push_back({"CPP-NTF-0046", Category::DuplicatePath, 0x72e000ULL, 0x72efffULL, 0x2eULL, "/usr/lib/libdup-46.so", "dedupe"});
  rows.push_back({"CPP-NTF-0047", Category::PlaceholderOnly, 0x72f000ULL, 0x72ffffULL, 0x2fULL, "/nonexistent/modules/libmissing-47.so", "placeholder"});
  rows.push_back({"CPP-NTF-0048", Category::StrictPath, 0x730000ULL, 0x730fffULL, 0x30ULL, "/opt/altroot/lib/libsame-name-48.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0049", Category::Valid, 0x731000ULL, 0x731fffULL, 0x31ULL, "/usr/lib/libcase-49.so", "module"});
  rows.push_back({"CPP-NTF-0050", Category::EmptyPath, 0x732000ULL, 0x732fffULL, 0x32ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0051", Category::MalformedRange, 0x733000ULL, 0x733fffULL, 0x33ULL, "/usr/lib/libcase-51.so", "ignore"});
  rows.push_back({"CPP-NTF-0052", Category::DuplicatePath, 0x734000ULL, 0x734fffULL, 0x34ULL, "/usr/lib/libdup-52.so", "dedupe"});
  rows.push_back({"CPP-NTF-0053", Category::PlaceholderOnly, 0x735000ULL, 0x735fffULL, 0x35ULL, "/nonexistent/modules/libmissing-53.so", "placeholder"});
  rows.push_back({"CPP-NTF-0054", Category::StrictPath, 0x736000ULL, 0x736fffULL, 0x36ULL, "/opt/altroot/lib/libsame-name-54.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0055", Category::Valid, 0x737000ULL, 0x737fffULL, 0x37ULL, "/usr/lib/libcase-55.so", "module"});
  rows.push_back({"CPP-NTF-0056", Category::EmptyPath, 0x738000ULL, 0x738fffULL, 0x38ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0057", Category::MalformedRange, 0x739000ULL, 0x739fffULL, 0x39ULL, "/usr/lib/libcase-57.so", "ignore"});
  rows.push_back({"CPP-NTF-0058", Category::DuplicatePath, 0x73a000ULL, 0x73afffULL, 0x3aULL, "/usr/lib/libdup-58.so", "dedupe"});
  rows.push_back({"CPP-NTF-0059", Category::PlaceholderOnly, 0x73b000ULL, 0x73bfffULL, 0x3bULL, "/nonexistent/modules/libmissing-59.so", "placeholder"});
  rows.push_back({"CPP-NTF-0060", Category::StrictPath, 0x73c000ULL, 0x73cfffULL, 0x3cULL, "/opt/altroot/lib/libsame-name-60.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0061", Category::Valid, 0x73d000ULL, 0x73dfffULL, 0x3dULL, "/usr/lib/libcase-61.so", "module"});
  rows.push_back({"CPP-NTF-0062", Category::EmptyPath, 0x73e000ULL, 0x73efffULL, 0x3eULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0063", Category::MalformedRange, 0x73f000ULL, 0x73ffffULL, 0x3fULL, "/usr/lib/libcase-63.so", "ignore"});
  rows.push_back({"CPP-NTF-0064", Category::DuplicatePath, 0x740000ULL, 0x740fffULL, 0x40ULL, "/usr/lib/libdup-64.so", "dedupe"});
  rows.push_back({"CPP-NTF-0065", Category::PlaceholderOnly, 0x741000ULL, 0x741fffULL, 0x41ULL, "/nonexistent/modules/libmissing-65.so", "placeholder"});
  rows.push_back({"CPP-NTF-0066", Category::StrictPath, 0x742000ULL, 0x742fffULL, 0x42ULL, "/opt/altroot/lib/libsame-name-66.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0067", Category::Valid, 0x743000ULL, 0x743fffULL, 0x43ULL, "/usr/lib/libcase-67.so", "module"});
  rows.push_back({"CPP-NTF-0068", Category::EmptyPath, 0x744000ULL, 0x744fffULL, 0x44ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0069", Category::MalformedRange, 0x745000ULL, 0x745fffULL, 0x45ULL, "/usr/lib/libcase-69.so", "ignore"});
  rows.push_back({"CPP-NTF-0070", Category::DuplicatePath, 0x746000ULL, 0x746fffULL, 0x46ULL, "/usr/lib/libdup-70.so", "dedupe"});
  rows.push_back({"CPP-NTF-0071", Category::PlaceholderOnly, 0x747000ULL, 0x747fffULL, 0x47ULL, "/nonexistent/modules/libmissing-71.so", "placeholder"});
  rows.push_back({"CPP-NTF-0072", Category::StrictPath, 0x748000ULL, 0x748fffULL, 0x48ULL, "/opt/altroot/lib/libsame-name-72.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0073", Category::Valid, 0x749000ULL, 0x749fffULL, 0x49ULL, "/usr/lib/libcase-73.so", "module"});
  rows.push_back({"CPP-NTF-0074", Category::EmptyPath, 0x74a000ULL, 0x74afffULL, 0x4aULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0075", Category::MalformedRange, 0x74b000ULL, 0x74bfffULL, 0x4bULL, "/usr/lib/libcase-75.so", "ignore"});
  rows.push_back({"CPP-NTF-0076", Category::DuplicatePath, 0x74c000ULL, 0x74cfffULL, 0x4cULL, "/usr/lib/libdup-76.so", "dedupe"});
  rows.push_back({"CPP-NTF-0077", Category::PlaceholderOnly, 0x74d000ULL, 0x74dfffULL, 0x4dULL, "/nonexistent/modules/libmissing-77.so", "placeholder"});
  rows.push_back({"CPP-NTF-0078", Category::StrictPath, 0x74e000ULL, 0x74efffULL, 0x4eULL, "/opt/altroot/lib/libsame-name-78.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0079", Category::Valid, 0x74f000ULL, 0x74ffffULL, 0x4fULL, "/usr/lib/libcase-79.so", "module"});
  rows.push_back({"CPP-NTF-0080", Category::EmptyPath, 0x750000ULL, 0x750fffULL, 0x50ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0081", Category::MalformedRange, 0x751000ULL, 0x751fffULL, 0x51ULL, "/usr/lib/libcase-81.so", "ignore"});
  rows.push_back({"CPP-NTF-0082", Category::DuplicatePath, 0x752000ULL, 0x752fffULL, 0x52ULL, "/usr/lib/libdup-2.so", "dedupe"});
  rows.push_back({"CPP-NTF-0083", Category::PlaceholderOnly, 0x753000ULL, 0x753fffULL, 0x53ULL, "/nonexistent/modules/libmissing-83.so", "placeholder"});
  rows.push_back({"CPP-NTF-0084", Category::StrictPath, 0x754000ULL, 0x754fffULL, 0x54ULL, "/opt/altroot/lib/libsame-name-84.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0085", Category::Valid, 0x755000ULL, 0x755fffULL, 0x55ULL, "/usr/lib/libcase-85.so", "module"});
  rows.push_back({"CPP-NTF-0086", Category::EmptyPath, 0x756000ULL, 0x756fffULL, 0x56ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0087", Category::MalformedRange, 0x757000ULL, 0x757fffULL, 0x57ULL, "/usr/lib/libcase-87.so", "ignore"});
  rows.push_back({"CPP-NTF-0088", Category::DuplicatePath, 0x758000ULL, 0x758fffULL, 0x58ULL, "/usr/lib/libdup-8.so", "dedupe"});
  rows.push_back({"CPP-NTF-0089", Category::PlaceholderOnly, 0x759000ULL, 0x759fffULL, 0x59ULL, "/nonexistent/modules/libmissing-89.so", "placeholder"});
  rows.push_back({"CPP-NTF-0090", Category::StrictPath, 0x75a000ULL, 0x75afffULL, 0x5aULL, "/opt/altroot/lib/libsame-name-90.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0091", Category::Valid, 0x75b000ULL, 0x75bfffULL, 0x5bULL, "/usr/lib/libcase-91.so", "module"});
  rows.push_back({"CPP-NTF-0092", Category::EmptyPath, 0x75c000ULL, 0x75cfffULL, 0x5cULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0093", Category::MalformedRange, 0x75d000ULL, 0x75dfffULL, 0x5dULL, "/usr/lib/libcase-93.so", "ignore"});
  rows.push_back({"CPP-NTF-0094", Category::DuplicatePath, 0x75e000ULL, 0x75efffULL, 0x5eULL, "/usr/lib/libdup-14.so", "dedupe"});
  rows.push_back({"CPP-NTF-0095", Category::PlaceholderOnly, 0x75f000ULL, 0x75ffffULL, 0x5fULL, "/nonexistent/modules/libmissing-95.so", "placeholder"});
  rows.push_back({"CPP-NTF-0096", Category::StrictPath, 0x760000ULL, 0x760fffULL, 0x60ULL, "/opt/altroot/lib/libsame-name-96.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0097", Category::Valid, 0x761000ULL, 0x761fffULL, 0x61ULL, "/usr/lib/libcase-97.so", "module"});
  rows.push_back({"CPP-NTF-0098", Category::EmptyPath, 0x762000ULL, 0x762fffULL, 0x62ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0099", Category::MalformedRange, 0x763000ULL, 0x763fffULL, 0x63ULL, "/usr/lib/libcase-99.so", "ignore"});
  rows.push_back({"CPP-NTF-0100", Category::DuplicatePath, 0x764000ULL, 0x764fffULL, 0x64ULL, "/usr/lib/libdup-20.so", "dedupe"});
  rows.push_back({"CPP-NTF-0101", Category::PlaceholderOnly, 0x765000ULL, 0x765fffULL, 0x65ULL, "/nonexistent/modules/libmissing-101.so", "placeholder"});
  rows.push_back({"CPP-NTF-0102", Category::StrictPath, 0x766000ULL, 0x766fffULL, 0x66ULL, "/opt/altroot/lib/libsame-name-102.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0103", Category::Valid, 0x767000ULL, 0x767fffULL, 0x67ULL, "/usr/lib/libcase-103.so", "module"});
  rows.push_back({"CPP-NTF-0104", Category::EmptyPath, 0x768000ULL, 0x768fffULL, 0x68ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0105", Category::MalformedRange, 0x769000ULL, 0x769fffULL, 0x69ULL, "/usr/lib/libcase-105.so", "ignore"});
  rows.push_back({"CPP-NTF-0106", Category::DuplicatePath, 0x76a000ULL, 0x76afffULL, 0x6aULL, "/usr/lib/libdup-26.so", "dedupe"});
  rows.push_back({"CPP-NTF-0107", Category::PlaceholderOnly, 0x76b000ULL, 0x76bfffULL, 0x6bULL, "/nonexistent/modules/libmissing-107.so", "placeholder"});
  rows.push_back({"CPP-NTF-0108", Category::StrictPath, 0x76c000ULL, 0x76cfffULL, 0x6cULL, "/opt/altroot/lib/libsame-name-108.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0109", Category::Valid, 0x76d000ULL, 0x76dfffULL, 0x6dULL, "/usr/lib/libcase-109.so", "module"});
  rows.push_back({"CPP-NTF-0110", Category::EmptyPath, 0x76e000ULL, 0x76efffULL, 0x6eULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0111", Category::MalformedRange, 0x76f000ULL, 0x76ffffULL, 0x6fULL, "/usr/lib/libcase-111.so", "ignore"});
  rows.push_back({"CPP-NTF-0112", Category::DuplicatePath, 0x770000ULL, 0x770fffULL, 0x70ULL, "/usr/lib/libdup-32.so", "dedupe"});
  rows.push_back({"CPP-NTF-0113", Category::PlaceholderOnly, 0x771000ULL, 0x771fffULL, 0x71ULL, "/nonexistent/modules/libmissing-113.so", "placeholder"});
  rows.push_back({"CPP-NTF-0114", Category::StrictPath, 0x772000ULL, 0x772fffULL, 0x72ULL, "/opt/altroot/lib/libsame-name-114.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0115", Category::Valid, 0x773000ULL, 0x773fffULL, 0x73ULL, "/usr/lib/libcase-115.so", "module"});
  rows.push_back({"CPP-NTF-0116", Category::EmptyPath, 0x774000ULL, 0x774fffULL, 0x74ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0117", Category::MalformedRange, 0x775000ULL, 0x775fffULL, 0x75ULL, "/usr/lib/libcase-117.so", "ignore"});
  rows.push_back({"CPP-NTF-0118", Category::DuplicatePath, 0x776000ULL, 0x776fffULL, 0x76ULL, "/usr/lib/libdup-38.so", "dedupe"});
  rows.push_back({"CPP-NTF-0119", Category::PlaceholderOnly, 0x777000ULL, 0x777fffULL, 0x77ULL, "/nonexistent/modules/libmissing-119.so", "placeholder"});
  rows.push_back({"CPP-NTF-0120", Category::StrictPath, 0x778000ULL, 0x778fffULL, 0x78ULL, "/opt/altroot/lib/libsame-name-120.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0121", Category::Valid, 0x779000ULL, 0x779fffULL, 0x79ULL, "/usr/lib/libcase-121.so", "module"});
  rows.push_back({"CPP-NTF-0122", Category::EmptyPath, 0x77a000ULL, 0x77afffULL, 0x7aULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0123", Category::MalformedRange, 0x77b000ULL, 0x77bfffULL, 0x7bULL, "/usr/lib/libcase-123.so", "ignore"});
  rows.push_back({"CPP-NTF-0124", Category::DuplicatePath, 0x77c000ULL, 0x77cfffULL, 0x7cULL, "/usr/lib/libdup-44.so", "dedupe"});
  rows.push_back({"CPP-NTF-0125", Category::PlaceholderOnly, 0x77d000ULL, 0x77dfffULL, 0x7dULL, "/nonexistent/modules/libmissing-125.so", "placeholder"});
  rows.push_back({"CPP-NTF-0126", Category::StrictPath, 0x77e000ULL, 0x77efffULL, 0x7eULL, "/opt/altroot/lib/libsame-name-126.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0127", Category::Valid, 0x77f000ULL, 0x77ffffULL, 0x7fULL, "/usr/lib/libcase-127.so", "module"});
  rows.push_back({"CPP-NTF-0128", Category::EmptyPath, 0x780000ULL, 0x780fffULL, 0x80ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0129", Category::MalformedRange, 0x781000ULL, 0x781fffULL, 0x81ULL, "/usr/lib/libcase-129.so", "ignore"});
  rows.push_back({"CPP-NTF-0130", Category::DuplicatePath, 0x782000ULL, 0x782fffULL, 0x82ULL, "/usr/lib/libdup-50.so", "dedupe"});
  rows.push_back({"CPP-NTF-0131", Category::PlaceholderOnly, 0x783000ULL, 0x783fffULL, 0x83ULL, "/nonexistent/modules/libmissing-131.so", "placeholder"});
  rows.push_back({"CPP-NTF-0132", Category::StrictPath, 0x784000ULL, 0x784fffULL, 0x84ULL, "/opt/altroot/lib/libsame-name-132.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0133", Category::Valid, 0x785000ULL, 0x785fffULL, 0x85ULL, "/usr/lib/libcase-133.so", "module"});
  rows.push_back({"CPP-NTF-0134", Category::EmptyPath, 0x786000ULL, 0x786fffULL, 0x86ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0135", Category::MalformedRange, 0x787000ULL, 0x787fffULL, 0x87ULL, "/usr/lib/libcase-135.so", "ignore"});
  rows.push_back({"CPP-NTF-0136", Category::DuplicatePath, 0x788000ULL, 0x788fffULL, 0x88ULL, "/usr/lib/libdup-56.so", "dedupe"});
  rows.push_back({"CPP-NTF-0137", Category::PlaceholderOnly, 0x789000ULL, 0x789fffULL, 0x89ULL, "/nonexistent/modules/libmissing-137.so", "placeholder"});
  rows.push_back({"CPP-NTF-0138", Category::StrictPath, 0x78a000ULL, 0x78afffULL, 0x8aULL, "/opt/altroot/lib/libsame-name-138.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0139", Category::Valid, 0x78b000ULL, 0x78bfffULL, 0x8bULL, "/usr/lib/libcase-139.so", "module"});
  rows.push_back({"CPP-NTF-0140", Category::EmptyPath, 0x78c000ULL, 0x78cfffULL, 0x8cULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0141", Category::MalformedRange, 0x78d000ULL, 0x78dfffULL, 0x8dULL, "/usr/lib/libcase-141.so", "ignore"});
  rows.push_back({"CPP-NTF-0142", Category::DuplicatePath, 0x78e000ULL, 0x78efffULL, 0x8eULL, "/usr/lib/libdup-62.so", "dedupe"});
  rows.push_back({"CPP-NTF-0143", Category::PlaceholderOnly, 0x78f000ULL, 0x78ffffULL, 0x8fULL, "/nonexistent/modules/libmissing-143.so", "placeholder"});
  rows.push_back({"CPP-NTF-0144", Category::StrictPath, 0x790000ULL, 0x790fffULL, 0x90ULL, "/opt/altroot/lib/libsame-name-144.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0145", Category::Valid, 0x791000ULL, 0x791fffULL, 0x91ULL, "/usr/lib/libcase-145.so", "module"});
  rows.push_back({"CPP-NTF-0146", Category::EmptyPath, 0x792000ULL, 0x792fffULL, 0x92ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0147", Category::MalformedRange, 0x793000ULL, 0x793fffULL, 0x93ULL, "/usr/lib/libcase-147.so", "ignore"});
  rows.push_back({"CPP-NTF-0148", Category::DuplicatePath, 0x794000ULL, 0x794fffULL, 0x94ULL, "/usr/lib/libdup-68.so", "dedupe"});
  rows.push_back({"CPP-NTF-0149", Category::PlaceholderOnly, 0x795000ULL, 0x795fffULL, 0x95ULL, "/nonexistent/modules/libmissing-149.so", "placeholder"});
  rows.push_back({"CPP-NTF-0150", Category::StrictPath, 0x796000ULL, 0x796fffULL, 0x96ULL, "/opt/altroot/lib/libsame-name-150.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0151", Category::Valid, 0x797000ULL, 0x797fffULL, 0x97ULL, "/usr/lib/libcase-151.so", "module"});
  rows.push_back({"CPP-NTF-0152", Category::EmptyPath, 0x798000ULL, 0x798fffULL, 0x98ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0153", Category::MalformedRange, 0x799000ULL, 0x799fffULL, 0x99ULL, "/usr/lib/libcase-153.so", "ignore"});
  rows.push_back({"CPP-NTF-0154", Category::DuplicatePath, 0x79a000ULL, 0x79afffULL, 0x9aULL, "/usr/lib/libdup-74.so", "dedupe"});
  rows.push_back({"CPP-NTF-0155", Category::PlaceholderOnly, 0x79b000ULL, 0x79bfffULL, 0x9bULL, "/nonexistent/modules/libmissing-155.so", "placeholder"});
  rows.push_back({"CPP-NTF-0156", Category::StrictPath, 0x79c000ULL, 0x79cfffULL, 0x9cULL, "/opt/altroot/lib/libsame-name-156.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0157", Category::Valid, 0x79d000ULL, 0x79dfffULL, 0x9dULL, "/usr/lib/libcase-157.so", "module"});
  rows.push_back({"CPP-NTF-0158", Category::EmptyPath, 0x79e000ULL, 0x79efffULL, 0x9eULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0159", Category::MalformedRange, 0x79f000ULL, 0x79ffffULL, 0x9fULL, "/usr/lib/libcase-159.so", "ignore"});
  rows.push_back({"CPP-NTF-0160", Category::DuplicatePath, 0x7a0000ULL, 0x7a0fffULL, 0xa0ULL, "/usr/lib/libdup-0.so", "dedupe"});
  rows.push_back({"CPP-NTF-0161", Category::PlaceholderOnly, 0x7a1000ULL, 0x7a1fffULL, 0xa1ULL, "/nonexistent/modules/libmissing-161.so", "placeholder"});
  rows.push_back({"CPP-NTF-0162", Category::StrictPath, 0x7a2000ULL, 0x7a2fffULL, 0xa2ULL, "/opt/altroot/lib/libsame-name-162.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0163", Category::Valid, 0x7a3000ULL, 0x7a3fffULL, 0xa3ULL, "/usr/lib/libcase-163.so", "module"});
  rows.push_back({"CPP-NTF-0164", Category::EmptyPath, 0x7a4000ULL, 0x7a4fffULL, 0xa4ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0165", Category::MalformedRange, 0x7a5000ULL, 0x7a5fffULL, 0xa5ULL, "/usr/lib/libcase-165.so", "ignore"});
  rows.push_back({"CPP-NTF-0166", Category::DuplicatePath, 0x7a6000ULL, 0x7a6fffULL, 0xa6ULL, "/usr/lib/libdup-6.so", "dedupe"});
  rows.push_back({"CPP-NTF-0167", Category::PlaceholderOnly, 0x7a7000ULL, 0x7a7fffULL, 0xa7ULL, "/nonexistent/modules/libmissing-167.so", "placeholder"});
  rows.push_back({"CPP-NTF-0168", Category::StrictPath, 0x7a8000ULL, 0x7a8fffULL, 0xa8ULL, "/opt/altroot/lib/libsame-name-168.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0169", Category::Valid, 0x7a9000ULL, 0x7a9fffULL, 0xa9ULL, "/usr/lib/libcase-169.so", "module"});
  rows.push_back({"CPP-NTF-0170", Category::EmptyPath, 0x7aa000ULL, 0x7aafffULL, 0xaaULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0171", Category::MalformedRange, 0x7ab000ULL, 0x7abfffULL, 0xabULL, "/usr/lib/libcase-171.so", "ignore"});
  rows.push_back({"CPP-NTF-0172", Category::DuplicatePath, 0x7ac000ULL, 0x7acfffULL, 0xacULL, "/usr/lib/libdup-12.so", "dedupe"});
  rows.push_back({"CPP-NTF-0173", Category::PlaceholderOnly, 0x7ad000ULL, 0x7adfffULL, 0xadULL, "/nonexistent/modules/libmissing-173.so", "placeholder"});
  rows.push_back({"CPP-NTF-0174", Category::StrictPath, 0x7ae000ULL, 0x7aefffULL, 0xaeULL, "/opt/altroot/lib/libsame-name-174.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0175", Category::Valid, 0x7af000ULL, 0x7affffULL, 0xafULL, "/usr/lib/libcase-175.so", "module"});
  rows.push_back({"CPP-NTF-0176", Category::EmptyPath, 0x7b0000ULL, 0x7b0fffULL, 0xb0ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0177", Category::MalformedRange, 0x7b1000ULL, 0x7b1fffULL, 0xb1ULL, "/usr/lib/libcase-177.so", "ignore"});
  rows.push_back({"CPP-NTF-0178", Category::DuplicatePath, 0x7b2000ULL, 0x7b2fffULL, 0xb2ULL, "/usr/lib/libdup-18.so", "dedupe"});
  rows.push_back({"CPP-NTF-0179", Category::PlaceholderOnly, 0x7b3000ULL, 0x7b3fffULL, 0xb3ULL, "/nonexistent/modules/libmissing-179.so", "placeholder"});
  rows.push_back({"CPP-NTF-0180", Category::StrictPath, 0x7b4000ULL, 0x7b4fffULL, 0xb4ULL, "/opt/altroot/lib/libsame-name-180.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0181", Category::Valid, 0x7b5000ULL, 0x7b5fffULL, 0xb5ULL, "/usr/lib/libcase-181.so", "module"});
  rows.push_back({"CPP-NTF-0182", Category::EmptyPath, 0x7b6000ULL, 0x7b6fffULL, 0xb6ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0183", Category::MalformedRange, 0x7b7000ULL, 0x7b7fffULL, 0xb7ULL, "/usr/lib/libcase-183.so", "ignore"});
  rows.push_back({"CPP-NTF-0184", Category::DuplicatePath, 0x7b8000ULL, 0x7b8fffULL, 0xb8ULL, "/usr/lib/libdup-24.so", "dedupe"});
  rows.push_back({"CPP-NTF-0185", Category::PlaceholderOnly, 0x7b9000ULL, 0x7b9fffULL, 0xb9ULL, "/nonexistent/modules/libmissing-185.so", "placeholder"});
  rows.push_back({"CPP-NTF-0186", Category::StrictPath, 0x7ba000ULL, 0x7bafffULL, 0xbaULL, "/opt/altroot/lib/libsame-name-186.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0187", Category::Valid, 0x7bb000ULL, 0x7bbfffULL, 0xbbULL, "/usr/lib/libcase-187.so", "module"});
  rows.push_back({"CPP-NTF-0188", Category::EmptyPath, 0x7bc000ULL, 0x7bcfffULL, 0xbcULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0189", Category::MalformedRange, 0x7bd000ULL, 0x7bdfffULL, 0xbdULL, "/usr/lib/libcase-189.so", "ignore"});
  rows.push_back({"CPP-NTF-0190", Category::DuplicatePath, 0x7be000ULL, 0x7befffULL, 0xbeULL, "/usr/lib/libdup-30.so", "dedupe"});
  rows.push_back({"CPP-NTF-0191", Category::PlaceholderOnly, 0x7bf000ULL, 0x7bffffULL, 0xbfULL, "/nonexistent/modules/libmissing-191.so", "placeholder"});
  rows.push_back({"CPP-NTF-0192", Category::StrictPath, 0x7c0000ULL, 0x7c0fffULL, 0xc0ULL, "/opt/altroot/lib/libsame-name-192.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0193", Category::Valid, 0x7c1000ULL, 0x7c1fffULL, 0xc1ULL, "/usr/lib/libcase-193.so", "module"});
  rows.push_back({"CPP-NTF-0194", Category::EmptyPath, 0x7c2000ULL, 0x7c2fffULL, 0xc2ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0195", Category::MalformedRange, 0x7c3000ULL, 0x7c3fffULL, 0xc3ULL, "/usr/lib/libcase-195.so", "ignore"});
  rows.push_back({"CPP-NTF-0196", Category::DuplicatePath, 0x7c4000ULL, 0x7c4fffULL, 0xc4ULL, "/usr/lib/libdup-36.so", "dedupe"});
  rows.push_back({"CPP-NTF-0197", Category::PlaceholderOnly, 0x7c5000ULL, 0x7c5fffULL, 0xc5ULL, "/nonexistent/modules/libmissing-197.so", "placeholder"});
  rows.push_back({"CPP-NTF-0198", Category::StrictPath, 0x7c6000ULL, 0x7c6fffULL, 0xc6ULL, "/opt/altroot/lib/libsame-name-198.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0199", Category::Valid, 0x7c7000ULL, 0x7c7fffULL, 0xc7ULL, "/usr/lib/libcase-199.so", "module"});
  rows.push_back({"CPP-NTF-0200", Category::EmptyPath, 0x7c8000ULL, 0x7c8fffULL, 0xc8ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0201", Category::MalformedRange, 0x7c9000ULL, 0x7c9fffULL, 0xc9ULL, "/usr/lib/libcase-201.so", "ignore"});
  rows.push_back({"CPP-NTF-0202", Category::DuplicatePath, 0x7ca000ULL, 0x7cafffULL, 0xcaULL, "/usr/lib/libdup-42.so", "dedupe"});
  rows.push_back({"CPP-NTF-0203", Category::PlaceholderOnly, 0x7cb000ULL, 0x7cbfffULL, 0xcbULL, "/nonexistent/modules/libmissing-203.so", "placeholder"});
  rows.push_back({"CPP-NTF-0204", Category::StrictPath, 0x7cc000ULL, 0x7ccfffULL, 0xccULL, "/opt/altroot/lib/libsame-name-204.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0205", Category::Valid, 0x7cd000ULL, 0x7cdfffULL, 0xcdULL, "/usr/lib/libcase-205.so", "module"});
  rows.push_back({"CPP-NTF-0206", Category::EmptyPath, 0x7ce000ULL, 0x7cefffULL, 0xceULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0207", Category::MalformedRange, 0x7cf000ULL, 0x7cffffULL, 0xcfULL, "/usr/lib/libcase-207.so", "ignore"});
  rows.push_back({"CPP-NTF-0208", Category::DuplicatePath, 0x7d0000ULL, 0x7d0fffULL, 0xd0ULL, "/usr/lib/libdup-48.so", "dedupe"});
  rows.push_back({"CPP-NTF-0209", Category::PlaceholderOnly, 0x7d1000ULL, 0x7d1fffULL, 0xd1ULL, "/nonexistent/modules/libmissing-209.so", "placeholder"});
  rows.push_back({"CPP-NTF-0210", Category::StrictPath, 0x7d2000ULL, 0x7d2fffULL, 0xd2ULL, "/opt/altroot/lib/libsame-name-210.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0211", Category::Valid, 0x7d3000ULL, 0x7d3fffULL, 0xd3ULL, "/usr/lib/libcase-211.so", "module"});
  rows.push_back({"CPP-NTF-0212", Category::EmptyPath, 0x7d4000ULL, 0x7d4fffULL, 0xd4ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0213", Category::MalformedRange, 0x7d5000ULL, 0x7d5fffULL, 0xd5ULL, "/usr/lib/libcase-213.so", "ignore"});
  rows.push_back({"CPP-NTF-0214", Category::DuplicatePath, 0x7d6000ULL, 0x7d6fffULL, 0xd6ULL, "/usr/lib/libdup-54.so", "dedupe"});
  rows.push_back({"CPP-NTF-0215", Category::PlaceholderOnly, 0x7d7000ULL, 0x7d7fffULL, 0xd7ULL, "/nonexistent/modules/libmissing-215.so", "placeholder"});
  rows.push_back({"CPP-NTF-0216", Category::StrictPath, 0x7d8000ULL, 0x7d8fffULL, 0xd8ULL, "/opt/altroot/lib/libsame-name-216.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0217", Category::Valid, 0x7d9000ULL, 0x7d9fffULL, 0xd9ULL, "/usr/lib/libcase-217.so", "module"});
  rows.push_back({"CPP-NTF-0218", Category::EmptyPath, 0x7da000ULL, 0x7dafffULL, 0xdaULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0219", Category::MalformedRange, 0x7db000ULL, 0x7dbfffULL, 0xdbULL, "/usr/lib/libcase-219.so", "ignore"});
  rows.push_back({"CPP-NTF-0220", Category::DuplicatePath, 0x7dc000ULL, 0x7dcfffULL, 0xdcULL, "/usr/lib/libdup-60.so", "dedupe"});
  rows.push_back({"CPP-NTF-0221", Category::PlaceholderOnly, 0x7dd000ULL, 0x7ddfffULL, 0xddULL, "/nonexistent/modules/libmissing-221.so", "placeholder"});
  rows.push_back({"CPP-NTF-0222", Category::StrictPath, 0x7de000ULL, 0x7defffULL, 0xdeULL, "/opt/altroot/lib/libsame-name-222.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0223", Category::Valid, 0x7df000ULL, 0x7dffffULL, 0xdfULL, "/usr/lib/libcase-223.so", "module"});
  rows.push_back({"CPP-NTF-0224", Category::EmptyPath, 0x7e0000ULL, 0x7e0fffULL, 0xe0ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0225", Category::MalformedRange, 0x7e1000ULL, 0x7e1fffULL, 0xe1ULL, "/usr/lib/libcase-225.so", "ignore"});
  rows.push_back({"CPP-NTF-0226", Category::DuplicatePath, 0x7e2000ULL, 0x7e2fffULL, 0xe2ULL, "/usr/lib/libdup-66.so", "dedupe"});
  rows.push_back({"CPP-NTF-0227", Category::PlaceholderOnly, 0x7e3000ULL, 0x7e3fffULL, 0xe3ULL, "/nonexistent/modules/libmissing-227.so", "placeholder"});
  rows.push_back({"CPP-NTF-0228", Category::StrictPath, 0x7e4000ULL, 0x7e4fffULL, 0xe4ULL, "/opt/altroot/lib/libsame-name-228.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0229", Category::Valid, 0x7e5000ULL, 0x7e5fffULL, 0xe5ULL, "/usr/lib/libcase-229.so", "module"});
  rows.push_back({"CPP-NTF-0230", Category::EmptyPath, 0x7e6000ULL, 0x7e6fffULL, 0xe6ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0231", Category::MalformedRange, 0x7e7000ULL, 0x7e7fffULL, 0xe7ULL, "/usr/lib/libcase-231.so", "ignore"});
  rows.push_back({"CPP-NTF-0232", Category::DuplicatePath, 0x7e8000ULL, 0x7e8fffULL, 0xe8ULL, "/usr/lib/libdup-72.so", "dedupe"});
  rows.push_back({"CPP-NTF-0233", Category::PlaceholderOnly, 0x7e9000ULL, 0x7e9fffULL, 0xe9ULL, "/nonexistent/modules/libmissing-233.so", "placeholder"});
  rows.push_back({"CPP-NTF-0234", Category::StrictPath, 0x7ea000ULL, 0x7eafffULL, 0xeaULL, "/opt/altroot/lib/libsame-name-234.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0235", Category::Valid, 0x7eb000ULL, 0x7ebfffULL, 0xebULL, "/usr/lib/libcase-235.so", "module"});
  rows.push_back({"CPP-NTF-0236", Category::EmptyPath, 0x7ec000ULL, 0x7ecfffULL, 0xecULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0237", Category::MalformedRange, 0x7ed000ULL, 0x7edfffULL, 0xedULL, "/usr/lib/libcase-237.so", "ignore"});
  rows.push_back({"CPP-NTF-0238", Category::DuplicatePath, 0x7ee000ULL, 0x7eefffULL, 0xeeULL, "/usr/lib/libdup-78.so", "dedupe"});
  rows.push_back({"CPP-NTF-0239", Category::PlaceholderOnly, 0x7ef000ULL, 0x7effffULL, 0xefULL, "/nonexistent/modules/libmissing-239.so", "placeholder"});
  rows.push_back({"CPP-NTF-0240", Category::StrictPath, 0x7f0000ULL, 0x7f0fffULL, 0xf0ULL, "/opt/altroot/lib/libsame-name-240.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0241", Category::Valid, 0x7f1000ULL, 0x7f1fffULL, 0xf1ULL, "/usr/lib/libcase-241.so", "module"});
  rows.push_back({"CPP-NTF-0242", Category::EmptyPath, 0x7f2000ULL, 0x7f2fffULL, 0xf2ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0243", Category::MalformedRange, 0x7f3000ULL, 0x7f3fffULL, 0xf3ULL, "/usr/lib/libcase-243.so", "ignore"});
  rows.push_back({"CPP-NTF-0244", Category::DuplicatePath, 0x7f4000ULL, 0x7f4fffULL, 0xf4ULL, "/usr/lib/libdup-4.so", "dedupe"});
  rows.push_back({"CPP-NTF-0245", Category::PlaceholderOnly, 0x7f5000ULL, 0x7f5fffULL, 0xf5ULL, "/nonexistent/modules/libmissing-245.so", "placeholder"});
  rows.push_back({"CPP-NTF-0246", Category::StrictPath, 0x7f6000ULL, 0x7f6fffULL, 0xf6ULL, "/opt/altroot/lib/libsame-name-246.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0247", Category::Valid, 0x7f7000ULL, 0x7f7fffULL, 0xf7ULL, "/usr/lib/libcase-247.so", "module"});
  rows.push_back({"CPP-NTF-0248", Category::EmptyPath, 0x7f8000ULL, 0x7f8fffULL, 0xf8ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0249", Category::MalformedRange, 0x7f9000ULL, 0x7f9fffULL, 0xf9ULL, "/usr/lib/libcase-249.so", "ignore"});
  rows.push_back({"CPP-NTF-0250", Category::DuplicatePath, 0x7fa000ULL, 0x7fafffULL, 0xfaULL, "/usr/lib/libdup-10.so", "dedupe"});
  rows.push_back({"CPP-NTF-0251", Category::PlaceholderOnly, 0x7fb000ULL, 0x7fbfffULL, 0xfbULL, "/nonexistent/modules/libmissing-251.so", "placeholder"});
  rows.push_back({"CPP-NTF-0252", Category::StrictPath, 0x7fc000ULL, 0x7fcfffULL, 0xfcULL, "/opt/altroot/lib/libsame-name-252.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0253", Category::Valid, 0x7fd000ULL, 0x7fdfffULL, 0xfdULL, "/usr/lib/libcase-253.so", "module"});
  rows.push_back({"CPP-NTF-0254", Category::EmptyPath, 0x7fe000ULL, 0x7fefffULL, 0xfeULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0255", Category::MalformedRange, 0x7ff000ULL, 0x7fffffULL, 0xffULL, "/usr/lib/libcase-255.so", "ignore"});
  rows.push_back({"CPP-NTF-0256", Category::DuplicatePath, 0x800000ULL, 0x800fffULL, 0x100ULL, "/usr/lib/libdup-16.so", "dedupe"});
  rows.push_back({"CPP-NTF-0257", Category::PlaceholderOnly, 0x801000ULL, 0x801fffULL, 0x101ULL, "/nonexistent/modules/libmissing-257.so", "placeholder"});
  rows.push_back({"CPP-NTF-0258", Category::StrictPath, 0x802000ULL, 0x802fffULL, 0x102ULL, "/opt/altroot/lib/libsame-name-258.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0259", Category::Valid, 0x803000ULL, 0x803fffULL, 0x103ULL, "/usr/lib/libcase-259.so", "module"});
  rows.push_back({"CPP-NTF-0260", Category::EmptyPath, 0x804000ULL, 0x804fffULL, 0x104ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0261", Category::MalformedRange, 0x805000ULL, 0x805fffULL, 0x105ULL, "/usr/lib/libcase-261.so", "ignore"});
  rows.push_back({"CPP-NTF-0262", Category::DuplicatePath, 0x806000ULL, 0x806fffULL, 0x106ULL, "/usr/lib/libdup-22.so", "dedupe"});
  rows.push_back({"CPP-NTF-0263", Category::PlaceholderOnly, 0x807000ULL, 0x807fffULL, 0x107ULL, "/nonexistent/modules/libmissing-263.so", "placeholder"});
  rows.push_back({"CPP-NTF-0264", Category::StrictPath, 0x808000ULL, 0x808fffULL, 0x108ULL, "/opt/altroot/lib/libsame-name-264.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0265", Category::Valid, 0x809000ULL, 0x809fffULL, 0x109ULL, "/usr/lib/libcase-265.so", "module"});
  rows.push_back({"CPP-NTF-0266", Category::EmptyPath, 0x80a000ULL, 0x80afffULL, 0x10aULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0267", Category::MalformedRange, 0x80b000ULL, 0x80bfffULL, 0x10bULL, "/usr/lib/libcase-267.so", "ignore"});
  rows.push_back({"CPP-NTF-0268", Category::DuplicatePath, 0x80c000ULL, 0x80cfffULL, 0x10cULL, "/usr/lib/libdup-28.so", "dedupe"});
  rows.push_back({"CPP-NTF-0269", Category::PlaceholderOnly, 0x80d000ULL, 0x80dfffULL, 0x10dULL, "/nonexistent/modules/libmissing-269.so", "placeholder"});
  rows.push_back({"CPP-NTF-0270", Category::StrictPath, 0x80e000ULL, 0x80efffULL, 0x10eULL, "/opt/altroot/lib/libsame-name-270.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0271", Category::Valid, 0x80f000ULL, 0x80ffffULL, 0x10fULL, "/usr/lib/libcase-271.so", "module"});
  rows.push_back({"CPP-NTF-0272", Category::EmptyPath, 0x810000ULL, 0x810fffULL, 0x110ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0273", Category::MalformedRange, 0x811000ULL, 0x811fffULL, 0x111ULL, "/usr/lib/libcase-273.so", "ignore"});
  rows.push_back({"CPP-NTF-0274", Category::DuplicatePath, 0x812000ULL, 0x812fffULL, 0x112ULL, "/usr/lib/libdup-34.so", "dedupe"});
  rows.push_back({"CPP-NTF-0275", Category::PlaceholderOnly, 0x813000ULL, 0x813fffULL, 0x113ULL, "/nonexistent/modules/libmissing-275.so", "placeholder"});
  rows.push_back({"CPP-NTF-0276", Category::StrictPath, 0x814000ULL, 0x814fffULL, 0x114ULL, "/opt/altroot/lib/libsame-name-276.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0277", Category::Valid, 0x815000ULL, 0x815fffULL, 0x115ULL, "/usr/lib/libcase-277.so", "module"});
  rows.push_back({"CPP-NTF-0278", Category::EmptyPath, 0x816000ULL, 0x816fffULL, 0x116ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0279", Category::MalformedRange, 0x817000ULL, 0x817fffULL, 0x117ULL, "/usr/lib/libcase-279.so", "ignore"});
  rows.push_back({"CPP-NTF-0280", Category::DuplicatePath, 0x818000ULL, 0x818fffULL, 0x118ULL, "/usr/lib/libdup-40.so", "dedupe"});
  rows.push_back({"CPP-NTF-0281", Category::PlaceholderOnly, 0x819000ULL, 0x819fffULL, 0x119ULL, "/nonexistent/modules/libmissing-281.so", "placeholder"});
  rows.push_back({"CPP-NTF-0282", Category::StrictPath, 0x81a000ULL, 0x81afffULL, 0x11aULL, "/opt/altroot/lib/libsame-name-282.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0283", Category::Valid, 0x81b000ULL, 0x81bfffULL, 0x11bULL, "/usr/lib/libcase-283.so", "module"});
  rows.push_back({"CPP-NTF-0284", Category::EmptyPath, 0x81c000ULL, 0x81cfffULL, 0x11cULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0285", Category::MalformedRange, 0x81d000ULL, 0x81dfffULL, 0x11dULL, "/usr/lib/libcase-285.so", "ignore"});
  rows.push_back({"CPP-NTF-0286", Category::DuplicatePath, 0x81e000ULL, 0x81efffULL, 0x11eULL, "/usr/lib/libdup-46.so", "dedupe"});
  rows.push_back({"CPP-NTF-0287", Category::PlaceholderOnly, 0x81f000ULL, 0x81ffffULL, 0x11fULL, "/nonexistent/modules/libmissing-287.so", "placeholder"});
  rows.push_back({"CPP-NTF-0288", Category::StrictPath, 0x820000ULL, 0x820fffULL, 0x120ULL, "/opt/altroot/lib/libsame-name-288.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0289", Category::Valid, 0x821000ULL, 0x821fffULL, 0x121ULL, "/usr/lib/libcase-289.so", "module"});
  rows.push_back({"CPP-NTF-0290", Category::EmptyPath, 0x822000ULL, 0x822fffULL, 0x122ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0291", Category::MalformedRange, 0x823000ULL, 0x823fffULL, 0x123ULL, "/usr/lib/libcase-291.so", "ignore"});
  rows.push_back({"CPP-NTF-0292", Category::DuplicatePath, 0x824000ULL, 0x824fffULL, 0x124ULL, "/usr/lib/libdup-52.so", "dedupe"});
  rows.push_back({"CPP-NTF-0293", Category::PlaceholderOnly, 0x825000ULL, 0x825fffULL, 0x125ULL, "/nonexistent/modules/libmissing-293.so", "placeholder"});
  rows.push_back({"CPP-NTF-0294", Category::StrictPath, 0x826000ULL, 0x826fffULL, 0x126ULL, "/opt/altroot/lib/libsame-name-294.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0295", Category::Valid, 0x827000ULL, 0x827fffULL, 0x127ULL, "/usr/lib/libcase-295.so", "module"});
  rows.push_back({"CPP-NTF-0296", Category::EmptyPath, 0x828000ULL, 0x828fffULL, 0x128ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0297", Category::MalformedRange, 0x829000ULL, 0x829fffULL, 0x129ULL, "/usr/lib/libcase-297.so", "ignore"});
  rows.push_back({"CPP-NTF-0298", Category::DuplicatePath, 0x82a000ULL, 0x82afffULL, 0x12aULL, "/usr/lib/libdup-58.so", "dedupe"});
  rows.push_back({"CPP-NTF-0299", Category::PlaceholderOnly, 0x82b000ULL, 0x82bfffULL, 0x12bULL, "/nonexistent/modules/libmissing-299.so", "placeholder"});
  rows.push_back({"CPP-NTF-0300", Category::StrictPath, 0x82c000ULL, 0x82cfffULL, 0x12cULL, "/opt/altroot/lib/libsame-name-300.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0301", Category::Valid, 0x82d000ULL, 0x82dfffULL, 0x12dULL, "/usr/lib/libcase-301.so", "module"});
  rows.push_back({"CPP-NTF-0302", Category::EmptyPath, 0x82e000ULL, 0x82efffULL, 0x12eULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0303", Category::MalformedRange, 0x82f000ULL, 0x82ffffULL, 0x12fULL, "/usr/lib/libcase-303.so", "ignore"});
  rows.push_back({"CPP-NTF-0304", Category::DuplicatePath, 0x830000ULL, 0x830fffULL, 0x130ULL, "/usr/lib/libdup-64.so", "dedupe"});
  rows.push_back({"CPP-NTF-0305", Category::PlaceholderOnly, 0x831000ULL, 0x831fffULL, 0x131ULL, "/nonexistent/modules/libmissing-305.so", "placeholder"});
  rows.push_back({"CPP-NTF-0306", Category::StrictPath, 0x832000ULL, 0x832fffULL, 0x132ULL, "/opt/altroot/lib/libsame-name-306.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0307", Category::Valid, 0x833000ULL, 0x833fffULL, 0x133ULL, "/usr/lib/libcase-307.so", "module"});
  rows.push_back({"CPP-NTF-0308", Category::EmptyPath, 0x834000ULL, 0x834fffULL, 0x134ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0309", Category::MalformedRange, 0x835000ULL, 0x835fffULL, 0x135ULL, "/usr/lib/libcase-309.so", "ignore"});
  rows.push_back({"CPP-NTF-0310", Category::DuplicatePath, 0x836000ULL, 0x836fffULL, 0x136ULL, "/usr/lib/libdup-70.so", "dedupe"});
  rows.push_back({"CPP-NTF-0311", Category::PlaceholderOnly, 0x837000ULL, 0x837fffULL, 0x137ULL, "/nonexistent/modules/libmissing-311.so", "placeholder"});
  rows.push_back({"CPP-NTF-0312", Category::StrictPath, 0x838000ULL, 0x838fffULL, 0x138ULL, "/opt/altroot/lib/libsame-name-312.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0313", Category::Valid, 0x839000ULL, 0x839fffULL, 0x139ULL, "/usr/lib/libcase-313.so", "module"});
  rows.push_back({"CPP-NTF-0314", Category::EmptyPath, 0x83a000ULL, 0x83afffULL, 0x13aULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0315", Category::MalformedRange, 0x83b000ULL, 0x83bfffULL, 0x13bULL, "/usr/lib/libcase-315.so", "ignore"});
  rows.push_back({"CPP-NTF-0316", Category::DuplicatePath, 0x83c000ULL, 0x83cfffULL, 0x13cULL, "/usr/lib/libdup-76.so", "dedupe"});
  rows.push_back({"CPP-NTF-0317", Category::PlaceholderOnly, 0x83d000ULL, 0x83dfffULL, 0x13dULL, "/nonexistent/modules/libmissing-317.so", "placeholder"});
  rows.push_back({"CPP-NTF-0318", Category::StrictPath, 0x83e000ULL, 0x83efffULL, 0x13eULL, "/opt/altroot/lib/libsame-name-318.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0319", Category::Valid, 0x83f000ULL, 0x83ffffULL, 0x13fULL, "/usr/lib/libcase-319.so", "module"});
  rows.push_back({"CPP-NTF-0320", Category::EmptyPath, 0x840000ULL, 0x840fffULL, 0x140ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0321", Category::MalformedRange, 0x841000ULL, 0x841fffULL, 0x141ULL, "/usr/lib/libcase-321.so", "ignore"});
  rows.push_back({"CPP-NTF-0322", Category::DuplicatePath, 0x842000ULL, 0x842fffULL, 0x142ULL, "/usr/lib/libdup-2.so", "dedupe"});
  rows.push_back({"CPP-NTF-0323", Category::PlaceholderOnly, 0x843000ULL, 0x843fffULL, 0x143ULL, "/nonexistent/modules/libmissing-323.so", "placeholder"});
  rows.push_back({"CPP-NTF-0324", Category::StrictPath, 0x844000ULL, 0x844fffULL, 0x144ULL, "/opt/altroot/lib/libsame-name-324.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0325", Category::Valid, 0x845000ULL, 0x845fffULL, 0x145ULL, "/usr/lib/libcase-325.so", "module"});
  rows.push_back({"CPP-NTF-0326", Category::EmptyPath, 0x846000ULL, 0x846fffULL, 0x146ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0327", Category::MalformedRange, 0x847000ULL, 0x847fffULL, 0x147ULL, "/usr/lib/libcase-327.so", "ignore"});
  rows.push_back({"CPP-NTF-0328", Category::DuplicatePath, 0x848000ULL, 0x848fffULL, 0x148ULL, "/usr/lib/libdup-8.so", "dedupe"});
  rows.push_back({"CPP-NTF-0329", Category::PlaceholderOnly, 0x849000ULL, 0x849fffULL, 0x149ULL, "/nonexistent/modules/libmissing-329.so", "placeholder"});
  rows.push_back({"CPP-NTF-0330", Category::StrictPath, 0x84a000ULL, 0x84afffULL, 0x14aULL, "/opt/altroot/lib/libsame-name-330.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0331", Category::Valid, 0x84b000ULL, 0x84bfffULL, 0x14bULL, "/usr/lib/libcase-331.so", "module"});
  rows.push_back({"CPP-NTF-0332", Category::EmptyPath, 0x84c000ULL, 0x84cfffULL, 0x14cULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0333", Category::MalformedRange, 0x84d000ULL, 0x84dfffULL, 0x14dULL, "/usr/lib/libcase-333.so", "ignore"});
  rows.push_back({"CPP-NTF-0334", Category::DuplicatePath, 0x84e000ULL, 0x84efffULL, 0x14eULL, "/usr/lib/libdup-14.so", "dedupe"});
  rows.push_back({"CPP-NTF-0335", Category::PlaceholderOnly, 0x84f000ULL, 0x84ffffULL, 0x14fULL, "/nonexistent/modules/libmissing-335.so", "placeholder"});
  rows.push_back({"CPP-NTF-0336", Category::StrictPath, 0x850000ULL, 0x850fffULL, 0x150ULL, "/opt/altroot/lib/libsame-name-336.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0337", Category::Valid, 0x851000ULL, 0x851fffULL, 0x151ULL, "/usr/lib/libcase-337.so", "module"});
  rows.push_back({"CPP-NTF-0338", Category::EmptyPath, 0x852000ULL, 0x852fffULL, 0x152ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0339", Category::MalformedRange, 0x853000ULL, 0x853fffULL, 0x153ULL, "/usr/lib/libcase-339.so", "ignore"});
  rows.push_back({"CPP-NTF-0340", Category::DuplicatePath, 0x854000ULL, 0x854fffULL, 0x154ULL, "/usr/lib/libdup-20.so", "dedupe"});
  rows.push_back({"CPP-NTF-0341", Category::PlaceholderOnly, 0x855000ULL, 0x855fffULL, 0x155ULL, "/nonexistent/modules/libmissing-341.so", "placeholder"});
  rows.push_back({"CPP-NTF-0342", Category::StrictPath, 0x856000ULL, 0x856fffULL, 0x156ULL, "/opt/altroot/lib/libsame-name-342.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0343", Category::Valid, 0x857000ULL, 0x857fffULL, 0x157ULL, "/usr/lib/libcase-343.so", "module"});
  rows.push_back({"CPP-NTF-0344", Category::EmptyPath, 0x858000ULL, 0x858fffULL, 0x158ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0345", Category::MalformedRange, 0x859000ULL, 0x859fffULL, 0x159ULL, "/usr/lib/libcase-345.so", "ignore"});
  rows.push_back({"CPP-NTF-0346", Category::DuplicatePath, 0x85a000ULL, 0x85afffULL, 0x15aULL, "/usr/lib/libdup-26.so", "dedupe"});
  rows.push_back({"CPP-NTF-0347", Category::PlaceholderOnly, 0x85b000ULL, 0x85bfffULL, 0x15bULL, "/nonexistent/modules/libmissing-347.so", "placeholder"});
  rows.push_back({"CPP-NTF-0348", Category::StrictPath, 0x85c000ULL, 0x85cfffULL, 0x15cULL, "/opt/altroot/lib/libsame-name-348.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0349", Category::Valid, 0x85d000ULL, 0x85dfffULL, 0x15dULL, "/usr/lib/libcase-349.so", "module"});
  rows.push_back({"CPP-NTF-0350", Category::EmptyPath, 0x85e000ULL, 0x85efffULL, 0x15eULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0351", Category::MalformedRange, 0x85f000ULL, 0x85ffffULL, 0x15fULL, "/usr/lib/libcase-351.so", "ignore"});
  rows.push_back({"CPP-NTF-0352", Category::DuplicatePath, 0x860000ULL, 0x860fffULL, 0x160ULL, "/usr/lib/libdup-32.so", "dedupe"});
  rows.push_back({"CPP-NTF-0353", Category::PlaceholderOnly, 0x861000ULL, 0x861fffULL, 0x161ULL, "/nonexistent/modules/libmissing-353.so", "placeholder"});
  rows.push_back({"CPP-NTF-0354", Category::StrictPath, 0x862000ULL, 0x862fffULL, 0x162ULL, "/opt/altroot/lib/libsame-name-354.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0355", Category::Valid, 0x863000ULL, 0x863fffULL, 0x163ULL, "/usr/lib/libcase-355.so", "module"});
  rows.push_back({"CPP-NTF-0356", Category::EmptyPath, 0x864000ULL, 0x864fffULL, 0x164ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0357", Category::MalformedRange, 0x865000ULL, 0x865fffULL, 0x165ULL, "/usr/lib/libcase-357.so", "ignore"});
  rows.push_back({"CPP-NTF-0358", Category::DuplicatePath, 0x866000ULL, 0x866fffULL, 0x166ULL, "/usr/lib/libdup-38.so", "dedupe"});
  rows.push_back({"CPP-NTF-0359", Category::PlaceholderOnly, 0x867000ULL, 0x867fffULL, 0x167ULL, "/nonexistent/modules/libmissing-359.so", "placeholder"});
  rows.push_back({"CPP-NTF-0360", Category::StrictPath, 0x868000ULL, 0x868fffULL, 0x168ULL, "/opt/altroot/lib/libsame-name-360.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0361", Category::Valid, 0x869000ULL, 0x869fffULL, 0x169ULL, "/usr/lib/libcase-361.so", "module"});
  rows.push_back({"CPP-NTF-0362", Category::EmptyPath, 0x86a000ULL, 0x86afffULL, 0x16aULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0363", Category::MalformedRange, 0x86b000ULL, 0x86bfffULL, 0x16bULL, "/usr/lib/libcase-363.so", "ignore"});
  rows.push_back({"CPP-NTF-0364", Category::DuplicatePath, 0x86c000ULL, 0x86cfffULL, 0x16cULL, "/usr/lib/libdup-44.so", "dedupe"});
  rows.push_back({"CPP-NTF-0365", Category::PlaceholderOnly, 0x86d000ULL, 0x86dfffULL, 0x16dULL, "/nonexistent/modules/libmissing-365.so", "placeholder"});
  rows.push_back({"CPP-NTF-0366", Category::StrictPath, 0x86e000ULL, 0x86efffULL, 0x16eULL, "/opt/altroot/lib/libsame-name-366.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0367", Category::Valid, 0x86f000ULL, 0x86ffffULL, 0x16fULL, "/usr/lib/libcase-367.so", "module"});
  rows.push_back({"CPP-NTF-0368", Category::EmptyPath, 0x870000ULL, 0x870fffULL, 0x170ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0369", Category::MalformedRange, 0x871000ULL, 0x871fffULL, 0x171ULL, "/usr/lib/libcase-369.so", "ignore"});
  rows.push_back({"CPP-NTF-0370", Category::DuplicatePath, 0x872000ULL, 0x872fffULL, 0x172ULL, "/usr/lib/libdup-50.so", "dedupe"});
  rows.push_back({"CPP-NTF-0371", Category::PlaceholderOnly, 0x873000ULL, 0x873fffULL, 0x173ULL, "/nonexistent/modules/libmissing-371.so", "placeholder"});
  rows.push_back({"CPP-NTF-0372", Category::StrictPath, 0x874000ULL, 0x874fffULL, 0x174ULL, "/opt/altroot/lib/libsame-name-372.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0373", Category::Valid, 0x875000ULL, 0x875fffULL, 0x175ULL, "/usr/lib/libcase-373.so", "module"});
  rows.push_back({"CPP-NTF-0374", Category::EmptyPath, 0x876000ULL, 0x876fffULL, 0x176ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0375", Category::MalformedRange, 0x877000ULL, 0x877fffULL, 0x177ULL, "/usr/lib/libcase-375.so", "ignore"});
  rows.push_back({"CPP-NTF-0376", Category::DuplicatePath, 0x878000ULL, 0x878fffULL, 0x178ULL, "/usr/lib/libdup-56.so", "dedupe"});
  rows.push_back({"CPP-NTF-0377", Category::PlaceholderOnly, 0x879000ULL, 0x879fffULL, 0x179ULL, "/nonexistent/modules/libmissing-377.so", "placeholder"});
  rows.push_back({"CPP-NTF-0378", Category::StrictPath, 0x87a000ULL, 0x87afffULL, 0x17aULL, "/opt/altroot/lib/libsame-name-378.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0379", Category::Valid, 0x87b000ULL, 0x87bfffULL, 0x17bULL, "/usr/lib/libcase-379.so", "module"});
  rows.push_back({"CPP-NTF-0380", Category::EmptyPath, 0x87c000ULL, 0x87cfffULL, 0x17cULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0381", Category::MalformedRange, 0x87d000ULL, 0x87dfffULL, 0x17dULL, "/usr/lib/libcase-381.so", "ignore"});
  rows.push_back({"CPP-NTF-0382", Category::DuplicatePath, 0x87e000ULL, 0x87efffULL, 0x17eULL, "/usr/lib/libdup-62.so", "dedupe"});
  rows.push_back({"CPP-NTF-0383", Category::PlaceholderOnly, 0x87f000ULL, 0x87ffffULL, 0x17fULL, "/nonexistent/modules/libmissing-383.so", "placeholder"});
  rows.push_back({"CPP-NTF-0384", Category::StrictPath, 0x880000ULL, 0x880fffULL, 0x180ULL, "/opt/altroot/lib/libsame-name-384.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0385", Category::Valid, 0x881000ULL, 0x881fffULL, 0x181ULL, "/usr/lib/libcase-385.so", "module"});
  rows.push_back({"CPP-NTF-0386", Category::EmptyPath, 0x882000ULL, 0x882fffULL, 0x182ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0387", Category::MalformedRange, 0x883000ULL, 0x883fffULL, 0x183ULL, "/usr/lib/libcase-387.so", "ignore"});
  rows.push_back({"CPP-NTF-0388", Category::DuplicatePath, 0x884000ULL, 0x884fffULL, 0x184ULL, "/usr/lib/libdup-68.so", "dedupe"});
  rows.push_back({"CPP-NTF-0389", Category::PlaceholderOnly, 0x885000ULL, 0x885fffULL, 0x185ULL, "/nonexistent/modules/libmissing-389.so", "placeholder"});
  rows.push_back({"CPP-NTF-0390", Category::StrictPath, 0x886000ULL, 0x886fffULL, 0x186ULL, "/opt/altroot/lib/libsame-name-390.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0391", Category::Valid, 0x887000ULL, 0x887fffULL, 0x187ULL, "/usr/lib/libcase-391.so", "module"});
  rows.push_back({"CPP-NTF-0392", Category::EmptyPath, 0x888000ULL, 0x888fffULL, 0x188ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0393", Category::MalformedRange, 0x889000ULL, 0x889fffULL, 0x189ULL, "/usr/lib/libcase-393.so", "ignore"});
  rows.push_back({"CPP-NTF-0394", Category::DuplicatePath, 0x88a000ULL, 0x88afffULL, 0x18aULL, "/usr/lib/libdup-74.so", "dedupe"});
  rows.push_back({"CPP-NTF-0395", Category::PlaceholderOnly, 0x88b000ULL, 0x88bfffULL, 0x18bULL, "/nonexistent/modules/libmissing-395.so", "placeholder"});
  rows.push_back({"CPP-NTF-0396", Category::StrictPath, 0x88c000ULL, 0x88cfffULL, 0x18cULL, "/opt/altroot/lib/libsame-name-396.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0397", Category::Valid, 0x88d000ULL, 0x88dfffULL, 0x18dULL, "/usr/lib/libcase-397.so", "module"});
  rows.push_back({"CPP-NTF-0398", Category::EmptyPath, 0x88e000ULL, 0x88efffULL, 0x18eULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0399", Category::MalformedRange, 0x88f000ULL, 0x88ffffULL, 0x18fULL, "/usr/lib/libcase-399.so", "ignore"});
  rows.push_back({"CPP-NTF-0400", Category::DuplicatePath, 0x890000ULL, 0x890fffULL, 0x190ULL, "/usr/lib/libdup-0.so", "dedupe"});
  rows.push_back({"CPP-NTF-0401", Category::PlaceholderOnly, 0x891000ULL, 0x891fffULL, 0x191ULL, "/nonexistent/modules/libmissing-401.so", "placeholder"});
  rows.push_back({"CPP-NTF-0402", Category::StrictPath, 0x892000ULL, 0x892fffULL, 0x192ULL, "/opt/altroot/lib/libsame-name-402.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0403", Category::Valid, 0x893000ULL, 0x893fffULL, 0x193ULL, "/usr/lib/libcase-403.so", "module"});
  rows.push_back({"CPP-NTF-0404", Category::EmptyPath, 0x894000ULL, 0x894fffULL, 0x194ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0405", Category::MalformedRange, 0x895000ULL, 0x895fffULL, 0x195ULL, "/usr/lib/libcase-405.so", "ignore"});
  rows.push_back({"CPP-NTF-0406", Category::DuplicatePath, 0x896000ULL, 0x896fffULL, 0x196ULL, "/usr/lib/libdup-6.so", "dedupe"});
  rows.push_back({"CPP-NTF-0407", Category::PlaceholderOnly, 0x897000ULL, 0x897fffULL, 0x197ULL, "/nonexistent/modules/libmissing-407.so", "placeholder"});
  rows.push_back({"CPP-NTF-0408", Category::StrictPath, 0x898000ULL, 0x898fffULL, 0x198ULL, "/opt/altroot/lib/libsame-name-408.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0409", Category::Valid, 0x899000ULL, 0x899fffULL, 0x199ULL, "/usr/lib/libcase-409.so", "module"});
  rows.push_back({"CPP-NTF-0410", Category::EmptyPath, 0x89a000ULL, 0x89afffULL, 0x19aULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0411", Category::MalformedRange, 0x89b000ULL, 0x89bfffULL, 0x19bULL, "/usr/lib/libcase-411.so", "ignore"});
  rows.push_back({"CPP-NTF-0412", Category::DuplicatePath, 0x89c000ULL, 0x89cfffULL, 0x19cULL, "/usr/lib/libdup-12.so", "dedupe"});
  rows.push_back({"CPP-NTF-0413", Category::PlaceholderOnly, 0x89d000ULL, 0x89dfffULL, 0x19dULL, "/nonexistent/modules/libmissing-413.so", "placeholder"});
  rows.push_back({"CPP-NTF-0414", Category::StrictPath, 0x89e000ULL, 0x89efffULL, 0x19eULL, "/opt/altroot/lib/libsame-name-414.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0415", Category::Valid, 0x89f000ULL, 0x89ffffULL, 0x19fULL, "/usr/lib/libcase-415.so", "module"});
  rows.push_back({"CPP-NTF-0416", Category::EmptyPath, 0x8a0000ULL, 0x8a0fffULL, 0x1a0ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0417", Category::MalformedRange, 0x8a1000ULL, 0x8a1fffULL, 0x1a1ULL, "/usr/lib/libcase-417.so", "ignore"});
  rows.push_back({"CPP-NTF-0418", Category::DuplicatePath, 0x8a2000ULL, 0x8a2fffULL, 0x1a2ULL, "/usr/lib/libdup-18.so", "dedupe"});
  rows.push_back({"CPP-NTF-0419", Category::PlaceholderOnly, 0x8a3000ULL, 0x8a3fffULL, 0x1a3ULL, "/nonexistent/modules/libmissing-419.so", "placeholder"});
  rows.push_back({"CPP-NTF-0420", Category::StrictPath, 0x8a4000ULL, 0x8a4fffULL, 0x1a4ULL, "/opt/altroot/lib/libsame-name-420.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0421", Category::Valid, 0x8a5000ULL, 0x8a5fffULL, 0x1a5ULL, "/usr/lib/libcase-421.so", "module"});
  rows.push_back({"CPP-NTF-0422", Category::EmptyPath, 0x8a6000ULL, 0x8a6fffULL, 0x1a6ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0423", Category::MalformedRange, 0x8a7000ULL, 0x8a7fffULL, 0x1a7ULL, "/usr/lib/libcase-423.so", "ignore"});
  rows.push_back({"CPP-NTF-0424", Category::DuplicatePath, 0x8a8000ULL, 0x8a8fffULL, 0x1a8ULL, "/usr/lib/libdup-24.so", "dedupe"});
  rows.push_back({"CPP-NTF-0425", Category::PlaceholderOnly, 0x8a9000ULL, 0x8a9fffULL, 0x1a9ULL, "/nonexistent/modules/libmissing-425.so", "placeholder"});
  rows.push_back({"CPP-NTF-0426", Category::StrictPath, 0x8aa000ULL, 0x8aafffULL, 0x1aaULL, "/opt/altroot/lib/libsame-name-426.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0427", Category::Valid, 0x8ab000ULL, 0x8abfffULL, 0x1abULL, "/usr/lib/libcase-427.so", "module"});
  rows.push_back({"CPP-NTF-0428", Category::EmptyPath, 0x8ac000ULL, 0x8acfffULL, 0x1acULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0429", Category::MalformedRange, 0x8ad000ULL, 0x8adfffULL, 0x1adULL, "/usr/lib/libcase-429.so", "ignore"});
  rows.push_back({"CPP-NTF-0430", Category::DuplicatePath, 0x8ae000ULL, 0x8aefffULL, 0x1aeULL, "/usr/lib/libdup-30.so", "dedupe"});
  rows.push_back({"CPP-NTF-0431", Category::PlaceholderOnly, 0x8af000ULL, 0x8affffULL, 0x1afULL, "/nonexistent/modules/libmissing-431.so", "placeholder"});
  rows.push_back({"CPP-NTF-0432", Category::StrictPath, 0x8b0000ULL, 0x8b0fffULL, 0x1b0ULL, "/opt/altroot/lib/libsame-name-432.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0433", Category::Valid, 0x8b1000ULL, 0x8b1fffULL, 0x1b1ULL, "/usr/lib/libcase-433.so", "module"});
  rows.push_back({"CPP-NTF-0434", Category::EmptyPath, 0x8b2000ULL, 0x8b2fffULL, 0x1b2ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0435", Category::MalformedRange, 0x8b3000ULL, 0x8b3fffULL, 0x1b3ULL, "/usr/lib/libcase-435.so", "ignore"});
  rows.push_back({"CPP-NTF-0436", Category::DuplicatePath, 0x8b4000ULL, 0x8b4fffULL, 0x1b4ULL, "/usr/lib/libdup-36.so", "dedupe"});
  rows.push_back({"CPP-NTF-0437", Category::PlaceholderOnly, 0x8b5000ULL, 0x8b5fffULL, 0x1b5ULL, "/nonexistent/modules/libmissing-437.so", "placeholder"});
  rows.push_back({"CPP-NTF-0438", Category::StrictPath, 0x8b6000ULL, 0x8b6fffULL, 0x1b6ULL, "/opt/altroot/lib/libsame-name-438.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0439", Category::Valid, 0x8b7000ULL, 0x8b7fffULL, 0x1b7ULL, "/usr/lib/libcase-439.so", "module"});
  rows.push_back({"CPP-NTF-0440", Category::EmptyPath, 0x8b8000ULL, 0x8b8fffULL, 0x1b8ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0441", Category::MalformedRange, 0x8b9000ULL, 0x8b9fffULL, 0x1b9ULL, "/usr/lib/libcase-441.so", "ignore"});
  rows.push_back({"CPP-NTF-0442", Category::DuplicatePath, 0x8ba000ULL, 0x8bafffULL, 0x1baULL, "/usr/lib/libdup-42.so", "dedupe"});
  rows.push_back({"CPP-NTF-0443", Category::PlaceholderOnly, 0x8bb000ULL, 0x8bbfffULL, 0x1bbULL, "/nonexistent/modules/libmissing-443.so", "placeholder"});
  rows.push_back({"CPP-NTF-0444", Category::StrictPath, 0x8bc000ULL, 0x8bcfffULL, 0x1bcULL, "/opt/altroot/lib/libsame-name-444.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0445", Category::Valid, 0x8bd000ULL, 0x8bdfffULL, 0x1bdULL, "/usr/lib/libcase-445.so", "module"});
  rows.push_back({"CPP-NTF-0446", Category::EmptyPath, 0x8be000ULL, 0x8befffULL, 0x1beULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0447", Category::MalformedRange, 0x8bf000ULL, 0x8bffffULL, 0x1bfULL, "/usr/lib/libcase-447.so", "ignore"});
  rows.push_back({"CPP-NTF-0448", Category::DuplicatePath, 0x8c0000ULL, 0x8c0fffULL, 0x1c0ULL, "/usr/lib/libdup-48.so", "dedupe"});
  rows.push_back({"CPP-NTF-0449", Category::PlaceholderOnly, 0x8c1000ULL, 0x8c1fffULL, 0x1c1ULL, "/nonexistent/modules/libmissing-449.so", "placeholder"});
  rows.push_back({"CPP-NTF-0450", Category::StrictPath, 0x8c2000ULL, 0x8c2fffULL, 0x1c2ULL, "/opt/altroot/lib/libsame-name-450.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0451", Category::Valid, 0x8c3000ULL, 0x8c3fffULL, 0x1c3ULL, "/usr/lib/libcase-451.so", "module"});
  rows.push_back({"CPP-NTF-0452", Category::EmptyPath, 0x8c4000ULL, 0x8c4fffULL, 0x1c4ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0453", Category::MalformedRange, 0x8c5000ULL, 0x8c5fffULL, 0x1c5ULL, "/usr/lib/libcase-453.so", "ignore"});
  rows.push_back({"CPP-NTF-0454", Category::DuplicatePath, 0x8c6000ULL, 0x8c6fffULL, 0x1c6ULL, "/usr/lib/libdup-54.so", "dedupe"});
  rows.push_back({"CPP-NTF-0455", Category::PlaceholderOnly, 0x8c7000ULL, 0x8c7fffULL, 0x1c7ULL, "/nonexistent/modules/libmissing-455.so", "placeholder"});
  rows.push_back({"CPP-NTF-0456", Category::StrictPath, 0x8c8000ULL, 0x8c8fffULL, 0x1c8ULL, "/opt/altroot/lib/libsame-name-456.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0457", Category::Valid, 0x8c9000ULL, 0x8c9fffULL, 0x1c9ULL, "/usr/lib/libcase-457.so", "module"});
  rows.push_back({"CPP-NTF-0458", Category::EmptyPath, 0x8ca000ULL, 0x8cafffULL, 0x1caULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0459", Category::MalformedRange, 0x8cb000ULL, 0x8cbfffULL, 0x1cbULL, "/usr/lib/libcase-459.so", "ignore"});
  rows.push_back({"CPP-NTF-0460", Category::DuplicatePath, 0x8cc000ULL, 0x8ccfffULL, 0x1ccULL, "/usr/lib/libdup-60.so", "dedupe"});
  rows.push_back({"CPP-NTF-0461", Category::PlaceholderOnly, 0x8cd000ULL, 0x8cdfffULL, 0x1cdULL, "/nonexistent/modules/libmissing-461.so", "placeholder"});
  rows.push_back({"CPP-NTF-0462", Category::StrictPath, 0x8ce000ULL, 0x8cefffULL, 0x1ceULL, "/opt/altroot/lib/libsame-name-462.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0463", Category::Valid, 0x8cf000ULL, 0x8cffffULL, 0x1cfULL, "/usr/lib/libcase-463.so", "module"});
  rows.push_back({"CPP-NTF-0464", Category::EmptyPath, 0x8d0000ULL, 0x8d0fffULL, 0x1d0ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0465", Category::MalformedRange, 0x8d1000ULL, 0x8d1fffULL, 0x1d1ULL, "/usr/lib/libcase-465.so", "ignore"});
  rows.push_back({"CPP-NTF-0466", Category::DuplicatePath, 0x8d2000ULL, 0x8d2fffULL, 0x1d2ULL, "/usr/lib/libdup-66.so", "dedupe"});
  rows.push_back({"CPP-NTF-0467", Category::PlaceholderOnly, 0x8d3000ULL, 0x8d3fffULL, 0x1d3ULL, "/nonexistent/modules/libmissing-467.so", "placeholder"});
  rows.push_back({"CPP-NTF-0468", Category::StrictPath, 0x8d4000ULL, 0x8d4fffULL, 0x1d4ULL, "/opt/altroot/lib/libsame-name-468.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0469", Category::Valid, 0x8d5000ULL, 0x8d5fffULL, 0x1d5ULL, "/usr/lib/libcase-469.so", "module"});
  rows.push_back({"CPP-NTF-0470", Category::EmptyPath, 0x8d6000ULL, 0x8d6fffULL, 0x1d6ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0471", Category::MalformedRange, 0x8d7000ULL, 0x8d7fffULL, 0x1d7ULL, "/usr/lib/libcase-471.so", "ignore"});
  rows.push_back({"CPP-NTF-0472", Category::DuplicatePath, 0x8d8000ULL, 0x8d8fffULL, 0x1d8ULL, "/usr/lib/libdup-72.so", "dedupe"});
  rows.push_back({"CPP-NTF-0473", Category::PlaceholderOnly, 0x8d9000ULL, 0x8d9fffULL, 0x1d9ULL, "/nonexistent/modules/libmissing-473.so", "placeholder"});
  rows.push_back({"CPP-NTF-0474", Category::StrictPath, 0x8da000ULL, 0x8dafffULL, 0x1daULL, "/opt/altroot/lib/libsame-name-474.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0475", Category::Valid, 0x8db000ULL, 0x8dbfffULL, 0x1dbULL, "/usr/lib/libcase-475.so", "module"});
  rows.push_back({"CPP-NTF-0476", Category::EmptyPath, 0x8dc000ULL, 0x8dcfffULL, 0x1dcULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0477", Category::MalformedRange, 0x8dd000ULL, 0x8ddfffULL, 0x1ddULL, "/usr/lib/libcase-477.so", "ignore"});
  rows.push_back({"CPP-NTF-0478", Category::DuplicatePath, 0x8de000ULL, 0x8defffULL, 0x1deULL, "/usr/lib/libdup-78.so", "dedupe"});
  rows.push_back({"CPP-NTF-0479", Category::PlaceholderOnly, 0x8df000ULL, 0x8dffffULL, 0x1dfULL, "/nonexistent/modules/libmissing-479.so", "placeholder"});
  rows.push_back({"CPP-NTF-0480", Category::StrictPath, 0x8e0000ULL, 0x8e0fffULL, 0x1e0ULL, "/opt/altroot/lib/libsame-name-480.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0481", Category::Valid, 0x8e1000ULL, 0x8e1fffULL, 0x1e1ULL, "/usr/lib/libcase-481.so", "module"});
  rows.push_back({"CPP-NTF-0482", Category::EmptyPath, 0x8e2000ULL, 0x8e2fffULL, 0x1e2ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0483", Category::MalformedRange, 0x8e3000ULL, 0x8e3fffULL, 0x1e3ULL, "/usr/lib/libcase-483.so", "ignore"});
  rows.push_back({"CPP-NTF-0484", Category::DuplicatePath, 0x8e4000ULL, 0x8e4fffULL, 0x1e4ULL, "/usr/lib/libdup-4.so", "dedupe"});
  rows.push_back({"CPP-NTF-0485", Category::PlaceholderOnly, 0x8e5000ULL, 0x8e5fffULL, 0x1e5ULL, "/nonexistent/modules/libmissing-485.so", "placeholder"});
  rows.push_back({"CPP-NTF-0486", Category::StrictPath, 0x8e6000ULL, 0x8e6fffULL, 0x1e6ULL, "/opt/altroot/lib/libsame-name-486.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0487", Category::Valid, 0x8e7000ULL, 0x8e7fffULL, 0x1e7ULL, "/usr/lib/libcase-487.so", "module"});
  rows.push_back({"CPP-NTF-0488", Category::EmptyPath, 0x8e8000ULL, 0x8e8fffULL, 0x1e8ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0489", Category::MalformedRange, 0x8e9000ULL, 0x8e9fffULL, 0x1e9ULL, "/usr/lib/libcase-489.so", "ignore"});
  rows.push_back({"CPP-NTF-0490", Category::DuplicatePath, 0x8ea000ULL, 0x8eafffULL, 0x1eaULL, "/usr/lib/libdup-10.so", "dedupe"});
  rows.push_back({"CPP-NTF-0491", Category::PlaceholderOnly, 0x8eb000ULL, 0x8ebfffULL, 0x1ebULL, "/nonexistent/modules/libmissing-491.so", "placeholder"});
  rows.push_back({"CPP-NTF-0492", Category::StrictPath, 0x8ec000ULL, 0x8ecfffULL, 0x1ecULL, "/opt/altroot/lib/libsame-name-492.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0493", Category::Valid, 0x8ed000ULL, 0x8edfffULL, 0x1edULL, "/usr/lib/libcase-493.so", "module"});
  rows.push_back({"CPP-NTF-0494", Category::EmptyPath, 0x8ee000ULL, 0x8eefffULL, 0x1eeULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0495", Category::MalformedRange, 0x8ef000ULL, 0x8effffULL, 0x1efULL, "/usr/lib/libcase-495.so", "ignore"});
  rows.push_back({"CPP-NTF-0496", Category::DuplicatePath, 0x8f0000ULL, 0x8f0fffULL, 0x1f0ULL, "/usr/lib/libdup-16.so", "dedupe"});
  rows.push_back({"CPP-NTF-0497", Category::PlaceholderOnly, 0x8f1000ULL, 0x8f1fffULL, 0x1f1ULL, "/nonexistent/modules/libmissing-497.so", "placeholder"});
  rows.push_back({"CPP-NTF-0498", Category::StrictPath, 0x8f2000ULL, 0x8f2fffULL, 0x1f2ULL, "/opt/altroot/lib/libsame-name-498.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0499", Category::Valid, 0x8f3000ULL, 0x8f3fffULL, 0x1f3ULL, "/usr/lib/libcase-499.so", "module"});
  rows.push_back({"CPP-NTF-0500", Category::EmptyPath, 0x8f4000ULL, 0x8f4fffULL, 0x1f4ULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0501", Category::MalformedRange, 0x8f5000ULL, 0x8f5fffULL, 0x1f5ULL, "/usr/lib/libcase-501.so", "ignore"});
  rows.push_back({"CPP-NTF-0502", Category::DuplicatePath, 0x8f6000ULL, 0x8f6fffULL, 0x1f6ULL, "/usr/lib/libdup-22.so", "dedupe"});
  rows.push_back({"CPP-NTF-0503", Category::PlaceholderOnly, 0x8f7000ULL, 0x8f7fffULL, 0x1f7ULL, "/nonexistent/modules/libmissing-503.so", "placeholder"});
  rows.push_back({"CPP-NTF-0504", Category::StrictPath, 0x8f8000ULL, 0x8f8fffULL, 0x1f8ULL, "/opt/altroot/lib/libsame-name-504.so", "strict-no-basename"});
  rows.push_back({"CPP-NTF-0505", Category::Valid, 0x8f9000ULL, 0x8f9fffULL, 0x1f9ULL, "/usr/lib/libcase-505.so", "module"});
  rows.push_back({"CPP-NTF-0506", Category::EmptyPath, 0x8fa000ULL, 0x8fafffULL, 0x1faULL, "", "ignore"});
  rows.push_back({"CPP-NTF-0507", Category::MalformedRange, 0x8fb000ULL, 0x8fbfffULL, 0x1fbULL, "/usr/lib/libcase-507.so", "ignore"});
  rows.push_back({"CPP-NTF-0508", Category::DuplicatePath, 0x8fc000ULL, 0x8fcfffULL, 0x1fcULL, "/usr/lib/libdup-28.so", "dedupe"});
  rows.push_back({"CPP-NTF-0509", Category::PlaceholderOnly, 0x8fd000ULL, 0x8fdfffULL, 0x1fdULL, "/nonexistent/modules/libmissing-509.so", "placeholder"});
}

} // namespace ntfile_fixture
