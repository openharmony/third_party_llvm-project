//===-- XVMCFGStackify.cpp - CFG Stackification -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements a CFG stacking pass.
///
/// This pass inserts BLOCK, LOOP, and TRY markers to mark the start of scopes,
/// since scope boundaries serve as the labels for XVM's control
/// transfers.
///
/// This is sufficient to convert arbitrary CFGs into a form that works on
/// XVM, provided that all loops are single-entry.
///
/// In case we use exceptions, this pass also fixes mismatches in unwind
/// destinations created during transforming CFG into xvm structured format.
///
//===----------------------------------------------------------------------===//
#ifdef XVM_DYLIB_MODE


#include "XVM.h"
//#include "XVMMachineFunctionInfo.h"
#include "XVMSortRegion.h"
#include "XVMSubtarget.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachinePostDominators.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/Target/TargetMachine.h"
using namespace llvm;
using XVM::SortRegionInfo;

#define DEBUG_TYPE "xvm-cfg-stackify"

STATISTIC(NumCallUnwindMismatches, "Number of call unwind mismatches found");
STATISTIC(NumCatchUnwindMismatches, "Number of catch unwind mismatches found");

namespace {
class XVMCFGStackify final : public MachineFunctionPass {
  StringRef getPassName() const override { return "XVM CFG Stackify"; }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineDominatorTree>();
    AU.addRequired<MachineLoopInfo>();
    AU.addRequired<MachinePostDominatorTree>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

  // For each block whose label represents the end of a scope, record the block
  // which holds the beginning of the scope. This will allow us to quickly skip
  // over scoped regions when walking blocks.
  SmallVector<MachineBasicBlock *, 8> ScopeTops;
  void updateScopeTops(MachineBasicBlock *Begin, MachineBasicBlock *End) {
    int EndNo = End->getNumber();
    if (!ScopeTops[EndNo] || ScopeTops[EndNo]->getNumber() > Begin->getNumber())
      ScopeTops[EndNo] = Begin;
  }

  // Placing markers.
  void placeMarkers(MachineFunction &MF);
  void placeBlockMarker(MachineBasicBlock &MBB);
  void placeLoopMarker(MachineBasicBlock &MBB);
  const XVMInstrInfo *TII = nullptr;
  void extendCondStmt(std::map<MachineInstr *, unsigned int> CondBranchsWithDepth,
                      MachineFunction &MF);
  void insertWithBreak(MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI);
  void insertRetBlock(MachineFunction &MF);
  void removeInFunctionRet(MachineFunction &MF);

  // Wrap-up
  using EndMarkerInfo =
      std::pair<const MachineBasicBlock *, const MachineInstr *>;
  unsigned getBranchDepth(const SmallVectorImpl<EndMarkerInfo> &Stack,
                          const std::set<const MachineBasicBlock*> *SetEndBlockLoop,
                          const MachineBasicBlock *MBB);
  void rewriteDepthImmediates(MachineFunction &MF);
  void fixBackEdgesOfLoops(MachineFunction &MF);
  void fixEndsAtEndOfFunction(MachineFunction &MF);
  void cleanupFunctionData(MachineFunction &MF);

  // For each BLOCK|LOOP|TRY, the corresponding END_(BLOCK|LOOP|TRY) or DELEGATE
  // (in case of TRY).
  DenseMap<const MachineInstr *, MachineInstr *> BeginToEnd;
  // For each END_(BLOCK|LOOP|TRY) or DELEGATE, the corresponding
  // BLOCK|LOOP|TRY.
  DenseMap<const MachineInstr *, MachineInstr *> EndToBegin;

  // We need an appendix block to place 'end_loop' or 'end_try' marker when the
  // loop / exception bottom block is the last block in a function
  MachineBasicBlock *AppendixBB = nullptr;
  MachineBasicBlock *getAppendixBlock(MachineFunction &MF) {
    AppendixBB = nullptr;
    if (!AppendixBB) {
      AppendixBB = MF.CreateMachineBasicBlock();
      // Give it a fake predecessor so that AsmPrinter prints its label.
      AppendixBB->addSuccessor(AppendixBB);
      MF.push_back(AppendixBB);
    }
    return AppendixBB;
  }

