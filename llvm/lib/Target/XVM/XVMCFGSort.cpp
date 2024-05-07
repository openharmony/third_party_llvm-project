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
#include "XVMSortRegion.h"
#include "XVMSubtarget.h"
#include "llvm/ADT/PriorityQueue.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;
using XVM::SortRegion;
using XVM::SortRegionInfo;

#define DEBUG_TYPE "xvm-cfg-sort"

namespace {

class XVMCFGSort final : public MachineFunctionPass {
  StringRef getPassName() const override { return "XVM CFG Sort"; }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addRequired<MachineDominatorTree>();
    AU.addPreserved<MachineDominatorTree>();
    AU.addRequired<MachineLoopInfo>();
    AU.addPreserved<MachineLoopInfo>();
    MachineFunctionPass::getAnalysisUsage(AU);
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

static void maybeUpdateTerminator(MachineBasicBlock *MBB) {
#ifndef NDEBUG
  bool AnyBarrier = false;
#endif
  bool AllAnalyzable = true;
  for (const MachineInstr &Term : MBB->terminators()) {
#ifndef NDEBUG
    AnyBarrier |= Term.isBarrier();
#endif
    AllAnalyzable &= Term.isBranch() && !Term.isIndirectBranch();
  }
  assert((AnyBarrier || AllAnalyzable) &&
         "analyzeBranch needs to analyze any block with a fallthrough");

  // Find the layout successor from the original block order.
  MachineFunction *MF = MBB->getParent();
  MachineBasicBlock *OriginalSuccessor =
      unsigned(MBB->getNumber() + 1) < MF->getNumBlockIDs()
          ? MF->getBlockNumbered(MBB->getNumber() + 1)
          : nullptr;

  if (AllAnalyzable)
    MBB->updateTerminator(OriginalSuccessor);
}

namespace {
/// Sort blocks by their number.
struct CompareBlockNumbers {
  bool operator()(const MachineBasicBlock *A,
                  const MachineBasicBlock *B) const {
    return A->getNumber() > B->getNumber();
  }
};
/// Sort blocks by their number in the opposite order..
struct CompareBlockNumbersBackwards {
  bool operator()(const MachineBasicBlock *A,
                  const MachineBasicBlock *B) const {
    return A->getNumber() < B->getNumber();
  }
};
/// Bookkeeping for a region to help ensure that we don't mix blocks not
/// dominated by the its header among its blocks.
struct Entry {
  const SortRegion *TheRegion;
  unsigned NumBlocksLeft;

  /// List of blocks not dominated by Loop's header that are deferred until
  /// after all of Loop's blocks have been seen.
  std::vector<MachineBasicBlock *> Deferred;

