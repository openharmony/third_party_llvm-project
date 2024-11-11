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
#define INIT_SMALL_VECTOR_SCOPESS_SIZE 8
#define INIT_SMALL_VECTOR_MO_SIZE 4

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
  SmallVector<MachineBasicBlock *, INIT_SMALL_VECTOR_SCOPESS_SIZE> ScopeTops;
  void updateScopeTops(MachineBasicBlock *Start, MachineBasicBlock *Finish) {
    int FinishNo = Finish->getNumber();
    if (!ScopeTops[FinishNo] || ScopeTops[FinishNo]->getNumber() > Start->getNumber())
      ScopeTops[FinishNo] = Start;
  }

  // Placing markers.
  void placeMarkers(MachineFunction &MFunction);
  void placeBlockMarker(MachineBasicBlock &MBBlock);
  void placeLoopMarker(MachineBasicBlock &MBBlock);
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
  void fixEndsAtEndOfFunction(MachineFunction &MFunction);
  void cleanupFunctionData(MachineFunction &MFunction);

  // The correspinding BLOCK|LOOP|TRY for each END_(BLOCK|LOOP|TRY)
  DenseMap<const MachineInstr *, MachineInstr *> EndToBegin;
  // The corresponding END_(BLOCK|LOOP|TRY) or DELEGATE for each BLOCK|LOOP|TRY
  // (in the TRY case).
  DenseMap<const MachineInstr *, MachineInstr *> BeginToEnd;

  // We need an appendix block to place 'end_loop' or 'end_try' marker when the
  // loop / exception bottom block is the last block in a function
  MachineBasicBlock *AppendixBBlock = nullptr;
  MachineBasicBlock *getAppendixBlock(MachineFunction &MFunction){
    AppendixBBlock = nullptr;
    if (!AppendixBBlock) {
      AppendixBBlock = MFunction.CreateMachineBasicBlock();
      //  For AsmPrinter to prints its label, we give it a fake predecessor
      AppendixBBlock->addSuccessor(AppendixBBlock);
      MFunction.push_back(AppendixBBlock);
    }
    return AppendixBBlock;
  }

  // "delegate" has a BB as its destincation operand, before running
  // rewriteDEpthImmediated. Use a fake BB for the operand when "delegate"
  // needs to rethrow to the caller, returned by getFakeCallerBlock. The
  // number of block depths + 1 will be rewritten as an immediate value in
  // rewriteDepthImmediates, and this fake BB will be removed at the end
  MachineBasicBlock *FakeBBlockCaller = nullptr;
  MachineBasicBlock *getFakeCallerBlock(MachineFunction &MFunction) {
    if (!FakeBBlockCaller)
      FakeBBlockCaller = MFunction.CreateMachineBasicBlock();
    return FakeBBlockCaller;
  }

  //marker instructions created function to register/unregister scope info
  void registerScope(MachineInstr *Start, MachineInstr *Finish);
  void registerTryScope(MachineInstr *Start,
                MachineInstr *Finish, MachineBasicBlock *EHPad);
  void unregisterScope(MachineInstr *Start);

public:
  static char ID; // typeid replacement, for identification
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


// As opposed to falling through, test whether Pre has any terminators
// explicitly branching to MBB. Note: We check to see if therey are any explicit
// mentions of a branch instruction both branching to and falling through
// to a block through the actual branch operands (e.g. unoptimzed code)
static bool explicitlyBranchesTo(MachineBasicBlock *Pre, MachineBasicBlock *MBBlock) {
  bool ret = false;
  for (MachineInstr &MInst : Pre->terminators())
    for (MachineOperand &MOp : MInst.explicit_operands())
      if (MOp.isMBB() && MOp.getMBB() == MBBlock)
        ret = true;
  return ret;
}