  // Before running rewriteDepthImmediates function, 'delegate' has a BB as its
  // destination operand. getFakeCallerBlock() returns a fake BB that will be
  // used for the operand when 'delegate' needs to rethrow to the caller. This
  // will be rewritten as an immediate value that is the number of block depths
  // + 1 in rewriteDepthImmediates, and this fake BB will be removed at the end
  // of the pass.
  MachineBasicBlock *FakeCallerBB = nullptr;
  MachineBasicBlock *getFakeCallerBlock(MachineFunction &MF) {
    if (!FakeCallerBB)
      FakeCallerBB = MF.CreateMachineBasicBlock();
    return FakeCallerBB;
  }

  // Helper functions to register / unregister scope information created by
  // marker instructions.
  void registerScope(MachineInstr *Begin, MachineInstr *End);
  void registerTryScope(MachineInstr *Begin, MachineInstr *End,
                        MachineBasicBlock *EHPad);
  void unregisterScope(MachineInstr *Begin);

public:
  static char ID; // Pass identification, replacement for typeid
  XVMCFGStackify() : MachineFunctionPass(ID) {}
  ~XVMCFGStackify() override { releaseMemory(); }
  void releaseMemory() override;
};
} // end anonymous namespace

char XVMCFGStackify::ID = 0;
INITIALIZE_PASS(XVMCFGStackify, DEBUG_TYPE,
                "Insert BLOCK/LOOP markers for XVM scopes", false,
                false)

FunctionPass *llvm::createXVMCFGStackify() {
  return new XVMCFGStackify();
}

/// Test whether Pred has any terminators explicitly branching to MBB, as
/// opposed to falling through. Note that it's possible (eg. in unoptimized
/// code) for a branch instruction to both branch to a block and fallthrough
/// to it, so we check the actual branch operands to see if there are any
/// explicit mentions.
static bool explicitlyBranchesTo(MachineBasicBlock *Pred,
                                 MachineBasicBlock *MBB) {
  for (MachineInstr &MI : Pred->terminators())
    for (MachineOperand &MO : MI.explicit_operands())
      if (MO.isMBB() && MO.getMBB() == MBB)
        return true;
  return false;
}

// Returns an iterator to the earliest position possible within the MBB,
// satisfying the restrictions given by BeforeSet and AfterSet. BeforeSet
// contains instructions that should go before the marker, and AfterSet contains
// ones that should go after the marker. In this function, AfterSet is only
// used for validation checking.
template <typename Container>
static MachineBasicBlock::iterator
getEarliestInsertPos(MachineBasicBlock *MBB, const Container &BeforeSet,
                     const Container &AfterSet) {
  auto InsertPos = MBB->end();
  while (InsertPos != MBB->begin()) {
    if (BeforeSet.count(&*std::prev(InsertPos))) {
#ifndef NDEBUG
      // Validation check
      for (auto Pos = InsertPos, E = MBB->begin(); Pos != E; --Pos)
        assert(!AfterSet.count(&*std::prev(Pos)));
#endif
      break;
    }
    --InsertPos;
  }
  return InsertPos;
}

// Returns an iterator to the latest position possible within the MBB,
// satisfying the restrictions given by BeforeSet and AfterSet. BeforeSet
// contains instructions that should go before the marker, and AfterSet contains
// ones that should go after the marker. In this function, BeforeSet is only
// used for validation checking.
template <typename Container>
static MachineBasicBlock::iterator
getLatestInsertPos(MachineBasicBlock *MBB, const Container &BeforeSet,
                   const Container &AfterSet) {
  auto InsertPos = MBB->begin();
  while (InsertPos != MBB->end()) {
    if (AfterSet.count(&*InsertPos)) {
#ifndef NDEBUG
      // Validation check
      for (auto Pos = InsertPos, E = MBB->end(); Pos != E; ++Pos)
        assert(!BeforeSet.count(&*Pos));
#endif
      break;
    }
    ++InsertPos;
  }
  return InsertPos;
}

