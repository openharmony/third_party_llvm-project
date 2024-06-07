#ifdef XVM_DYLIB_MODE

#include "XVM.h"
#include "XVMSubtarget.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachinePostDominators.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Analysis/DomTreeUpdater.h"

using namespace llvm;

#define DEBUG_TYPE "xvm-cfg-structure"

namespace {
class XVMCFGStructure final : public MachineFunctionPass {
  StringRef getPassName() const override { return "XVM CFG Structure"; }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineDominatorTree>();
    AU.addRequired<MachinePostDominatorTree>();
    AU.addRequired<MachineLoopInfo>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

  // Placing markers
  void placeMarkers(MachineFunction &MF);
  void placeLoopMarker(MachineBasicBlock &MBB);
  void placeIfMarker(MachineBasicBlock &MBB);

  // Add break/continue for Loops
  void fixLoops(MachineBasicBlock &MBB);
public:
  static char ID; // Pass identification, replacement for typeid
  XVMCFGStructure() : MachineFunctionPass(ID) {}
  ~XVMCFGStructure() override {}
};
}

char XVMCFGStructure::ID = 0;
INITIALIZE_PASS(XVMCFGStructure, DEBUG_TYPE,
                "Insert LOOP/IF markers for XVM scopes", false,
                false)

FunctionPass *llvm::createXVMCFGStructure() {
  return new XVMCFGStructure();
}

static void getBeginInsertPos(MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator &MBBI) {
  MBBI = MBB.instr_begin();
  MachineBasicBlock::iterator E = MBB.instr_end();
  for (; MBBI != E; ++MBBI) {
    if (MBBI->getOpcode() == XVM::LOOP ||
        MBBI->getOpcode() == XVM::THEN ||
        MBBI->getOpcode() == XVM::ELSE ||
        MBBI->getOpcode() == XVM::BLOCK)
      continue;
    else
      return;
  }
}

static void getEndInsertPos(MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator &MBBI) {
  MBBI  = MBB.instr_begin();
  MachineBasicBlock::iterator E = MBB.instr_end();
  for (; MBBI != E; ++MBBI) {
    if (MBBI->getOpcode() == XVM::END_LOOP ||
        MBBI->getOpcode() == XVM::END_BLOCK ||
        MBBI->getOpcode() == XVM::END_THEN ||
        MBBI->getOpcode() == XVM::END_ELSE) {
      return;
    }
  }
}

void XVMCFGStructure::placeLoopMarker(MachineBasicBlock &MBB) {
  MachineLoopInfo &MLI = getAnalysis<MachineLoopInfo>();
  MachineFunction *MF = MBB.getParent();
  const auto &TII = MF->getSubtarget<XVMSubtarget>().getInstrInfo();

  MachineLoop *Loop = MLI.getLoopFor(&MBB);
  if (!Loop || Loop->getHeader() != &MBB)
    return;

  //We have the following 2 assumptions:
  // 1. the loop header should be first MBB within the loop
  // 2. the loop MBBs are continguous. This should be guaranteed
  // by the previous sorting pass.
  MachineBasicBlock *LoopTop = Loop->getTopBlock();
  MachineBasicBlock *LoopBottom = Loop->getBottomBlock();

  //verify that the loop header should be the first MBB within the loop
  if (LoopTop != &MBB) {
    assert ("we assume the first block is the loop header!");
  }

  MachineBasicBlock::iterator LoopInsertPos, LoopEndInsertPos;
  getBeginInsertPos(*LoopTop, LoopInsertPos);
  getEndInsertPos(*LoopBottom, LoopEndInsertPos);

  BuildMI(*LoopTop, LoopInsertPos,
          LoopTop->findDebugLoc(LoopInsertPos),
          TII->get(XVM::LOOP));
  //Note: the debug location might be problematic
  BuildMI(*LoopBottom, LoopEndInsertPos,
          LoopBottom->findDebugLoc(LoopEndInsertPos),
          TII->get(XVM::END_LOOP));
}

static bool isContinueBlock(MachineBasicBlock &MBB) {
  for (auto &MI : MBB) {
    if (MI.getOpcode() == XVM::CONTINUE)
      return true;
  }
  return false;
}