// Satisfying the restriction set by Before and After; returns
// an iterator to the earliet position possible withtin the MBBlock
// Before and After hold the instructions that should go
// before and after the set respectivley. In this case After is
// only used for validation checking
template <typename Container>
static MachineBasicBlock::iterator getEarliestInsertPos(MachineBasicBlock *MBBlock,
                                                        const Container &Before,
                                                        const Container &After) {
  auto InsertPosition = MBBlock->end();
  while (InsertPosition != MBBlock->begin()) {
    if (Before.count(&*std::prev(InsertPosition))) {
#ifndef NDEBUG  // Validation check
      for (auto Position = InsertPosition, E = MBBlock->begin(); Position != E; --Position)
        assert(!After.count(&*std::prev(Position)));
#endif
      break;
    }
    --InsertPosition;
  }
  return InsertPosition;
}

// Satisfying the restriction set by Before and After; returns
// an iterator to the earliet position possible withtin the MBBlock
// Before and After hold the instructions that should go
// before and after the set respectivley. In this case Before is
// only used for validation checking
template <typename Container>
static MachineBasicBlock::iterator  getLatestInsertPos(MachineBasicBlock *MBBlock,
                                                       const Container &Before,
                                                       const Container &After) {
  auto InsertPosition = MBBlock->begin();
  while (InsertPosition != MBBlock->end()) {
    if (After.count(&*InsertPosition)) {
#ifndef NDEBUG  // Validation check
      for (auto Position = InsertPosition, E = MBBlock->end(); Position != E; ++Position)
        assert(!Before.count(&*Position));
#endif
      break;
    }
    ++InsertPosition;
  }
  return InsertPosition;
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
      if (MI.getNumOperands() <=0) {
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
        if (&MBB == Loop->getBottomBlock() &&
            &MI == &*(--MBB.end()) &&
            MLI.getLoopFor(MBB.getFallThrough()) != Loop) {
          TII->negateCondBranch(&MI);
          action_opcode = XVM::BREAK;
        }
        // Check if we need to add a break statement at the end of loop
        int LevelBreakAfterLoop = -1;
        if (action_opcode == XVM::CONTINUE) {
          // 1. if the last stmt of the BB is a conditional branch statement and
          // 2. if the next BB is one of its successor and
          // 3. if the first statements of the next BB are END_BLOCK/END_LOOP
          //  insert a break if the above conditions are true
          MachineBasicBlock::reverse_iterator MBBI = MBB.rbegin();
          MachineInstr &MITmp = *MBBI;
          if (MITmp.isTerminator() && TII->isCondBranch(&MITmp)) {
            MachineBasicBlock *NextBB = MBB.getNextNode();
            bool NextBBIsSucc = false;
            for (auto *Succ : MBB.successors()) {
              if (Succ == NextBB) {
                NextBBIsSucc = true;
                break;
              }
            }
            if (NextBBIsSucc) {
              MachineBasicBlock::const_iterator MBBINext = NextBB->begin(), ENext = NextBB->end();
              while (MBBINext != ENext) {
                  MachineBasicBlock::const_iterator NMBBINext = std::next(MBBINext);
                  const MachineInstr &MINext = *MBBINext;
                  if (MINext.getOpcode() == XVM::END_BLOCK || MINext.getOpcode() == XVM::END_LOOP) {
                    LevelBreakAfterLoop ++;
                  } else {
                    break;
                  }
                  MBBINext = NMBBINext;
              }
            }
          }
        }
        MachineInstr *MIThen = MBB.getParent()->CreateMachineInstr(TII->get(XVM::THEN), DebugLoc());
        MBB.insertAfter(MI.getIterator(), MIThen);
        MachineInstr *MIAction = MBB.getParent()->CreateMachineInstr(TII->get(action_opcode), DebugLoc());
        MBB.insertAfter(MIThen->getIterator(), MIAction);
        MachineInstr *MIEndThen = MBB.getParent()->CreateMachineInstr(TII->get(XVM::END_THEN), DebugLoc());
        MBB.insertAfter(MIAction->getIterator(), MIEndThen);
        if (LevelBreakAfterLoop >= 0) {
          MachineInstr *MIElse = MBB.getParent()->CreateMachineInstr(TII->get(XVM::ELSE), DebugLoc());
          MBB.insertAfter(MIEndThen->getIterator(), MIElse);
          MachineInstr *MI_BREAK_IMM = MBB.getParent()->CreateMachineInstr(TII->get(XVM::BREAK_IMM), DebugLoc());
          MBB.insertAfter(MIElse->getIterator(), MI_BREAK_IMM);
          MachineInstrBuilder MIB(MF, MI_BREAK_IMM);
          MIB.addImm(LevelBreakAfterLoop);
          MachineInstr *MIEndElse = MBB.getParent()->CreateMachineInstr(TII->get(XVM::END_ELSE), DebugLoc());
          MBB.insertAfter(MI_BREAK_IMM->getIterator(), MIEndElse);
          MachineInstr *MIEndIf = MBB.getParent()->CreateMachineInstr(TII->get(XVM::END_IF), DebugLoc());
          MBB.insertAfter(MIEndElse->getIterator(), MIEndIf);
          ChangeBranchCondOpc(MI, TII);
        } else {
          MachineInstr *MIEndIf = MBB.getParent()->CreateMachineInstr(TII->get(XVM::END_IF), DebugLoc());
          MBB.insertAfter(MIEndThen->getIterator(), MIEndIf);
          ChangeBranchCondOpc(MI, TII);
        }
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
 *  Note: we may find other approach for fixing this.
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
void XVMCFGStackify::placeMarkers(MachineFunction &MFunction) {
  // To accommodate for the possible fake block we may insert at the end
  // we allocate one more than the number of blocks in the func
  ScopeTops.resize(MFunction.getNumBlockIDs() + 1);
  // If MBB is the header of a loop, place the LOOP for MBB.
  for (auto &MBBlock : MFunction)
    placeLoopMarker(MBBlock);

  for (auto &MBBlock : MFunction) {
    // Place the BLOCK for MBB if MBB is branched to from above.
    placeBlockMarker(MBBlock);
  }

  removeInFunctionRet(MFunction);
}

/// Insert a BLOCK marker for branches to MBB (if needed).
// Note: Consider a more generalized way of handling block (and also loop and
// try) signatures when we implement the multi-value proposal later.
void XVMCFGStackify::placeBlockMarker(MachineBasicBlock &MBBlock) {
  assert(!MBBlock.isEHPad());
  MachineFunction &MFunction = *MBBlock.getParent();
  auto &MDT = getAnalysis<MachineDominatorTree>();
  const auto &TII = *MFunction.getSubtarget<XVMSubtarget>().getInstrInfo();

  // For all forward non-fallthrough predecessors, compute the nearest commomn
  // dominator, reduceing stack height, by minimizing the time the block is on the stack
  MachineBasicBlock *Head = nullptr;
  bool BranchedTo = false;
  int MBBlockNumber = MBBlock.getNumber();
  for (MachineBasicBlock *Predecessor : MBBlock.predecessors()) {
    if (Predecessor->getNumber() < MBBlockNumber) {
      Head = Head ? MDT.findNearestCommonDominator(Head, Predecessor) : Predecessor;
      if (explicitlyBranchesTo(Predecessor, &MBBlock))
        BranchedTo = true;
    }
  }
  if (!(Head && BranchedTo))
    return;

  assert(&MBBlock != &MFunction.front() && "Header blocks shouldn't have predecessors");
  MachineBasicBlock *LayoutPredecessors = MBBlock.getPrevNode();

  // walk out to the nearest scope which isnt more deeply nester, if the nearest
  // common cominator is inside a more deeply nested context
  for (MachineFunction::iterator Iter(LayoutPredecessors), E(Head); Iter != E; --Iter) {
    if (MachineBasicBlock *ST = ScopeTops[Iter->getNumber()]) {
      if (ST->getNumber() > Head->getNumber()) {
        // Intervening scope, skip
        Iter = std::next(ST->getIterator());
      } else {
        // scope level at an appropriate depth. end
        Head = ST;
        break;
      }
    }
  }

  // Where in Head should we put the block
  // pre block instruction set
  SmallPtrSet<const MachineInstr *, 4> Before;
  // post block instruction set
  SmallPtrSet<const MachineInstr *, 4> After;
  for (const auto &MInst : *Head) {
    // If the bottom block of the loop is above MBB and there is a previosly placed
    // LOOP marker, it should be after the BLOCK, because the loop is nested inside it
    // Otherwise it should be before the BLOCK
    if (MInst.getOpcode() == XVM::LOOP) {
      auto *LoopBot = BeginToEnd[&MInst]->getParent()->getPrevNode();
      if (MBBlock.getNumber() > LoopBot->getNumber())
        After.insert(&MInst);
#ifndef NDEBUG
      else
        Before.insert(&MInst);
#endif
    }

    // If an END marker is before the current BLOCK's END marker, and its
    // corresponding BLOCK/TRY has been previously places, that should be palced
    // after this BLOCK, otherwise is hould before.
    if (MInst.getOpcode() == XVM::BLOCK) {
      if (BeginToEnd[&MInst]->getParent()->getNumber() <= MBBlock.getNumber())
        After.insert(&MInst);
#ifndef NDEBUG
      else
        Before.insert(&MInst);
#endif
    }

#ifndef NDEBUG  // END_(BLOCK|LOOP|TRY) markers should all be before the BLOCK.
    if (MInst.getOpcode() == XVM::END_BLOCK ||
        MInst.getOpcode() == XVM::END_LOOP)
      Before.insert(&MInst);
#endif

    if (MInst.isTerminator()) // Terminators should go after the BLOCK.
      After.insert(&MInst);
  }

  // Local expression tree are placed after the BLOCK.
  for (auto Iter = Head->getFirstTerminator(), E = Head->begin(); Iter != E; --Iter) {
    if (std::prev(Iter)->isDebugInstr() || std::prev(Iter)->isPosition())
      continue;
    if (isChild(*std::prev(Iter)))
      After.insert(&*std::prev(Iter));
    else
      break;
  }

  // Add the BLOCK.
//  XVM::BlockType ReturnType = XVM::BlockType::Void;
  auto InsertPosition = getLatestInsertPos(Head, Before, After);
  MachineInstr *Begin =
      BuildMI(*Head, InsertPosition, Head->findDebugLoc(InsertPosition),
              TII.get(XVM::BLOCK));
//Note: Check if we need it later

  // Decide where in Head to put the END_BLOCK.
  Before.clear();
  After.clear();
  for (auto &MInst : MBBlock) {
#ifndef NDEBUG
    // END_BLOCK should precede existing LOOP and TRY markers.
    if (MInst.getOpcode() == XVM::LOOP)
      After.insert(&MInst);
#endif

    // If the head of the loop is above this block's head, and there is a
    // previously places END_LOOP marker, the END_LOOp should be inserted
    // after the BLOCK, since the loop contains this block, otherwise, it
    // should be places before. the for END_TRY
    if (MInst.getOpcode() == XVM::END_LOOP) {
      if (EndToBegin[&MInst]->getParent()->getNumber() >= Head->getNumber())
        Before.insert(&MInst);
#ifndef NDEBUG
      else
        After.insert(&MInst);
#endif
    }
  }

  // End of the block marker.
  InsertPosition = getEarliestInsertPos(&MBBlock, Before, After);
  MachineInstr *End = BuildMI(MBBlock, InsertPosition, MBBlock.findPrevDebugLoc(InsertPosition),
                              TII.get(XVM::END_BLOCK));
  registerScope(Begin, End);

  // Track the farthest-spanning scope that ends at this point.
  updateScopeTops(Head, &MBBlock);
}

/// Insert a LOOP marker for a loop starting at MBB (if it's a loop header).
void XVMCFGStackify::placeLoopMarker(MachineBasicBlock &MBBlock) {
  MachineFunction &MFunction = *MBBlock.getParent();
  const auto &MLInfo = getAnalysis<MachineLoopInfo>();
  SortRegionInfo SRI(MLInfo);
  const auto &TII = *MFunction.getSubtarget<XVMSubtarget>().getInstrInfo();

  MachineLoop *MLoop = MLInfo.getLoopFor(&MBBlock);
  if (!MLoop || MLoop->getHeader() != &MBBlock) {
    return;
  }

  // The operand of a LOOP is the first block after the loop. If the loop is the
  // bottom of the function, insert a dummy block at the end.
  MachineBasicBlock *Bot = SRI.getBottom(MLoop);
  auto I = std::next(Bot->getIterator());
  if (I == MFunction.end()) {
    getAppendixBlock(MFunction);
    I = std::next(Bot->getIterator());
  }
  MachineBasicBlock *AfterLoop = &*I;

  // Find where in Header to put the MLOOP.
  SmallPtrSet<const MachineInstr *, 4> Before;
  SmallPtrSet<const MachineInstr *, 4> After;
  for (const auto &MInst : MBBlock) {
    // Add a loop tag here, after any existing loop.
    // Otherwise, we assume that the instruction is a loop.
    if (MInst.getOpcode() == XVM::END_LOOP)
      Before.insert(&MInst);
#ifndef NDEBUG
    else
      After.insert(&MInst);
#endif
  }

  // tag the start of the loop.
  auto InsertPosition = getEarliestInsertPos(&MBBlock, Before, After);
  //Note: modify the form of the LOOP instruction
  MachineInstr *Begin = BuildMI(MBBlock, InsertPosition, MBBlock.findDebugLoc(InsertPosition),
                                TII.get(XVM::LOOP));

  // Decide where in Header to put the END_LOOP.
  Before.clear();
  After.clear();
#ifndef NDEBUG
  for (const auto &MInst : MBBlock)
    // The current loop is not associated with the existing END_LOOP markers,
    // which belong to the parent loops.
    if (MInst.getOpcode() == XVM::END_LOOP)
      After.insert(&MInst);
#endif

  // Mark the end of the loop (using arbitrary debug location that branched to
  // the loop end as its location).
  InsertPosition = getEarliestInsertPos(AfterLoop, Before, After);
  DebugLoc EndDL = AfterLoop->pred_empty()
                      ? DebugLoc()
                      : (*AfterLoop->pred_rbegin())->findBranchDebugLoc();
  
  MachineInstr *End = BuildMI(*AfterLoop, InsertPosition, EndDL, TII.get(XVM::END_LOOP));
  registerScope(Begin, End);

  assert((!ScopeTops[AfterLoop->getNumber()] ||
          ScopeTops[AfterLoop->getNumber()]->getNumber() < MBBlock.getNumber()) &&
          "With block sorting the outermost loop for a block should be first.");
  updateScopeTops(&MBBlock, AfterLoop);
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
  SmallVector<EndMarkerInfo, INIT_SMALL_VECTOR_SCOPESS_SIZE> Stack;
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
        MachineInstr *PrevMI = MI.getPrevNode();
        if (PrevMI != NULL) {
          if (PrevMI->getOpcode() == XVM::END_BLOCK || PrevMI->getOpcode() == XVM::END_LOOP) {
            SetEndBlockLoop.insert(&MBB);
          }
        }
        break;
      }
      default:
        if (MI.isTerminator()) {
          // Rewrite MBB operands to be depth immediates.
          SmallVector<MachineOperand, INIT_SMALL_VECTOR_MO_SIZE> Ops(MI.operands());
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
  if (FakeBBlockCaller)
    MF.deleteMachineBasicBlock(FakeBBlockCaller);
  AppendixBBlock = FakeBBlockCaller = nullptr;
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
  releaseMemory();

  // Place the BLOCK/LOOP/TRY markers to indicate the beginnings of scopes.
  placeMarkers(MF);

  // Place the continue statements for each backedge of loops.
  fixBackEdgesOfLoops(MF);

  // Convert MBB operands in terminators to relative depth immediates.
  rewriteDepthImmediates(MF);

  // Add an end instruction at the end of the function body.
  if (!MF.getSubtarget<XVMSubtarget>()
           .getTargetTriple()
           .isOSBinFormatELF())
// Note: See if we need it later appendEndToFunction(MF, TII);

  cleanupFunctionData(MF);

  return true;
}

#endif