void ChangeBranchCondOpc(MachineInstr &MI, const XVMInstrInfo *TII) {
  switch (MI.getOpcode()) {
    case XVM::BUEQ_rr  : MI.setDesc(TII->get(XVM::LOOP_BUEQ_rr)); break;
    case XVM::BSNEQ_rr : MI.setDesc(TII->get(XVM::LOOP_BSNEQ_rr)); break;
    case XVM::BSGE_rr  : MI.setDesc(TII->get(XVM::LOOP_BSGE_rr)); break;
    case XVM::BUGE_rr : MI.setDesc(TII->get(XVM::LOOP_BUGE_rr)); break;
    case XVM::BSLE_rr  : MI.setDesc(TII->get(XVM::LOOP_BSLE_rr)); break;
    case XVM::BULE_rr : MI.setDesc(TII->get(XVM::LOOP_BULE_rr)); break;
    case XVM::BSGT_rr  : MI.setDesc(TII->get(XVM::LOOP_BSGT_rr)); break;
    case XVM::BUGT_rr : MI.setDesc(TII->get(XVM::LOOP_BUGT_rr)); break;
    case XVM::BSLT_rr  : MI.setDesc(TII->get(XVM::LOOP_BSLT_rr)); break;
    case XVM::BULT_rr : MI.setDesc(TII->get(XVM::LOOP_BULT_rr)); break;
    case XVM::BSEQ_ri  : MI.setDesc(TII->get(XVM::LOOP_BSEQ_ri)); break;
    case XVM::BUEQ_ri : MI.setDesc(TII->get(XVM::LOOP_BUEQ_ri)); break;
    case XVM::BSNEQ_ri : MI.setDesc(TII->get(XVM::LOOP_BSNEQ_ri)); break;
    case XVM::BUNEQ_ri: MI.setDesc(TII->get(XVM::LOOP_BUNEQ_ri)); break;
    case XVM::BSGE_ri  : MI.setDesc(TII->get(XVM::LOOP_BSGE_ri)); break;
    case XVM::BUGE_ri : MI.setDesc(TII->get(XVM::LOOP_BUGE_ri)); break;
    case XVM::BSLE_ri  : MI.setDesc(TII->get(XVM::LOOP_BSLE_ri)); break;
    case XVM::BULE_ri : MI.setDesc(TII->get(XVM::LOOP_BULE_ri)); break;
    case XVM::BSGT_ri  : MI.setDesc(TII->get(XVM::LOOP_BSGT_ri)); break;
    case XVM::BUGT_ri : MI.setDesc(TII->get(XVM::LOOP_BUGT_ri)); break;
    case XVM::BSLT_ri  : MI.setDesc(TII->get(XVM::LOOP_BSLT_ri)); break;
    case XVM::BULT_ri : MI.setDesc(TII->get(XVM::LOOP_BULT_ri)); break;
    default: return;
  }
}

void XVMCFGStackify::fixBackEdgesOfLoops(MachineFunction &MF) {
  TII = MF.getSubtarget<XVMSubtarget>().getInstrInfo();
  auto &MLI = getAnalysis<MachineLoopInfo>();
  for (auto &MBB : MF) {
    MachineLoop *Loop = MLI.getLoopFor(&MBB);
    if (!Loop)
      continue;

    MachineBasicBlock *LoopHeader = Loop->getHeader();
    for (auto &MI : MBB.terminators()) {
      /* skip the added opcode such as THEN ... */
      if (MI.getNumOperands() <=0 ) {
        continue;
      }
      if (TII->isUnCondBranch(&MI) && MI.getOperand(0).getMBB() == LoopHeader) {
        if (&MBB != Loop->getBottomBlock()) {
          BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(XVM::CONTINUE));
        }
        MI.eraseFromParent();
        break;
      } else if (TII->isCondBranch(&MI) && MI.getOperand(0).getMBB() == LoopHeader) {
          uint32_t action_opcode = XVM::CONTINUE;
          /* Fix Loop Exiting Fallthrough */
          if(&MBB == Loop->getBottomBlock() && &MI == &*(--MBB.end()) && MLI.getLoopFor(MBB.getFallThrough()) != Loop){
            TII->negateCondBranch(&MI);
            action_opcode = XVM::BREAK;
          }
          MachineInstr *MIThen = MBB.getParent()->CreateMachineInstr(TII->get(XVM::THEN), DebugLoc());
          MBB.insertAfter(MI.getIterator(), MIThen);
          MachineInstr *MIAction = MBB.getParent()->CreateMachineInstr(TII->get(action_opcode), DebugLoc());
          MBB.insertAfter(MIThen->getIterator(), MIAction);
          MachineInstr *MIEndThen = MBB.getParent()->CreateMachineInstr(TII->get(XVM::END_THEN), DebugLoc());
          MBB.insertAfter(MIAction->getIterator(), MIEndThen);
          MachineInstr *MIEndIf = MBB.getParent()->CreateMachineInstr(TII->get(XVM::END_IF), DebugLoc());
          MBB.insertAfter(MIEndThen->getIterator(), MIEndIf);
          ChangeBranchCondOpc(MI, TII);
      }
    }
  }
}

