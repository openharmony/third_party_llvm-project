//===-- XVMCFGSort.cpp - CFG Sorting ------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements a CFG sorting pass.
///
/// This pass reorders the blocks in a function to put them into topological
/// order, ignoring loop backedges, and without any loop or exception being
/// interrupted by a block not dominated by the its header, with special care
/// to keep the order as similar as possible to the original order.
///
////===----------------------------------------------------------------------===//
#ifdef XVM_DYLIB_MODE

#include "MCTargetDesc/XVMMCTargetDesc.h"
#include "XVM.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "XVMSortRegion.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "XVMSubtarget.h"
#include "llvm/ADT/PriorityQueue.h"

#define DEBUG_TYPE "xvm-cfg-sort"
#define INIT_SMALL_VECTOR_PREDS_SIZE 16
#define INIT_SMALL_VECTOR_ENTRIES_SIZE 4

using namespace llvm;
using XVM::SortRegion;
using XVM::SortRegionInfo;

namespace {

class XVMCFGSort final : public MachineFunctionPass {
  StringRef getPassName() const override { return "XVM CFG Sort"; }

  void getAnalysisUsage(AnalysisUsage &AUsage) const override
  {
    AUsage.setPreservesCFG();
    AUsage.addRequired<MachineDominatorTree>();
    AUsage.addPreserved<MachineDominatorTree>();
    AUsage.addRequired<MachineLoopInfo>();
    AUsage.addPreserved<MachineLoopInfo>();
    MachineFunctionPass::getAnalysisUsage(AUsage);
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

public:
  static char ID; // Pass identification, replacement for typeid
  XVMCFGSort() : MachineFunctionPass(ID) {}
};
} // end anonymous namespace

char XVMCFGSort::ID = 0;
INITIALIZE_PASS(XVMCFGSort, DEBUG_TYPE,
                "Reorders blocks in topological order", false, false)

FunctionPass *llvm::createXVMCFGSort() {
  return new XVMCFGSort();
}

static void maybeUpdateTerminator(MachineBasicBlock *MBBlock) {
#ifndef NDEBUG
  bool Barrier = false;
#endif
  bool Analyzable = true;
  for (const MachineInstr &TermIter : MBBlock->terminators()) {
#ifndef NDEBUG
    Barrier |= TermIter.isBarrier();
#endif
    Analyzable &= TermIter.isBranch() && !TermIter.isIndirectBranch();
  }
  assert((Barrier || Analyzable) && "all fallthrough blocks are processed by analyzeBranch");

  // Using the original block order find the layotu successor
  MachineFunction *MFunction = MBBlock->getParent();
  MachineBasicBlock *OriginalNext =
      unsigned(MBBlock->getNumber() + 1) < MFunction->getNumBlockIDs()
          ? MFunction->getBlockNumbered(MBBlock->getNumber() + 1) : nullptr;

  if (Analyzable)
    MBBlock->updateTerminator(OriginalNext);
}

/// Reverse Block sorting based on their number.
struct CompareBlockNumbersBackwards {
  bool operator()(const MachineBasicBlock *First,
                  const MachineBasicBlock *Second) const {
    return First->getNumber() < Second->getNumber();
  }
};

namespace {
/// Block sorting based on their number.
struct CompareBlockNumbers {
  bool operator()(const MachineBasicBlock *First,
                  const MachineBasicBlock *Second) const {
    return First->getNumber() > Second->getNumber();
  }
};

// So we don't mix blocks not dominated by its header
// amond its blocks we use bookkeeping
struct Entry {
  const SortRegion *Region;
  unsigned BlocksLeftCount;

  // A List of blocks not dominated by Loop's header that
  // are deferred until after all of the Loop's blocks have been seen.
  std::vector<MachineBasicBlock *> Def;

