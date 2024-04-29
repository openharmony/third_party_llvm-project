#ifndef LLVM_LIB_TARGET_XVM_XVMMACHINEFUNCTIONINFO_H
#define LLVM_LIB_TARGET_XVM_XVMMACHINEFUNCTIONINFO_H

#include "llvm/CodeGen/MachineFunction.h"
 
namespace llvm {

class XVMMachineFunctionInfo : public MachineFunctionInfo {
public:
  XVMMachineFunctionInfo(const MachineFunction &MF) {}
  void SetVarArgsFrameIndex(int _index) { VarArgsFrameIndex = _index; }
  int GetVarArgsFrameIndex() { return VarArgsFrameIndex; }
private:
  int VarArgsFrameIndex;
};


}

#endif