void XVMCFGStackify::registerScope(MachineInstr *Begin,
                                           MachineInstr *End) {
  BeginToEnd[Begin] = End;
  EndToBegin[End] = Begin;
}

void XVMCFGStackify::unregisterScope(MachineInstr *Begin) {
  assert(BeginToEnd.count(Begin));
  MachineInstr *End = BeginToEnd[Begin];
  assert(EndToBegin.count(End));
  BeginToEnd.erase(Begin);
  EndToBegin.erase(End);
}

static bool isChild(const MachineInstr &MI) {
  if (MI.getNumOperands() == 0)
    return false;
  const MachineOperand &MO = MI.getOperand(0);
  if (!MO.isReg() || MO.isImplicit() || !MO.isDef())
    return false;
  Register Reg = MO.getReg();
  return Register::isVirtualRegister(Reg);
}

/** if END_BLOCK is followed with END_LOOP in the same BB, and both of them
 *  are in the beginning of the BB, the break_imm needs to increase one.
 *  This is the case where there are multiple conditions ||ed or &&ed in a
 *  loop condition such as for or while loops.
 *  FIXME: we may find other approach for fixing this.
*/
unsigned XVMCFGStackify::getBranchDepth(
    const SmallVectorImpl<EndMarkerInfo> &Stack,
    const std::set<const MachineBasicBlock*> *SetEndBlockLoop,
    const MachineBasicBlock *MBB) {
  unsigned Depth = 0;
  for (auto X : reverse(Stack)) {
    if (X.first == MBB) {
      std::set<const MachineBasicBlock*>::iterator I = SetEndBlockLoop->find(MBB);
      if (I != SetEndBlockLoop->end())
        ++Depth;
      break;
    }
    ++Depth;
  }
  assert(Depth < Stack.size() && "Branch destination should be in scope");
  return Depth;
}

static bool isContinueBlock(MachineBasicBlock &MBB) {
  for (auto &MI : MBB) {
    if (MI.getOpcode() == XVM::CONTINUE)
      return true;
  }
  return false;
}

static inline MachineBasicBlock *getNextBlock(MachineBasicBlock &MBB) {
  MachineFunction::iterator I = MBB.getIterator();
  MachineFunction::iterator E = MBB.getParent()->end();
  if (++I == E)
    return nullptr;
  return &*I;
}

static inline MachineInstr *getNextInstruction(MachineInstr &MI) {
  MachineBasicBlock::iterator I = MI.getIterator();
  MachineBasicBlock::iterator E = MI.getParent()->end();
  if (++I == E)
    return nullptr;
  return &*I;
}

void XVMCFGStackify::insertWithBreak(MachineBasicBlock &MBB,
                                     MachineBasicBlock::iterator MBBI) {
  DebugLoc DL;
  MachineInstr &Inst = *MBBI;

  MachineBasicBlock *EndBlock = &MBB.getParent()->back();
  const auto &TII = MBB.getParent()->getSubtarget<XVMSubtarget>().getInstrInfo();
  MBB.addSuccessor(EndBlock);
  MachineInstr *BreakInstr = BuildMI(*MBB.getParent(), DL, TII->get(XVM::BR)).addMBB(EndBlock);
  MBB.insertAfter(MBBI, BreakInstr);
}

void XVMCFGStackify::insertRetBlock(MachineFunction &MF) {
  const auto &TII = *MF.getSubtarget<XVMSubtarget>().getInstrInfo();
  MachineBasicBlock *BeginBlock = nullptr;
  MachineBasicBlock *EndBlock = nullptr;
  DebugLoc DL;

  BeginBlock = MF.CreateMachineBasicBlock();
  MachineInstr *Begin =
  BuildMI(MF, DL, TII.get(XVM::BLOCK));
  BeginBlock->push_back(Begin);
  MF.push_front(BeginBlock);

  EndBlock = MF.CreateMachineBasicBlock();
  MachineInstr *End = BuildMI(MF, DL, TII.get(XVM::END_BLOCK));
  EndBlock->push_back(End);
  MachineInstr *Ret = BuildMI(MF, DL, TII.get(XVM::RETURN));
  EndBlock->push_back(Ret);
  MF.push_back(EndBlock);

  ScopeTops.resize(MF.getNumBlockIDs() + 1);
  updateScopeTops(BeginBlock, EndBlock);
}

