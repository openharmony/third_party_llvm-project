#ifdef XVM_DYLIB_MODE

#include "XVMSortRegion.h"
#include "llvm/CodeGen/MachineLoopInfo.h"

using namespace llvm;
using namespace XVM;

namespace llvm {
namespace XVM {
template <>
bool ConcreteSortRegion<MachineLoop>::isLoop() const {
  return true;
}
} // end namespace XVM
} // end namespace llvm

const SortRegion *SortRegionInfo::getRegionFor(const MachineBasicBlock *MBB) {
  const auto *ML = MLI.getLoopFor(MBB);
  if (!ML)
    return nullptr;
  // We determine subregion relationship by domination of their headers, i.e.,
  // if region A's header dominates region B's header, B is a subregion of A.
  if (ML) {
    // If the smallest region containing MBB is a loop
    if (LoopMap.count(ML))
      return LoopMap[ML].get();
    LoopMap[ML] = std::make_unique<ConcreteSortRegion<MachineLoop>>(ML);
    return LoopMap[ML].get();
  }
  return nullptr;
}

MachineBasicBlock *SortRegionInfo::getBottom(const SortRegion *R) {
  if (R->isLoop())
    return getBottom(MLI.getLoopFor(R->getHeader()));
  return nullptr;
}

MachineBasicBlock *SortRegionInfo::getBottom(const MachineLoop *ML) {
  MachineBasicBlock *Bottom = ML->getHeader();
  for (MachineBasicBlock *MBB : ML->blocks()) {
    if (MBB->getNumber() > Bottom->getNumber())
      Bottom = MBB;
  }
  return Bottom;
}

#endif