  explicit Entry(const SortRegion *R)
      : TheRegion(R), NumBlocksLeft(R->getNumBlocks()) {}
};
} // end anonymous namespace

/// Sort the blocks, taking special care to make sure that regions are not
/// interrupted by blocks not dominated by their header.
/// TODO: There are many opportunities for improving the heuristics here.
/// Explore them.
static void sortBlocks(MachineFunction &MF, const MachineLoopInfo &MLI,
                       const MachineDominatorTree &MDT) {
  // Remember original layout ordering, so we can update terminators after
  // reordering to point to the original layout successor.
  MF.RenumberBlocks();

  // Prepare for a topological sort: Record the number of predecessors each
  // block has, ignoring loop backedges.
  SmallVector<unsigned, 16> NumPredsLeft(MF.getNumBlockIDs(), 0);
  for (MachineBasicBlock &MBB : MF) {
    unsigned N = MBB.pred_size();
    if (MachineLoop *L = MLI.getLoopFor(&MBB))
      if (L->getHeader() == &MBB)
        for (const MachineBasicBlock *Pred : MBB.predecessors())
          if (L->contains(Pred))
            --N;
    NumPredsLeft[MBB.getNumber()] = N;
  }

  // Topological sort the CFG, with additional constraints:
  //  - Between a region header and the last block in the region, there can be
  //    no blocks not dominated by its header.
  //  - It's desirable to preserve the original block order when possible.
  // We use two ready lists; Preferred and Ready. Preferred has recently
  // processed successors, to help preserve block sequences from the original
  // order. Ready has the remaining ready blocks.
  PriorityQueue<MachineBasicBlock *, std::vector<MachineBasicBlock *>,
                CompareBlockNumbers>
      Preferred;
  PriorityQueue<MachineBasicBlock *, std::vector<MachineBasicBlock *>,
                CompareBlockNumbersBackwards>
      Ready;

  SortRegionInfo SRI(MLI);
  SmallVector<Entry, 4> Entries;
  for (MachineBasicBlock *MBB = &MF.front();;) {
    const SortRegion *R = SRI.getRegionFor(MBB);
    if (R) {
      // If MBB is a region header, add it to the active region list. We can't
      // put any blocks that it doesn't dominate until we see the end of the
      // region.
      if (R->getHeader() == MBB)
        Entries.push_back(Entry(R));
      // For each active region the block is in, decrement the count. If MBB is
      // the last block in an active region, take it off the list and pick up
      // any blocks deferred because the header didn't dominate them.
      for (Entry &E : Entries)
        if (E.TheRegion->contains(MBB) && --E.NumBlocksLeft == 0)
          for (auto DeferredBlock : E.Deferred)
            Ready.push(DeferredBlock);
      while (!Entries.empty() && Entries.back().NumBlocksLeft == 0)
        Entries.pop_back();
    }
    // The main topological sort logic.
    for (MachineBasicBlock *Succ : MBB->successors()) {
      // Ignore backedges.
      if (MachineLoop *SuccL = MLI.getLoopFor(Succ))
        if (SuccL->getHeader() == Succ && SuccL->contains(MBB))
          continue;
      // Decrement the predecessor count. If it's now zero, it's ready.
      if (--NumPredsLeft[Succ->getNumber()] == 0) {
        // When we are in a SortRegion, we allow sorting of not only BBs that
        // belong to the current (innermost) region but also BBs that are
        // dominated by the current region header.
        Preferred.push(Succ);
      }
    }
    // Determine the block to follow MBB. First try to find a preferred block,
    // to preserve the original block order when possible.
    MachineBasicBlock *Next = nullptr;
    while (!Preferred.empty()) {
      Next = Preferred.top();
      Preferred.pop();
      // If X isn't dominated by the top active region header, defer it until
      // that region is done.
      if (!Entries.empty() &&
          !MDT.dominates(Entries.back().TheRegion->getHeader(), Next)) {
        Entries.back().Deferred.push_back(Next);
        Next = nullptr;
        continue;
      }
      // If Next was originally ordered before MBB, and it isn't because it was
      // loop-rotated above the header, it's not preferred.
      if (Next->getNumber() < MBB->getNumber() &&
          (!R || !R->contains(Next) ||
           R->getHeader()->getNumber() < Next->getNumber())) {
        Ready.push(Next);
        Next = nullptr;
        continue;
      }
      break;
    }
    // If we didn't find a suitable block in the Preferred list, check the
    // general Ready list.
    if (!Next) {
      // If there are no more blocks to process, we're done.
      if (Ready.empty()) {
        maybeUpdateTerminator(MBB);
        break;
      }
      for (;;) {
        Next = Ready.top();
        Ready.pop();
        // If Next isn't dominated by the top active region header, defer it
        // until that region is done.
        if (!Entries.empty() &&
            !MDT.dominates(Entries.back().TheRegion->getHeader(), Next)) {
          Entries.back().Deferred.push_back(Next);
          continue;
        }
        break;
      }
    }
    // Move the next block into place and iterate.
    Next->moveAfter(MBB);
    maybeUpdateTerminator(MBB);
    MBB = Next;
  }
  assert(Entries.empty() && "Active sort region list not finished");
  MF.RenumberBlocks();

#ifndef NDEBUG
  SmallSetVector<const SortRegion *, 8> OnStack;

  // Insert a sentinel representing the degenerate loop that starts at the
  // function entry block and includes the entire function as a "loop" that
  // executes once.
  OnStack.insert(nullptr);

  for (auto &MBB : MF) {
    assert(MBB.getNumber() >= 0 && "Renumbered blocks should be non-negative.");
    const SortRegion *Region = SRI.getRegionFor(&MBB);

    if (Region && &MBB == Region->getHeader()) {
      // Region header.
      if (Region->isLoop()) {
        // Loop header. The loop predecessor should be sorted above, and the
        // other predecessors should be backedges below.
        for (auto Pred : MBB.predecessors())
          assert(
              (Pred->getNumber() < MBB.getNumber() || Region->contains(Pred)) &&
              "Loop header predecessors must be loop predecessors or "
              "backedges");
      } else {
        // Exception header. All predecessors should be sorted above.
        for (auto Pred : MBB.predecessors())
          assert(Pred->getNumber() < MBB.getNumber() &&
                 "Non-loop-header predecessors should be topologically sorted");
      }
      assert(OnStack.insert(Region) &&
             "Regions should be declared at most once.");

    } else {
      // Not a region header. All predecessors should be sorted above.
      for (auto Pred : MBB.predecessors())
        assert(Pred->getNumber() < MBB.getNumber() &&
               "Non-loop-header predecessors should be topologically sorted");
      assert(OnStack.count(SRI.getRegionFor(&MBB)) &&
             "Blocks must be nested in their regions");
    }
    while (OnStack.size() > 1 && &MBB == SRI.getBottom(OnStack.back()))
      OnStack.pop_back();
  }
  assert(OnStack.pop_back_val() == nullptr &&
         "The function entry block shouldn't actually be a region header");
  assert(OnStack.empty() &&
         "Control flow stack pushes and pops should be balanced.");
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