void XVMCFGStackify::removeInFunctionRet(MachineFunction &MF) {
  bool retSeen = false;
  bool retExists = false;
  for (auto &MBB: MF) {
    for (auto MBBI = MBB.begin(); MBBI != MBB.end(); ++MBBI) {
      MachineInstr &Inst = *MBBI;
      if (Inst.getOpcode() == XVM::RETURN) {
        retExists = true;
        if (getNextBlock(MBB) == nullptr) {
          // This is the last block
          if (getNextInstruction(Inst) != nullptr) {
            // This is NOT the last instruction
            insertRetBlock(MF);
            insertWithBreak(MBB, MBBI);
            MBBI = MBB.erase(MBBI);
          }
        } else {
          // This is not the last block
          if (retSeen) {
            insertWithBreak(MBB, MBBI);
            MBBI = MBB.erase(MBBI);
          } else {
            insertRetBlock(MF);
            insertWithBreak(MBB, MBBI);
            MBBI = MBB.erase(MBBI);
            retSeen = true;
          }
        }
      }
    }
  }
  if (!retExists) {
    const auto &TII = *MF.getSubtarget<XVMSubtarget>().getInstrInfo();
    DebugLoc DL;
    MachineBasicBlock &EndBlock = MF.back();
    MachineInstr *ret = BuildMI(MF, DL, TII.get(XVM::RETURN));
    EndBlock.push_back(ret);
  }
}

/// Insert LOOP/BLOCK markers at appropriate places.
void XVMCFGStackify::placeMarkers(MachineFunction &MF) {
  // We allocate one more than the number of blocks in the function to
  // accommodate for the possible fake block we may insert at the end.
  ScopeTops.resize(MF.getNumBlockIDs() + 1);
  // Place the LOOP for MBB if MBB is the header of a loop.
  for (auto &MBB : MF)
    placeLoopMarker(MBB);

  const MCAsmInfo *MCAI = MF.getTarget().getMCAsmInfo();
  for (auto &MBB : MF) {
    // Place the BLOCK for MBB if MBB is branched to from above.
    placeBlockMarker(MBB);
  }

  removeInFunctionRet(MF);
}