void XVMCFGStructure::placeIfMarker(MachineBasicBlock &MBB) {
  // We assume 2 things:
  // 1. the then part and the else part are contingous
  // 2. at the end of the MBB, only one branch instruction

  // We need to negate the conditions in each branch
  MachineFunction *MF = MBB.getParent();
  const auto &TII = MF->getSubtarget<XVMSubtarget>().getInstrInfo();
  getAnalysis<MachineDominatorTree>();
  auto &MPDT = getAnalysis<MachinePostDominatorTree>();

  if (!MBB.canFallThrough() || MBB.empty())
    return;

  MachineBasicBlock::iterator MBBFirstTerminator = MBB.getFirstTerminator();
  if (MBBFirstTerminator == MBB.end() || !TII->isCondBranch(&(*MBBFirstTerminator)))
    return;

  MachineBasicBlock *ThenBeginBlock = MBB.getFallThrough();
  MachineBasicBlock *ElseOrOtherBeginBlock = MBB.back().getOperand(0).getMBB();
  MachineBasicBlock *ThenEndBlock = &*--ElseOrOtherBeginBlock->getIterator();

  MachineBasicBlock::iterator ThenInsertPos, ThenEndInsertPos;
  getBeginInsertPos(*ThenBeginBlock, ThenInsertPos);
  getEndInsertPos(*ThenEndBlock, ThenEndInsertPos);
  BuildMI(*ThenBeginBlock, ThenInsertPos,
          ThenBeginBlock->findDebugLoc(ThenInsertPos),
          TII->get(XVM::THEN));
  BuildMI(*ThenEndBlock, ThenEndInsertPos,
          ThenEndBlock->findDebugLoc(ThenEndInsertPos),
          TII->get(XVM::END_THEN));

  bool HaveElse = !MPDT.properlyDominates(ElseOrOtherBeginBlock, ThenBeginBlock);

  MachineBasicBlock::iterator ElseInsertPos, ElseEndInsertPos;
  MachineBasicBlock *ElseEndBlock = nullptr;

  if (HaveElse) {
    // We need to handle Continue in a special case currently
    if (isContinueBlock(*ElseOrOtherBeginBlock)) {
      ElseEndBlock = ElseOrOtherBeginBlock;
      getBeginInsertPos(*ElseOrOtherBeginBlock, ElseInsertPos) ;
      getEndInsertPos(*ElseOrOtherBeginBlock, ElseEndInsertPos) ;
    } else {
      MachineBasicBlock *CommonSucc = MPDT.findNearestCommonDominator(ThenBeginBlock, ElseOrOtherBeginBlock);
      ElseEndBlock = &*--CommonSucc->getIterator();
      getBeginInsertPos(*ElseOrOtherBeginBlock, ElseInsertPos) ;
      getEndInsertPos(*ElseEndBlock, ElseEndInsertPos) ;
    }
    BuildMI(*ElseOrOtherBeginBlock, ElseInsertPos,
            ElseOrOtherBeginBlock->findDebugLoc(ElseInsertPos),
            TII->get(XVM::ELSE));
    BuildMI(*ElseEndBlock, ElseEndInsertPos,
            ElseEndBlock->findDebugLoc(ElseEndInsertPos),
            TII->get(XVM::END_ELSE));
    ++ElseEndInsertPos;
    BuildMI(*ElseEndBlock, ElseEndInsertPos,
            ElseEndBlock->findDebugLoc(ElseEndInsertPos),
            TII->get(XVM::END_IF));
  } else {
    ++ThenEndInsertPos;
    BuildMI(*ThenEndBlock, ThenEndInsertPos,
            ThenEndBlock->findDebugLoc(ThenEndInsertPos),
            TII->get(XVM::END_IF));
  }
}

