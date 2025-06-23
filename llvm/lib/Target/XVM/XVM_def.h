#ifndef LLVM_LIB_TARGET_XVM_XVM_DEF_H
#define LLVM_LIB_TARGET_XVM_XVM_DEF_H

#include <string>
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

/* Flags for MCInst */
#define FUNC_CALL_FLAG_MC_INST_IMM  0x00000080
#define FUNC_CALL_FLAG_MC_INST_REG  0x00000081
#define FUNC_ID_FLAG_MC_INST_REG    0x00000082
#define GLOBAL_DATAREF_FLAG_MC_INST 0x00000083


namespace llvm {

/** XVM section permissions
  index: | 7 6 5 4 3  2  1  0
meaning: | reserved   X  W  R
*/
#define XVM_SECTION_PERM_UNKNOWN  0b00000000
#define XVM_SECTION_PERM_RO       0b00000001
#define XVM_SECTION_PERM_WO       0b00000010
#define XVM_SECTION_PERM_XO       0b00000100
#define XVM_SECTION_PERM_RW       0b00000011
#define XVM_SECTION_PERM_RX       0b00000101
#define XVM_SECTION_PERM_WX       0b00000110
#define XVM_SECTION_PERM_RWX      0b00000111

#define XVM_SECTION_DATA_TYPE_UNKNOWN     0b00000000
#define XVM_SECTION_DATA_TYPE_UNDEF       0b00000001
#define XVM_SECTION_DATA_TYPE_NUMERIC     0b00000010
#define XVM_SECTION_DATA_TYPE_POINTER     0b00000100
#define XVM_SECTION_DATA_TYPE_STRING      0b00001000
#define XVM_SECTION_DATA_TYPE_ARRAY       0b00010000
#define XVM_SECTION_DATA_TYPE_STRUCT      0b00100000
#define XVM_SECTION_DATA_TYPE_BSS         0b01000000

typedef struct XVMGVPatchInfo {
  std::string SymName;
  int AddEnd;
} XVMGVPathInfo;

typedef struct XVMSectionInfo {
    unsigned short MergedSecIndex;/* the merged section index */
    uint64_t SubSecOffset; /* the offset of the subsection in the merged section*/

    unsigned short SecIndex; /* start from 0, 1, ...*/
    std::string SecComment; /* the comment to be printed */
    std::string SymName;

    std::string SecName; /* such as '$bss', '$data', '$rodata'*/
    unsigned char Permission; /* such as rw, ro,*/
    unsigned int SecSize; /* such as 32, 64, 111,*/
    std::string SecBuf; /* such as 0 for some integer, or "hello world!\x00" for some string*/
    // SmallVector<const unsigned char, 16> SecBuf;
    unsigned char BufType; /* such as int, char, string ...*/
    short PtrSecIndex; /* the section index pointer to: -1-> not ptr */
    SmallVector<XVMGVPatchInfo, 8> PatchListInfo;
} XVMSectionInfo;

}

extern unsigned int GetPtrRegisterLevelBasedOnName(const char * _name);
extern uint64_t GetSubSecOffsetForGlobal(const char * _name);
extern int GetFuncIndex(const char * _name);

#endif /* LLVM_LIB_TARGET_XVM_XVM_DEF_H */