/// Insert a BLOCK marker for branches to MBB (if needed).
// TODO Consider a more generalized way of handling block (and also loop and
// try) signatures when we implement the multi-value proposal later.
void XVMCFGStackify::placeBlockMarker(MachineBasicBlock &MBB) {
  assert(!MBB.isEHPad());
  MachineFunction &MF = *MBB.getParent();
  auto &MDT = getAnalysis<MachineDominatorTree>();
  const auto &TII = *MF.getSubtarget<XVMSubtarget>().getInstrInfo();

  // First compute the nearest common dominator of all forward non-fallthrough
  // predecessors so that we minimize the time that the BLOCK is on the stack,
  // which reduces overall stack height.
  MachineBasicBlock *Header = nullptr;
  bool IsBranchedTo = false;
  int MBBNumber = MBB.getNumber();
  for (MachineBasicBlock *Pred : MBB.predecessors()) {
    if (Pred->getNumber() < MBBNumber) {
      Header = Header ? MDT.findNearestCommonDominator(Header, Pred) : Pred;
      if (explicitlyBranchesTo(Pred, &MBB))
        IsBranchedTo = true;
    }
  }
  if (!Header)
    return;
  if (!IsBranchedTo)
    return;

  assert(&MBB != &MF.front() && "Header blocks shouldn't have predecessors");
  MachineBasicBlock *LayoutPred = MBB.getPrevNode();

  // If the nearest common dominator is inside a more deeply nested context,
  // walk out to the nearest scope which isn't more deeply nested.
  for (MachineFunction::iterator I(LayoutPred), E(Header); I != E; --I) {
    if (MachineBasicBlock *ScopeTop = ScopeTops[I->getNumber()]) {
      if (ScopeTop->getNumber() > Header->getNumber()) {
        // Skip over an intervening scope.
        I = std::next(ScopeTop->getIterator());
      } else {
        // We found a scope level at an appropriate depth.
        Header = ScopeTop;
        break;
      }
    }
  }

  // Decide where in Header to put the BLOCK.

  // Instructions that should go before the BLOCK.
  SmallPtrSet<const MachineInstr *, 4> BeforeSet;
  // Instructions that should go after the BLOCK.
  SmallPtrSet<const MachineInstr *, 4> AfterSet;
  for (const auto &MI : *Header) {
    // If there is a previously placed LOOP marker and the bottom block of the
    // loop is above MBB, it should be after the BLOCK, because the loop is
    // nested in this BLOCK. Otherwise it should be before the BLOCK.
    if (MI.getOpcode() == XVM::LOOP) {
      auto *LoopBottom = BeginToEnd[&MI]->getParent()->getPrevNode();
      if (MBB.getNumber() > LoopBottom->getNumber())
        AfterSet.insert(&MI);
#ifndef NDEBUG
      else
        BeforeSet.insert(&MI);
#endif
    }

    // If there is a previously placed BLOCK/TRY marker and its corresponding
    // END marker is before the current BLOCK's END marker, that should be
    // placed after this BLOCK. Otherwise it should be placed before this BLOCK
    // marker.
    if (MI.getOpcode() == XVM::BLOCK) {
      if (BeginToEnd[&MI]->getParent()->getNumber() <= MBB.getNumber())
        AfterSet.insert(&MI);
#ifndef NDEBUG
      else
        BeforeSet.insert(&MI);
#endif
    }

#ifndef NDEBUG
    // All END_(BLOCK|LOOP|TRY) markers should be before the BLOCK.
    if (MI.getOpcode() == XVM::END_BLOCK ||
        MI.getOpcode() == XVM::END_LOOP)
      BeforeSet.insert(&MI);
#endif

    // Terminators should go after the BLOCK.
    if (MI.isTerminator())
      AfterSet.insert(&MI);
  }

  // Local expression tree should go after the BLOCK.
  for (auto I = Header->getFirstTerminator(), E = Header->begin(); I != E;
       --I) {
    if (std::prev(I)->isDebugInstr() || std::prev(I)->isPosition())
      continue;
    if (isChild(*std::prev(I)))
      AfterSet.insert(&*std::prev(I));
    else
      break;
  }

  // Add the BLOCK.
//  XVM::BlockType ReturnType = XVM::BlockType::Void;
  auto InsertPos = getLatestInsertPos(Header, BeforeSet, AfterSet);
  MachineInstr *Begin =
      BuildMI(*Header, InsertPos, Header->findDebugLoc(InsertPos),
              TII.get(XVM::BLOCK));
//FIXME: Check if we need it
//          .addImm(int64_t(ReturnType));

  // Decide where in Header to put the END_BLOCK.
  BeforeSet.clear();
  AfterSet.clear();
  for (auto &MI : MBB) {
#ifndef NDEBUG
    // END_BLOCK should precede existing LOOP and TRY markers.
    if (MI.getOpcode() == XVM::LOOP)
      AfterSet.insert(&MI);
#endif

    // If there is a previously placed END_LOOP marker and the header of the
    // loop is above this block's header, the END_LOOP should be placed after
    // the BLOCK, because the loop contains this block. Otherwise the END_LOOP
    // should be placed before the BLOCK. The same for END_TRY.
    if (MI.getOpcode() == XVM::END_LOOP) {
      if (EndToBegin[&MI]->getParent()->getNumber() >= Header->getNumber())
        BeforeSet.insert(&MI);
#ifndef NDEBUG
      else
        AfterSet.insert(&MI);
#endif
    }
  }

  // Mark the end of the block.
  InsertPos = getEarliestInsertPos(&MBB, BeforeSet, AfterSet);
  MachineInstr *End = BuildMI(MBB, InsertPos, MBB.findPrevDebugLoc(InsertPos),
                              TII.get(XVM::END_BLOCK));
  registerScope(Begin, End);

  // Track the farthest-spanning scope that ends at this point.
  updateScopeTops(Header, &MBB);
}