  explicit Entry(const SortRegion *SR) : Region(SR), BlocksLeftCount(SR->getNumBlocks()) {}
};
} // anonymous namespace end

// Making sure that regions are not inerrupted by blocks not
// dominated by their header sort the blocks
/// Note: There are many opportunities for improving the heuristics here.
/// Explore them.
static void sortBlocks(MachineFunction &MFunction, const MachineLoopInfo &MLInfo,
                       const MachineDominatorTree &MDTree) {
  // So we can update terminators after reordering to point to the
  // original layout successor, remember original layout
  MFunction.RenumberBlocks();

  // Record the number of predecessors each block has, ignoring loop backedges,
  // to prepare for a topological sort
  SmallVector<unsigned, INIT_SMALL_VECTOR_PREDS_SIZE> NumPredsLeft(MFunction.getNumBlockIDs(), 0);
  for (MachineBasicBlock &MBBlock : MFunction) {
    unsigned Size = MBBlock.pred_size();
    if (MachineLoop *Loop = MLInfo.getLoopFor(&MBBlock))
      if (Loop->getHeader() == &MBBlock)
        for (const MachineBasicBlock *Prev : MBBlock.predecessors())
          if (Loop->contains(Prev))
            --Size;
    NumPredsLeft[MBBlock.getNumber()] = Size;
  }
  // Topological sort, but between a region header and the last block in
  // the region, there can be no block not dominated by its header.
  // AS well its desirable to preserve the original block order of possible
  // Perfer has revenetly processed successors to help keep block order
  // BlockReady has the remaining blocks
  PriorityQueue<MachineBasicBlock *, std::vector<MachineBasicBlock *>, CompareBlockNumbers>
      Prefer;
  PriorityQueue<MachineBasicBlock *, std::vector<MachineBasicBlock *>, CompareBlockNumbersBackwards>
      BlockReady;

  SortRegionInfo SRI(MLInfo);
  SmallVector<Entry, INIT_SMALL_VECTOR_ENTRIES_SIZE> SEntries;
  for (MachineBasicBlock *MBBlock = &MFunction.front();;) {
    const SortRegion *SR = SRI.getRegionFor(MBBlock);
    if (SR) {
      // Can't put any blocks that it doesnt dominate until we see the end
      // of the region if MBBlock is a header
      if (SR->getHeader() == MBBlock)
        SEntries.push_back(Entry(SR));
      // Decrement the count for each active region the block is a part of
      // Take any MBBlock that is the last block in an active region off the list
      // picking up any blocks deferred, since the header didnt dominate them.
      for (Entry &Ent : SEntries)
        if (Ent.Region->contains(MBBlock) && --Ent.BlocksLeftCount == 0)
          for (auto DBlock : Ent.Def)
            BlockReady.push(DBlock);
      while (!SEntries.empty() && SEntries.back().BlocksLeftCount == 0)
        SEntries.pop_back();
    }
    for (MachineBasicBlock *Next : MBBlock->successors()) { // Topological sort logic.
      if (MachineLoop *NextL = MLInfo.getLoopFor(Next)) // Backedges can be ignored.
        if (NextL->getHeader() == Next && NextL->contains(MBBlock))
          continue;
      // Decrement how many are left. It's ready if its now 0.
      if (--NumPredsLeft[Next->getNumber()] == 0) {
        // We allow soritng of BBs that belong to the current region, and also
        // BB's that are dominated bu the current region, when we are in a SortRegion
        Prefer.push(Next);
      }
    }
    // To find the block to follow MBBlock, try to find a preferred block to
    // save the orinal block order if permitted
    MachineBasicBlock *Successor = nullptr;
    while (!Prefer.empty()) {
      Successor = Prefer.top();
      Prefer.pop();
      // until the region is done, keep defering X until it is dominated
      // by the top active region
      if (!SEntries.empty() &&
          !MDTree.dominates(SEntries.back().Region->getHeader(), Successor)) {
        SEntries.back().Def.push_back(Successor);
        Successor = nullptr;
        continue;
      }
      // Succcessor is not preferred ig it was originally ordered before MBBlock
      // and that wasnt caused by it being loop-rotated above the header
      if (Successor->getNumber() < MBBlock->getNumber() &&
          (!SR || !SR->contains(Successor) ||
          SR->getHeader()->getNumber() < Successor->getNumber())) {
        BlockReady.push(Successor);
        Successor = nullptr;
        continue;
      }
      break;
    }
    // Check the general Ready list, if we did't find suitable block in
    // in the prefer list
    if (!Successor) {
      // We are done if ther are no more blocks to process
      if (BlockReady.empty()) {
        maybeUpdateTerminator(MBBlock);
        break;
      }
      for (;;) {
        Successor = BlockReady.top();
        BlockReady.pop();
        // Unitl the region is done, keep defering Successor if it
        // isnt dominated by the top active region header
        if (!SEntries.empty() &&
            !MDTree.dominates(SEntries.back().Region->getHeader(), Successor)) {
          SEntries.back().Def.push_back(Successor);
          continue;
        }
        break;
      }
    }
    // Iterate and move the Successor block into place.
    Successor->moveAfter(MBBlock);
    maybeUpdateTerminator(MBBlock);
    MBBlock = Successor;
  }
  assert(SEntries.empty() && "Have to finish Active sort region");
  MFunction.RenumberBlocks();

#ifndef NDEBUG
  SmallSetVector<const SortRegion *, 8> OnS;

  // Representing the degenerate loop that starts at the function entry,
  // and includes the entire function as a loop that executes one,
  // insert a sentinel
  OnS.insert(nullptr);

  for (auto &MBBlock : MFunction) {
    assert(MBBlock.getNumber() >= 0 && "Renumbered block is not non-negative.");
    const SortRegion *SRegion = SRI.getRegionFor(&MBBlock);

    if (SRegion && &MBBlock == SRegion->getHeader()) {  // Region header.
      if (SRegion->isLoop()) {
        // Loop header, one predecossor should be backedges bellow,
        // and the other should have been processed above
        for (auto Prev : MBBlock.predecessors())
          assert(
              (Prev->getNumber() < MBBlock.getNumber() || SRegion->contains(Prev)) &&
               "Loop header predecessors have to be loop backedges or "
               "predecessors");
      } else {
        // All predecessors should have all been sorted above. Exception header.
        for (auto Prev : MBBlock.predecessors())
          assert(Prev->getNumber() < MBBlock.getNumber() &&
                 "Non-loop-header predecessors have to be topologically sorted");
      }
      assert(OnS.insert(SRegion) &&
             "Can't declare regions more than once.");
    } else {
      // Predecessors should have all beeen sorted above. Not region header.
      for (auto Prev : MBBlock.predecessors())
        assert(Prev->getNumber() < MBBlock.getNumber() &&
               "Non-loop-header predecessors are not topologically sorted");
      assert(OnS.count(SRI.getRegionFor(&MBBlock)) &&
             "Blocks are not nested in their regions");
    }
    while (OnS.size() > 1 && &MBBlock == SRI.getBottom(OnS.back()))
      OnS.pop_back();
  }
  assert(OnS.pop_back_val() == nullptr &&
         "A region header can't be the function entry block");
  assert(OnS.empty() &&
         "Control flow stack pops and stack pushes should be balanced.");
#endif
}

bool XVMCFGSort::runOnMachineFunction(MachineFunction &MF) {
  LLVM_DEBUG(dbgs() << "********** CFG Sorting **********\n"
                       "********** Function: "
                    << MF.getName() << '\n');

  const auto &MLI = getAnalysis<MachineLoopInfo>();
  auto &MDT = getAnalysis<MachineDominatorTree>();

  MF.getRegInfo().invalidateLiveness();

  // Sort the blocks, with contiguous sort regions.
  sortBlocks(MF, MLI, MDT);

  return true;
}

#endif