void XVMCFGStructure::fixLoops(MachineBasicBlock &MBB) {
  // We do the following operations:
  // 1. Every backedge is a continue
  // 2. Every exit condition leads to a break
  //
  // We assume the following 2:
  // 1. The loop has single entry (reducible)
  // 2. The loop exit block is unique
  // Assumption 1 is realisitic in most of times.
  // Assumption 2 is not. In next step, we add (nested) blocks which end before exit blocks,
  // we added breaks (with indexes) in every exiting block
  MachineFunction *MF = MBB.getParent();
  auto &MLI = getAnalysis<MachineLoopInfo>();
  const auto &TII = MF->getSubtarget<XVMSubtarget>().getInstrInfo();
  MachineLoop *Loop = MLI.getLoopFor(&MBB);
  SmallVector<MachineBasicBlock *, 4> ExitingBlocks;
  auto &MPDT = getAnalysis<MachinePostDominatorTree>();
  using DomTreeT = PostDomTreeBase<MachineBasicBlock>;
  SmallVector<DomTreeT::UpdateType, 16> PDTUpdates;

  if (!Loop || Loop->getHeader() != &MBB) // MBB must be loop head
    return;

  Loop->getExitingBlocks(ExitingBlocks);

  // For every predecessor of loop header, if it is within the loop,
  // then it is supposed to insert a continue at the backedge.
  for (MachineBasicBlock *Pred : MBB.predecessors()) {
    if (Loop->contains(Pred)) {
      // create continue block
      MachineBasicBlock *ContinueBlock = MF->CreateMachineBasicBlock();
      auto ContinueInsertPos = ContinueBlock->begin();
      BuildMI(*ContinueBlock, ContinueInsertPos,
              ContinueBlock->findDebugLoc(ContinueInsertPos),
              TII->get(XVM::CONTINUE));

      // directly put it after the source of the backedge,
      // modify the branch target to the direct successor
      // later in placeIfMarker, we will negate all branch conditions
      bool canFallThrough = Pred->canFallThrough();
      MachineBasicBlock *NewSucc = canFallThrough ?
                                   Pred->getFallThrough() :
                                   ContinueBlock;

       //Note: Does adding the new block destroy the loop structure?
      MachineFunction::iterator MBBI = ++Pred->getIterator();
      MF->insert(MBBI, ContinueBlock);

      MachineBasicBlock *Old = &MBB;
      if (Old != NewSucc) {
        // Note: do we need to do anything for else case??
        Pred->ReplaceUsesOfBlockWith(&MBB, NewSucc);
      } else {
        report_fatal_error("old MBB == new succ, need to handle this case");
      }

      MPDT.getBase().applyUpdates({{DomTreeT::Delete, &*Pred, &MBB},
                                   {DomTreeT::Insert, &*Pred, NewSucc}});
      if (canFallThrough) {
        Pred->addSuccessorWithoutProb(ContinueBlock);
        MPDT.getBase().applyUpdates({{DomTreeT::Insert, &*Pred, ContinueBlock}});
      }
      ContinueBlock->addSuccessor(&MBB, BranchProbability::getOne());
      MPDT.getBase().applyUpdates({{DomTreeT::Insert, ContinueBlock, &MBB}});
      // we jump to the fall through and the continue block became new fall through
      Loop->addBasicBlockToLoop(ContinueBlock, MLI.getBase());
    }
  }
  // fin adding continue

  // For every ExitingBlock, we insert a break instruction
  while (!ExitingBlocks.empty()) {
    MachineBasicBlock *LoopExiting = ExitingBlocks.pop_back_val();
    assert(LoopExiting->canFallThrough() && "Loop exiting blocks must have fall through!");

    // create break block
    MachineBasicBlock *BreakBlock = MF->CreateMachineBasicBlock();
    auto BreakInsertPos = BreakBlock->begin();
    BuildMI(*BreakBlock, BreakInsertPos,
            BreakBlock->findDebugLoc(BreakInsertPos),
            TII->get(XVM::BREAK));

    // Note: to insert blocks for multiple exits
    MachineBasicBlock *FallThrough = LoopExiting->getFallThrough();
    MachineFunction::iterator MBBI2 = FallThrough->getIterator();
    MF->insert(MBBI2, BreakBlock);
    Loop->addBasicBlockToLoop(BreakBlock, MLI.getBase());

    // puts the break block in place
    MachineBasicBlock *JumpTarget = LoopExiting->back().getOperand(0).getMBB();
    if (JumpTarget != FallThrough) {
      // when the last instruction in the block is conditional branch
      LoopExiting->ReplaceUsesOfBlockWith(JumpTarget, FallThrough);
      LoopExiting->addSuccessorWithoutProb(BreakBlock);
    } else {
      // when the last instruction in the block is unconditional branch
      // need to find the last conditional branch, insert break between LoopExiting its JumpTarget
      for (MachineBasicBlock::reverse_iterator It = LoopExiting->rbegin(), E = LoopExiting->rend();
           It != E; ++It) {
        MachineInstr *MI = &*It;
        if (MI) {
          if (TII->isCondBranch(MI)) {
            JumpTarget = MI->getOperand(0).getMBB();
            LoopExiting->ReplaceUsesOfBlockWith(JumpTarget, BreakBlock);
            break;
          }
        }
      }
    }
    MPDT.getBase().applyUpdates({{DomTreeT::Delete, LoopExiting, JumpTarget},
                                 {DomTreeT::Insert, LoopExiting, FallThrough}});
    // The condition need to be negated
    MPDT.getBase().applyUpdates({{DomTreeT::Insert, LoopExiting, BreakBlock}});
    BreakBlock->addSuccessor(JumpTarget, BranchProbability::getOne());
    MPDT.getBase().applyUpdates({{DomTreeT::Insert, BreakBlock, JumpTarget}});
  }
}

// Insert LOOP/END_LOOP/THEN/END_THEN/ELSE/ELSE_END
void XVMCFGStructure::placeMarkers(MachineFunction &MF) {
  for (auto &MBB : MF) {
    fixLoops(MBB);
    placeLoopMarker(MBB);
  }

  for (auto &MBB : MF) {
    placeIfMarker(MBB);
  }
}

bool XVMCFGStructure::runOnMachineFunction(MachineFunction &MF) {
  LLVM_DEBUG(dbgs() << "********** CFG Stackifying **********\n"
                       "********** Function: "
                    << MF.getName() << '\n');

  placeMarkers(MF);

  return true;
}

#endif