/// Insert a LOOP marker for a loop starting at MBB (if it's a loop header).
void XVMCFGStackify::placeLoopMarker(MachineBasicBlock &MBB) {
  MachineFunction &MF = *MBB.getParent();
  const auto &MLI = getAnalysis<MachineLoopInfo>();
  SortRegionInfo SRI(MLI);
  const auto &TII = *MF.getSubtarget<XVMSubtarget>().getInstrInfo();

  MachineLoop *Loop = MLI.getLoopFor(&MBB);
  if (!Loop || Loop->getHeader() != &MBB)
    return;

  // The operand of a LOOP is the first block after the loop. If the loop is the
  // bottom of the function, insert a dummy block at the end.
  MachineBasicBlock *Bottom = SRI.getBottom(Loop);
  auto Iter = std::next(Bottom->getIterator());
  if (Iter == MF.end()) {
    getAppendixBlock(MF);
    Iter = std::next(Bottom->getIterator());
  }
  MachineBasicBlock *AfterLoop = &*Iter;

  // Decide where in Header to put the LOOP.
  SmallPtrSet<const MachineInstr *, 4> BeforeSet;
  SmallPtrSet<const MachineInstr *, 4> AfterSet;
  for (const auto &MI : MBB) {
    // LOOP marker should be after any existing loop that ends here. Otherwise
    // we assume the instruction belongs to the loop.
    if (MI.getOpcode() == XVM::END_LOOP)
      BeforeSet.insert(&MI);
#ifndef NDEBUG
    else
      AfterSet.insert(&MI);
#endif
  }

  // Mark the beginning of the loop.
  auto InsertPos = getEarliestInsertPos(&MBB, BeforeSet, AfterSet);
  //FIXME: modify the form of the LOOP instruction
  MachineInstr *Begin = BuildMI(MBB, InsertPos, MBB.findDebugLoc(InsertPos),
                                TII.get(XVM::LOOP));
//                            .addImm(int64_t(XVM::BlockType::Void));

  // Decide where in Header to put the END_LOOP.
  BeforeSet.clear();
  AfterSet.clear();
#ifndef NDEBUG
  for (const auto &MI : MBB)
    // Existing END_LOOP markers belong to parent loops of this loop
    if (MI.getOpcode() == XVM::END_LOOP)
      AfterSet.insert(&MI);
#endif

  // Mark the end of the loop (using arbitrary debug location that branched to
  // the loop end as its location).
  InsertPos = getEarliestInsertPos(AfterLoop, BeforeSet, AfterSet);
  DebugLoc EndDL = AfterLoop->pred_empty()
                       ? DebugLoc()
                       : (*AfterLoop->pred_rbegin())->findBranchDebugLoc();
  MachineInstr *End =
      BuildMI(*AfterLoop, InsertPos, EndDL, TII.get(XVM::END_LOOP));
  registerScope(Begin, End);

  assert((!ScopeTops[AfterLoop->getNumber()] ||
          ScopeTops[AfterLoop->getNumber()]->getNumber() < MBB.getNumber()) &&
         "With block sorting the outermost loop for a block should be first.");
  updateScopeTops(&MBB, AfterLoop);
}

void XVMCFGStackify::extendCondStmt(std::map<MachineInstr *, unsigned int> CondBranchsWithDepth,
                   MachineFunction &MF) {
  for (auto& I : CondBranchsWithDepth) {
    MachineInstr *MI = I.first;
    unsigned int depth = I.second;
    bool isDone = false;
    for (auto &MBB : reverse(MF)) {
      for (MachineInstr &EachMI : llvm::reverse(MBB)) {
        if (MI == &EachMI) {
          MachineInstr *MI_THEN = MBB.getParent()->CreateMachineInstr(TII->get(XVM::THEN), DebugLoc());
          MBB.insertAfter(EachMI.getIterator(), MI_THEN);

          MachineInstr *MI_BREAK_IMM = MBB.getParent()->CreateMachineInstr(TII->get(XVM::BREAK_IMM), DebugLoc());
          MBB.insertAfter(MI_THEN->getIterator(), MI_BREAK_IMM);
          MachineInstrBuilder MIB(MF, MI_BREAK_IMM);
          MIB.addImm(depth);

          MachineInstr *MI_END_THEN = MBB.getParent()->CreateMachineInstr(TII->get(XVM::END_THEN), DebugLoc());
          MBB.insertAfter(MI_BREAK_IMM->getIterator(), MI_END_THEN);

          MachineInstr *MI_END_IF = MBB.getParent()->CreateMachineInstr(TII->get(XVM::END_IF), DebugLoc());
          MBB.insertAfter(MI_END_THEN->getIterator(), MI_END_IF);
          isDone = true;
        }
      }
      if (isDone) {
        break;
      }
    }
  }
}

void XVMCFGStackify::rewriteDepthImmediates(MachineFunction &MF) {
  // Now rewrite references to basic blocks to be depth immediates.
  std::map<MachineInstr *, unsigned int> CondBranchsWithDepth;
  TII = MF.getSubtarget<XVMSubtarget>().getInstrInfo();
  SmallVector<EndMarkerInfo, 8> Stack;
  std::set<const MachineBasicBlock*> SetEndBlockLoop;
  SmallVector<const MachineBasicBlock *, 8> EHPadStack;
  for (auto &MBB : reverse(MF)) {
    for (MachineInstr &MI : llvm::reverse(MBB)) {
      switch (MI.getOpcode()) {
      case XVM::BLOCK:
        assert(ScopeTops[Stack.back().first->getNumber()]->getNumber() <=
                   MBB.getNumber() &&
               "Block/try marker should be balanced");
        Stack.pop_back();
        break;

      case XVM::LOOP:
        assert(Stack.back().first == &MBB && "Loop top should be balanced");
        Stack.pop_back();
        break;

      case XVM::END_BLOCK:
        Stack.push_back(std::make_pair(&MBB, &MI));
        break;

      case XVM::END_LOOP: {
        Stack.push_back(std::make_pair(EndToBegin[&MI]->getParent(), &MI));
        MachineInstr * PrevMI = MI.getPrevNode();
        if (PrevMI != NULL && PrevMI == MBB.begin()) {
          if (PrevMI->getOpcode() == XVM::END_BLOCK) {
            SetEndBlockLoop.insert(&MBB);
          }
        }
        break;
      }
      default:
        if (MI.isTerminator()) {
          // Rewrite MBB operands to be depth immediates.
          SmallVector<MachineOperand, 4> Ops(MI.operands());
          unsigned int Opcode = MI.getOpcode();
          unsigned int depth = 0;
          while (MI.getNumOperands() > 0)
            MI.removeOperand(MI.getNumOperands() - 1);
          for (auto MO : Ops) {
            if (MO.isMBB()) {
              depth = getBranchDepth(Stack, &SetEndBlockLoop, MO.getMBB());
              MO = MachineOperand::CreateImm(depth);
            }
            MI.addOperand(MF, MO);
          }
          if (Opcode == XVM::BR) {
            MI.setDesc(TII->get(XVM::BREAK_IMM));
          } else if (TII->isCondBranch(&MI) && !TII->isCondBranchProcessed(&MI)) {
            /** add the following instructions: THEN, BRREAK_IMM and END_THEN */
            CondBranchsWithDepth.insert(std::make_pair(&MI, depth));
          }
        }
        break;
      }
    }
  }
  extendCondStmt(CondBranchsWithDepth, MF);
  assert(Stack.empty() && "Control flow should be balanced");
}

void XVMCFGStackify::cleanupFunctionData(MachineFunction &MF) {
  if (FakeCallerBB)
    MF.deleteMachineBasicBlock(FakeCallerBB);
  AppendixBB = FakeCallerBB = nullptr;
}

void XVMCFGStackify::releaseMemory() {
  ScopeTops.clear();
  BeginToEnd.clear();
  EndToBegin.clear();
}

bool XVMCFGStackify::runOnMachineFunction(MachineFunction &MF) {
  LLVM_DEBUG(dbgs() << "********** CFG Stackifying **********\n"
                       "********** Function: "
                    << MF.getName() << '\n');
  const MCAsmInfo *MCAI = MF.getTarget().getMCAsmInfo();

  releaseMemory();

  // Place the BLOCK/LOOP/TRY markers to indicate the beginnings of scopes.
  placeMarkers(MF);

  // Place the continue statements for each backedge of loops.
  fixBackEdgesOfLoops(MF);

  // Convert MBB operands in terminators to relative depth immediates.
  rewriteDepthImmediates(MF);

  // Add an end instruction at the end of the function body.
  const auto &TII = *MF.getSubtarget<XVMSubtarget>().getInstrInfo();
  if (!MF.getSubtarget<XVMSubtarget>()
           .getTargetTriple()
           .isOSBinFormatELF())
//FIXME: See if we need it
//    appendEndToFunction(MF, TII);

  cleanupFunctionData(MF);

  return true;
}

#endif
