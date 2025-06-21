#ifdef XVM_DYLIB_MODE

#include "XVM.h"
#include "XVM_def.h"
#include "XVMInstrInfo.h"
#include "XVMTargetMachine.h"
#include "XVMErrorMsg.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "MCTargetDesc/XVMInstPrinter.h"
#include "llvm/CodeGen/MachineModuleInfo.h"

#define DEBUG_TYPE "xvm-ref-trace"

using namespace llvm;

#define XVM_REF_DETERMINE_NAME "XVM pseudo instruction ref determine pass"
#define XVM_SYM_REG_NON_REF 0b00000000
#define XVM_SYM_REG_REF     0b00000001
#define XVM_MAX_NUM_PARAM 5
#define SRET_REG_NUM 6
#define NUM_MO_2 2
#define NUM_MO_3 3

#define MO_FIRST  0
#define MO_SECOND 1
#define MO_THIRD  2
#define MAX_IMM 8191

/** mov rd #simm (16 bits)
  * movk rd, #uimm (16 bits), #shift (0:no 1:16bits 2:32bits 3:48bits)
  * addref rd, rn, #uimm (14bits)
  */
#define NUM_OF_IMM_BITS_ADDREF 14
#define NUM_OF_IMM_BITS_MOV    16
#define NUM_OF_IMM_BITS_MOVK1  32
#define NUM_OF_IMM_BITS_MOVK2  48

#define NUM_MO_PER_PHI_BRANCH 2
#define INIT_SMALL_VECTOR_SIZE 4

namespace {
class XVMUpdateRefInstrForMI : public MachineFunctionPass {
public:
  static char ID;
  const XVMInstrInfo *TII = nullptr;
  XVMUpdateRefInstrForMI() : MachineFunctionPass(ID) {
    initializeXVMUpdateRefInstrForMIPass(*PassRegistry::getPassRegistry());
  }
  bool runOnMachineFunction(MachineFunction &MF) override;
  StringRef getPassName() const override { return XVM_REF_DETERMINE_NAME; }
private:
  bool scanRefInfoInMBB(MachineBasicBlock &MBB);
  bool updateRefInfoBasedInMBB(MachineBasicBlock &MBB);

  bool checkAndUpdateOrInMBB(MachineBasicBlock &MBB);
  void updatePtrRefInMBB(MachineBasicBlock &MBB);
  void doubleCheckPhiMIWithRef(MachineBasicBlock &MBB);
  void doubleCheckRefs(MachineBasicBlock &MBB);
  bool updateRegistersOfMIInMBB(MachineBasicBlock &MBB);
  bool finalFixRefs(void);
  void FindNonRefRegInFunc(const MachineFunction &MF);
};
  char XVMUpdateRefInstrForMI::ID = 0;
}

static std::map<MachineInstr *, MachineBasicBlock *> MapMIToBeFixed;
static std::map<Register, unsigned char> MapRefRegInFunc;
static std::map<Register, unsigned int> MapPtrRegInFunc;
static std::set<Register> SetNonRefRegInFunc;

static bool IsValidRefOpCode(unsigned OpCode) {
  switch (OpCode) {
  case XVM::ADD_ri:
  case XVM::ADD_rr:
  case XVM::SUB_ri:
  case XVM::SUB_rr:
  case XVM::OR_ri:
  case XVM::OR_rr:
  case XVM::XOR_ri:
  case XVM::XOR_rr:
  case XVM::AND_ri:
  case XVM::AND_rr:
  case XVM::STW:
  case XVM::STH:
  case XVM::STB:
  case XVM::STD:
  case XVM::LDW_z:
  case XVM::LDH_z:
  case XVM::LDB_z:
  case XVM::LDW:
  case XVM::LDH:
  case XVM::LDB:
  case XVM::LDD:
  case XVM::MOV_rr:
    return true;
  default:
    return false;
  }
}

static inline void SetRegTypeForMO(MachineOperand &MO, unsigned char RegType) {
  if (MO.isReg()) {
    Register regNo = MO.getReg();
    std::map<Register, unsigned char>::iterator I = MapRefRegInFunc.find(regNo);
    if (I == MapRefRegInFunc.end()) {
      MapRefRegInFunc.insert(std::pair<Register, unsigned char>(regNo, RegType));
    } else {
      if (IsValidRefOpCode(MO.getParent()->getOpcode())) {
        MapRefRegInFunc[regNo] = MapRefRegInFunc[regNo] | RegType;
      }
    }
  }
}

static void CheckFunctionReturn(Function &F) {
  Type *Ty = F.getReturnType();
  /* Return is always r0 for xvm */
  if (auto *PTy = dyn_cast<PointerType>(Ty)) {
    MapRefRegInFunc.insert(std::pair<Register, unsigned char>(XVM::R0, XVM_SYM_REG_REF));
   } else if (Ty->isIntegerTy()) {
    MapRefRegInFunc.insert(std::pair<Register, unsigned char>(XVM::R0, XVM_SYM_REG_NON_REF));
  } else if (Ty->isVoidTy()) {
    ;
  } else {
    llvm_unreachable("Invalid return type");
  }
}

static Register inline getPhysicalRegister(unsigned index) {
  switch (index) {
  case 0: return XVM::R0;
  case 1: return XVM::R1;
  case 2: return XVM::R2;
  case 3: return XVM::R3;
  case 4: return XVM::R4;
  case 5: return XVM::R5;
  case 6: return XVM::R6;
  case 7: return XVM::R7;
  case 8: return XVM::R8;
  case 9: return XVM::R9;
  case 10: return XVM::R10;
  case 11: return XVM::R11;
  case 12: return XVM::R12;
  case 13: return XVM::R13;
  case 14: return XVM::R14;
  case 15: return XVM::R15;
  case 16: return XVM::R16;
  case 17: return XVM::R17;
  default:
    llvm_unreachable("Invalid physical register index");
  }
}

static void CheckFunctionArgs(Function &F) {
  int idx = 0;
  /* Here we assume r0, ..., r5 are the registers for input parameters
   * We only need to check the register ones: for others in stack,
   * it is handled with check/update steps followed.
   */
  for (Function::const_arg_iterator I = F.arg_begin(), E = F.arg_end(); I != E; ++I) {
    if (idx > XVM_MAX_NUM_PARAM) {
      break;
    }
    Type *Ty = I->getType();
    std::string regName = "r" + itostr(idx);
    if (auto *PTy = dyn_cast<PointerType>(Ty)) {
      LLVM_DEBUG(dbgs() << "arg[" << idx << "]=" << I->getName().data() << " is ref.\n");
      if (!I->hasAttribute(Attribute::StructRet)) {
        MapRefRegInFunc.insert(std::pair<Register, unsigned char>(getPhysicalRegister(idx), XVM_SYM_REG_REF));
      } else {
        // R6 is used to pass the sret return
        MapRefRegInFunc.insert(std::pair<Register, unsigned char>(getPhysicalRegister(SRET_REG_NUM),
                                                                  XVM_SYM_REG_REF));
      }
    } else if (Ty->isIntegerTy()) {
      MapRefRegInFunc.insert(std::pair<Register, unsigned char>(getPhysicalRegister(idx), XVM_SYM_REG_NON_REF));
      LLVM_DEBUG(dbgs() << "arg[" << idx << "]=" << I->getName().data() << " is not ref.\n");
    } else {
      LLVM_DEBUG(dbgs() << "XVM Error: Invalid param type");
    }
    idx++;
  }
}

static void setRefFlagFor2Ops(MachineOperand &MO_def, MachineOperand &MO_use) {
  /* verify def */
  if (MO_def.isReg()) {
    if (!MO_def.isDef() && !MO_def.isImplicit() && !MO_def.isKill()) {
      llvm_unreachable("unhandled Operand!!");
    }
  }
  /* check use */
  if (MO_use.isReg()) {
    if (!MO_def.isDef() && !MO_def.isImplicit() && !MO_def.isKill()) {
      llvm_unreachable("unhandled Operand!!");
    }
    Register regNo = MO_use.getReg();
    std::map<Register, unsigned char>::iterator I = MapRefRegInFunc.find(regNo);
    if (I == MapRefRegInFunc.end()) {
      SetRegTypeForMO(MO_def, XVM_SYM_REG_NON_REF);
    } else {
      SetRegTypeForMO(MO_def, I->second);
    }
  }
}

static inline void updateRefMapForRefInst(MachineOperand &MO_use, unsigned char flag) {
  std::map<Register, unsigned char>::iterator I1 = MapRefRegInFunc.find(MO_use.getReg());
  if (I1 == MapRefRegInFunc.end()) {
    SetRegTypeForMO(MO_use, flag);
  }
  else
    I1->second = flag;
}

static void checkSimpleMIWithRef(MachineInstr &MI) {
  if (MI.getOpcode() == XVM::COPY) {
    assert(MI.getNumOperands() == NUM_MO_2);
    MachineOperand &MO_def = MI.getOperand(MO_FIRST);
    MachineOperand &MO_use = MI.getOperand(MO_SECOND);
    setRefFlagFor2Ops(MO_def, MO_use);
  }
}

static bool updateCopyMIWithRef(MachineInstr &MI, const XVMInstrInfo *TII) {
  /* No update for Copy */
  return false;
}

static void checkMovMIWithRef(MachineInstr &MI) {
  if (MI.getOpcode() == XVM::MOV_rr) {
    assert(MI.getNumOperands() == NUM_MO_2);
    MachineOperand &MO_def = MI.getOperand(MO_FIRST);
    MachineOperand &MO_use = MI.getOperand(MO_SECOND);
    setRefFlagFor2Ops(MO_def, MO_use);
    if (MO_use.isFI()) {
      SetRegTypeForMO(MO_def, XVM_SYM_REG_REF);
    }
  }
  if (MI.getOpcode() == XVM::MOV_ri) {
    assert(MI.getNumOperands() == NUM_MO_2);
    MachineOperand &MO_def = MI.getOperand(MO_FIRST);
    SetRegTypeForMO(MO_def, XVM_SYM_REG_NON_REF);
  }
}

static bool updateMovMIWithRef(MachineInstr &MI, const XVMInstrInfo *TII) {
  /* No update for Move */
  return false;
}

static void checkLoadMIWithRef(MachineInstr &MI) {
  if (MI.getOpcode() == XVM::LDB   || MI.getOpcode() == XVM::LDH   || MI.getOpcode() == XVM::LDW   ||
      MI.getOpcode() == XVM::LDB_z || MI.getOpcode() == XVM::LDH_z || MI.getOpcode() == XVM::LDW_z ||
      MI.getOpcode() == XVM::LDD) {
    assert(MI.getNumOperands() == NUM_MO_3);
    MachineOperand &MO_def = MI.getOperand(MO_FIRST);
    MachineOperand &MO_use = MI.getOperand(MO_SECOND);
    /* verify def */
    if (MO_def.isReg()) {
      if (!MO_def.isDef() && !MO_def.isImplicit() && !MO_def.isKill()) {
        llvm_unreachable("unhandled Operand!!");
      }
    }
    /* check use */
    if (MO_use.isReg()) {
      if (!MO_def.isDef() && !MO_def.isImplicit() && !MO_def.isKill()) {
        llvm_unreachable("unhandled Operand!!");
      }
      // always be ref
      SetRegTypeForMO(MO_use, XVM_SYM_REG_REF);
    }
  }

  if (MI.getOpcode() == XVM::LD_global_imm64) {
    assert(MI.getNumOperands() == NUM_MO_2);
    MachineOperand &MO_def = MI.getOperand(MO_FIRST);
    MachineOperand &MO_use = MI.getOperand(MO_SECOND);
    if (MO_use.isGlobal()) {
      LLVM_DEBUG(dbgs() << "Global:" << MO_use.getGlobal()->getName().data() << " to load.\n");
      SetRegTypeForMO(MO_def, XVM_SYM_REG_REF);
    }
  }
}

static bool updateLoadMIWithRef(MachineInstr &MI, const XVMInstrInfo *TII) {
  if (MI.getOpcode() == XVM::LDD) {
    assert(MI.getNumOperands() == NUM_MO_3);
    MachineOperand &MO_def = MI.getOperand(MO_FIRST);
    /* if MO_def is a ref (it is determined when it is used somewhere), then it should be ldrref */
    Register regNo = MO_def.getReg();
    std::map<Register, unsigned char>::iterator I = MapRefRegInFunc.find(regNo);
    if (I != MapRefRegInFunc.end()) {
      if (I->second == XVM_SYM_REG_REF &&
        SetNonRefRegInFunc.find(regNo) == SetNonRefRegInFunc.end()) {
        MI.setDesc(TII->get(XVM::LoadRef_ri));
        return true;
      }
    }
  }
  return false;
}

static void checkAddSubMIWithRef(MachineInstr &MI) {
  if (MI.getOpcode() == XVM::ADD_ri || MI.getOpcode() == XVM::ADD_rr ||
      MI.getOpcode() == XVM::SUB_ri || MI.getOpcode() == XVM::SUB_rr) {
    assert(MI.getNumOperands() == NUM_MO_3);
    MachineOperand &MO_def = MI.getOperand(MO_FIRST);
    MachineOperand &MO_use = MI.getOperand(MO_SECOND);
    setRefFlagFor2Ops(MO_def, MO_use);
    return;
  }
}

static void checkOrXorAndMIWithRef(MachineInstr &MI) {
  if (MI.getOpcode() == XVM::OR_ri ||
      MI.getOpcode() == XVM::XOR_ri||
      MI.getOpcode() == XVM::AND_ri) {
    assert(MI.getNumOperands() == NUM_MO_3);
    MachineOperand &MO_def = MI.getOperand(MO_FIRST);
    MachineOperand &MO_use = MI.getOperand(MO_SECOND);
    setRefFlagFor2Ops(MO_def, MO_use);
    return;
  }
  if (MI.getOpcode() == XVM::OR_rr ||
      MI.getOpcode() == XVM::XOR_rr||
      MI.getOpcode() == XVM::AND_rr) {
    assert(MI.getNumOperands() == NUM_MO_3);
    MachineOperand &MO_def = MI.getOperand(MO_FIRST);
    MachineOperand &MO_use = MI.getOperand(MO_SECOND);
    MachineOperand &MO_use2 = MI.getOperand(MO_THIRD);
    Register regNo1 = MO_use.getReg();
    Register regNo2 = MO_use2.getReg();
    std::map<Register, unsigned char>::iterator I1 = MapRefRegInFunc.find(regNo1);
    std::map<Register, unsigned char>::iterator I2 = MapRefRegInFunc.find(regNo2);
    if (I1 != MapRefRegInFunc.end() && I1->second == XVM_SYM_REG_REF) {
      if (I2 != MapRefRegInFunc.end() && I2->second == XVM_SYM_REG_REF) {
        ;
      } else {
        setRefFlagFor2Ops(MO_def, MO_use);
      }
    } else {
      if (I2 != MapRefRegInFunc.end() && I2->second == XVM_SYM_REG_REF) {
        setRefFlagFor2Ops(MO_def, MO_use);
        setRefFlagFor2Ops(MO_def, MO_use2);
      } else {
        ;
      }
    }
    return;
  }
}

static inline bool updateAddSubWithSubAddForImm(MachineInstr &MI, const XVMInstrInfo *TII) {
  if (MI.getOpcode() == XVM::ADD_ri || MI.getOpcode() == XVM::SUB_ri) {
    assert(MI.getOperand(MO_THIRD).isImm());
    int64_t imm = MI.getOperand(MO_THIRD).getImm();
    if (imm < 0) {
      imm = -imm;
      MI.getOperand(MO_THIRD).setImm(imm);
      if (MI.getOpcode() == XVM::ADD_ri) {
        MI.setDesc(TII->get(XVM::SUB_ri));
        return true;
      } else {
        MI.setDesc(TII->get(XVM::ADD_ri));
        return true;
      }
    }
  }
  return false;
}

static inline void updateAddRiWithRef(MachineInstr &MI, const XVMInstrInfo *TII) {
  assert(MI.getOperand(MO_THIRD).isImm());
  int64_t imm = MI.getOperand(MO_THIRD).getImm();
  if (imm < 0) {
    imm = -imm;
    MI.getOperand(MO_THIRD).setImm(imm);
    MI.setDesc(TII->get(XVM::SubRef_ri));
  } else {
    MI.setDesc(TII->get(XVM::AddRef_ri));
  }
}

static inline bool updateAddMIWithRef(MachineInstr &MI, const XVMInstrInfo *TII) {
  assert(MI.getNumOperands() == NUM_MO_3);
  MachineOperand &MO_def = MI.getOperand(MO_FIRST);
  MachineOperand &MO_use1 = MI.getOperand(MO_SECOND);
  /* if MO_def is a ref (it is determined when it is used somewhere), then it should be addref */
  Register regNo = MO_def.getReg();
  std::map<Register, unsigned char>::iterator I = MapRefRegInFunc.find(regNo);
  // Update instr if MO_def is ref
  if (I != MapRefRegInFunc.end()) {
    if (I->second == XVM_SYM_REG_REF &&
        SetNonRefRegInFunc.find(regNo) == SetNonRefRegInFunc.end()) {
        // Update MO_use1 to be ref if MO_def is ref
        if (MI.getOpcode() == XVM::ADD_ri) {
          updateRefMapForRefInst(MO_use1, XVM_SYM_REG_REF);
          updateAddRiWithRef(MI, TII);
        } else {
          // It may be the case there use2 is a ref while use1 is not a ref
          MachineOperand &MO_use2 = MI.getOperand(2);
          Register regNoUse1 = MO_use1.getReg();
          Register regNoUse2 = MO_use2.getReg();
          std::map<Register, unsigned char>::iterator ItrUse1 = MapRefRegInFunc.find(regNoUse1);
          std::map<Register, unsigned char>::iterator ItrUse2 = MapRefRegInFunc.find(regNoUse2);
          if (ItrUse2 != MapRefRegInFunc.end()) {
            if (ItrUse2->second == XVM_SYM_REG_REF &&
              SetNonRefRegInFunc.find(regNoUse2) == SetNonRefRegInFunc.end()) {
                // Now Use2 is a ref: we need to Switch Operand1 and Operand2
                updateRefMapForRefInst(MO_use1, XVM_SYM_REG_NON_REF);

                // by create a new MI
                MachineBasicBlock::iterator II = MI.getIterator();
                MachineBasicBlock &MBB = *MI.getParent();
                MachineFunction *MF = MBB.getParent();
                DebugLoc DL = MI.getDebugLoc();
                MachineRegisterInfo &MRI = MF->getRegInfo();
                MachineInstr *ReplaceMI = BuildMI(MBB, II, DL, TII->get(XVM::AddRef_rr));

                MachineOperand &MO_def = MI.getOperand(0);
                MachineOperand &MO_use1 = MI.getOperand(1);
                MachineOperand &MO_use2 = MI.getOperand(2);

                ReplaceMI->addOperand(MO_def);
                ReplaceMI->addOperand(MO_use2);
                ReplaceMI->addOperand(MO_use1);
                MBB.remove_instr(&MI);
                MRI.verifyUseLists();
              } else {
                // Now Use2 is not ref,  then Use1 should be ref
                updateRefMapForRefInst(MO_use1, XVM_SYM_REG_REF);
                MI.setDesc(TII->get(XVM::AddRef_rr));
              }
          }
        }
        return true;
    }
  }
  // Update instr if MO_use1 if ref
  regNo = MO_use1.getReg();
  I = MapRefRegInFunc.find(regNo);
  if (I != MapRefRegInFunc.end()) {
    if (I->second == XVM_SYM_REG_REF &&
        SetNonRefRegInFunc.find(regNo) == SetNonRefRegInFunc.end()) {
        if (MI.getOpcode() == XVM::ADD_ri)
          updateAddRiWithRef(MI, TII);
        else
          MI.setDesc(TII->get(XVM::AddRef_rr));
        return true;
      }
  }
  return false;
}

static inline bool updateSubriMIWithRef(MachineInstr &MI, const XVMInstrInfo *TII) {
  assert(MI.getNumOperands() == NUM_MO_3);
  MachineOperand &MO_def = MI.getOperand(MO_FIRST);
  MachineOperand &MO_use = MI.getOperand(MO_SECOND);
  /* if MO_def is a ref (it is determined when it is used somewhere), then it should be subref */
  Register regNo = MO_def.getReg();
  std::map<Register, unsigned char>::iterator I = MapRefRegInFunc.find(regNo);
  if (I != MapRefRegInFunc.end()) {
    if (I->second == XVM_SYM_REG_REF &&
        SetNonRefRegInFunc.find(regNo) == SetNonRefRegInFunc.end()) {
      updateRefMapForRefInst(MO_use, XVM_SYM_REG_REF);
      if (MI.getOpcode() == XVM::SUB_ri)
        MI.setDesc(TII->get(XVM::SubRef_ri));
      else
        MI.setDesc(TII->get(XVM::SubRef_rr));
      return true;
    }
  }
  regNo = MO_use.getReg();
  I = MapRefRegInFunc.find(regNo);
  if (I != MapRefRegInFunc.end()) {
    if (I->second == XVM_SYM_REG_REF &&
        SetNonRefRegInFunc.find(regNo) == SetNonRefRegInFunc.end()) {
      if (MI.getOpcode() == XVM::SUB_ri)
        MI.setDesc(TII->get(XVM::SubRef_ri));
      else
        MI.setDesc(TII->get(XVM::SubRef_rr));
      return true;
    }
  }
  return false;
}

static inline bool updateSubrrMIWithRef(MachineInstr &MI, const XVMInstrInfo *TII) {
  assert(MI.getNumOperands() == NUM_MO_3);
  MachineOperand &MO_def = MI.getOperand(MO_FIRST);
  MachineOperand &MO_use1 = MI.getOperand(MO_SECOND);
  MachineOperand &MO_use2 = MI.getOperand(MO_THIRD);
  /* if MO_def is a ref (it is determined when it is used somewhere), then it should be subref */
  Register regNo = MO_def.getReg();
  std::map<Register, unsigned char>::iterator I = MapRefRegInFunc.find(regNo);
  if (I != MapRefRegInFunc.end()) {
    if (I->second == XVM_SYM_REG_REF &&
        SetNonRefRegInFunc.find(regNo) == SetNonRefRegInFunc.end()) {
      updateRefMapForRefInst(MO_use1, XVM_SYM_REG_REF);
      I = MapRefRegInFunc.find(MO_use2.getReg());
      if (I != MapRefRegInFunc.end() && I->second == XVM_SYM_REG_REF) {
        MI.setDesc(TII->get(XVM::DifRef_rr));
      } else {
        MI.setDesc(TII->get(XVM::SubRef_rr));
      }
      return true;
    }
  }
  regNo = MO_use1.getReg();
  I = MapRefRegInFunc.find(regNo);
  if (I != MapRefRegInFunc.end()) {
    if (I->second == XVM_SYM_REG_REF &&
        SetNonRefRegInFunc.find(regNo) == SetNonRefRegInFunc.end()) {
      if (MO_use2.isReg()) {
        I = MapRefRegInFunc.find(MO_use2.getReg());
        if (I != MapRefRegInFunc.end() && I->second == XVM_SYM_REG_REF) {
          MI.setDesc(TII->get(XVM::DifRef_rr));
        } else {
          MI.setDesc(TII->get(XVM::SubRef_rr));
        }
        return true;
      }
    }
  }
  return false;
}

static bool updateAddSubMIWithRef(MachineInstr &MI, const XVMInstrInfo *TII) {
  bool Modified = false;
  switch (MI.getOpcode()) {
    case XVM::ADD_ri:
    case XVM::ADD_rr:
      Modified = updateAddMIWithRef(MI, TII);
      return Modified | updateAddSubWithSubAddForImm(MI, TII);
    case XVM::SUB_ri:
      Modified = updateSubriMIWithRef(MI, TII);
      return Modified | updateAddSubWithSubAddForImm(MI, TII);
    case XVM::SUB_rr:
      return updateSubrrMIWithRef(MI, TII);
    default:
      return false;
  }
}

static inline bool switchOperandUse1Use2(MachineBasicBlock &MBB, MachineInstr &MI, const XVMInstrInfo *TII) {
  // we have cases where use1 is not ref while use2 is ref
  if (MI.getOpcode() == XVM::OrRef_rr ||
      MI.getOpcode() == XVM::XorRef_rr ||
      MI.getOpcode() == XVM::AndRef_rr) {
    MachineOperand &MO_def = MI.getOperand(MO_FIRST);
    MachineOperand &MO_use1 = MI.getOperand(MO_SECOND);
    MachineOperand &MO_use2 = MI.getOperand(MO_THIRD);
    Register regNo = MO_use2.getReg();
    std::map<Register, unsigned char>::iterator I = MapRefRegInFunc.find(regNo);
    if (I != MapRefRegInFunc.end() &&
        SetNonRefRegInFunc.find(regNo) == SetNonRefRegInFunc.end()) {
      if (I->second == XVM_SYM_REG_REF) {
        // MO_def has to be ref
        updateRefMapForRefInst(MO_def, XVM_SYM_REG_REF);
        // we need to Switch the order of use1 and use2
        MachineBasicBlock::iterator II = MI.getIterator();
        MachineFunction *MF = MBB.getParent();
        DebugLoc DL = MI.getDebugLoc();
        MachineRegisterInfo &MRI = MF->getRegInfo();
        MachineInstr *ReplaceMI = BuildMI(MBB, II, DL, TII->get(MI.getOpcode()));

        MachineOperand &NewMO_def = MI.getOperand(0);
        MachineOperand &NewMO_use1 = MI.getOperand(1);
        MachineOperand &NewMO_use2 = MI.getOperand(2);

        ReplaceMI->addOperand(NewMO_def);
        ReplaceMI->addOperand(NewMO_use2);
        ReplaceMI->addOperand(NewMO_use1);
        MBB.remove_instr(&MI);
        MRI.verifyUseLists();
        return true;
      }
    }
  }
}

static inline bool updateOrMIWithRef(MachineBasicBlock &MBB, MachineInstr &MI, const XVMInstrInfo *TII) {
  assert(MI.getNumOperands() == NUM_MO_3);
  MachineOperand &MO_def = MI.getOperand(MO_FIRST);
  MachineOperand &MO_use = MI.getOperand(MO_SECOND);
  /* if MO_def is a ref (it is determined when it is used somewhere), then it should be orref */
  Register regNo = MO_def.getReg();
  std::map<Register, unsigned char>::iterator I = MapRefRegInFunc.find(regNo);
  // Update instr if MO_def is ref
  if (I != MapRefRegInFunc.end()) {
    if (I->second == XVM_SYM_REG_REF &&
        SetNonRefRegInFunc.find(regNo) == SetNonRefRegInFunc.end()) {
      if (MI.getOpcode() == XVM::OR_ri)
        MI.setDesc(TII->get(XVM::OrRef_ri));
      else {
        MI.setDesc(TII->get(XVM::OrRef_rr));
        switchOperandUse1Use2(MBB, MI, TII);
      }
      return true;
    }
  }
  // Update instr if MO_use if ref
  regNo = MO_use.getReg();
  I = MapRefRegInFunc.find(regNo);
  if (I != MapRefRegInFunc.end()) {
    if (I->second == XVM_SYM_REG_REF &&
        SetNonRefRegInFunc.find(regNo) == SetNonRefRegInFunc.end()) {
      // MO_def has to be ref
      updateRefMapForRefInst(MO_def, XVM_SYM_REG_REF);
      if (MI.getOpcode() == XVM::OR_ri)
        MI.setDesc(TII->get(XVM::OrRef_ri));
      else
        MI.setDesc(TII->get(XVM::OrRef_rr));
      return true;
    }
  }
  return false;
}

static inline bool updateXorMIWithRef(MachineBasicBlock &MBB, MachineInstr &MI, const XVMInstrInfo *TII) {
  assert(MI.getNumOperands() == NUM_MO_3);
  MachineOperand &MO_def = MI.getOperand(MO_FIRST);
  MachineOperand &MO_use = MI.getOperand(MO_SECOND);
  /* if MO_def is a ref (it is determined when it is used somewhere), then it should be xorref */
  Register regNo = MO_def.getReg();
  std::map<Register, unsigned char>::iterator I = MapRefRegInFunc.find(regNo);
  // Update instr if MO_def is ref
  if (I != MapRefRegInFunc.end()) {
    if (I->second == XVM_SYM_REG_REF &&
        SetNonRefRegInFunc.find(regNo) == SetNonRefRegInFunc.end()) {
      if (MI.getOpcode() == XVM::XOR_ri)
        MI.setDesc(TII->get(XVM::XorRef_ri));
      else {
        MI.setDesc(TII->get(XVM::XorRef_rr));
        switchOperandUse1Use2(MBB, MI, TII);
      }
      return true;
    }
  }
  // Update instr if MO_use if ref
  regNo = MO_use.getReg();
  I = MapRefRegInFunc.find(regNo);
  if (I != MapRefRegInFunc.end() &&
      SetNonRefRegInFunc.find(regNo) == SetNonRefRegInFunc.end()) {
    if (I->second == XVM_SYM_REG_REF) {
      // MO_def has to be ref
      updateRefMapForRefInst(MO_def, XVM_SYM_REG_REF);
      if (MI.getOpcode() == XVM::XOR_ri)
        MI.setDesc(TII->get(XVM::XorRef_ri));
      else
        MI.setDesc(TII->get(XVM::XorRef_rr));
      return true;
    }
  }
  return false;
}

static inline bool updateAndMIWithRef(MachineBasicBlock &MBB, MachineInstr &MI, const XVMInstrInfo *TII) {
  assert(MI.getNumOperands() == NUM_MO_3);
  MachineOperand &MO_def = MI.getOperand(MO_FIRST);
  MachineOperand &MO_use = MI.getOperand(MO_SECOND);
  /* if MO_def is a ref (it is determined when it is used somewhere), then it should be andref */
  Register regNo = MO_def.getReg();
  std::map<Register, unsigned char>::iterator I = MapRefRegInFunc.find(regNo);
  // Update instr if MO_def is ref
  if (I != MapRefRegInFunc.end()) {
    if (I->second == XVM_SYM_REG_REF &&
        SetNonRefRegInFunc.find(regNo) == SetNonRefRegInFunc.end()) {
      if (MI.getOpcode() == XVM::AND_ri)
        MI.setDesc(TII->get(XVM::AndRef_ri));
      else {
        MI.setDesc(TII->get(XVM::AndRef_rr));
        switchOperandUse1Use2(MBB, MI, TII);
      }
      return true;
    }
  }
  // Update instr if MO_use is ref
  regNo = MO_use.getReg();
  I = MapRefRegInFunc.find(regNo);
  if (I != MapRefRegInFunc.end() &&
      SetNonRefRegInFunc.find(regNo) == SetNonRefRegInFunc.end()) {
    if (I->second == XVM_SYM_REG_REF) {
      // MO_def has to be ref
      updateRefMapForRefInst(MO_def, XVM_SYM_REG_REF);
      if (MI.getOpcode() == XVM::AND_ri)
        MI.setDesc(TII->get(XVM::AndRef_ri));
      else
        MI.setDesc(TII->get(XVM::AndRef_rr));
      return true;
    }
  }
  return false;
}

static bool updateOrXorAndMIWithRef(MachineBasicBlock &MBB, MachineInstr &MI, const XVMInstrInfo *TII) {
  switch (MI.getOpcode()) {
  case XVM::OR_ri:
  case XVM::OR_rr:
    return updateOrMIWithRef(MBB, MI, TII);
  case XVM::XOR_ri:
  case XVM::XOR_rr:
    return updateXorMIWithRef(MBB, MI, TII);
  case XVM::AND_ri:
  case XVM::AND_rr:
    return updateAndMIWithRef(MBB, MI, TII);
  default:
    return false;
  }
}

static std::map<Register, MachineOperand *> MORegReplaceMap;
static std::set<MachineInstr *> MachineInstrExceptionSet;

static void handleOffsetWithInstr(MachineInstr &MI, const char *GlobalName) {
  uint64_t SubSecOffset = GetSubSecOffsetForGlobal(GlobalName);
  MachineBasicBlock &MB = *MI.getParent();
  MachineBasicBlock::iterator II = MI.getIterator();
  MachineFunction *MF = MB.getParent();
  DebugLoc DL = MI.getDebugLoc();
  const TargetInstrInfo *TII = MF->getSubtarget().getInstrInfo();
  MachineRegisterInfo &MRI = MF->getRegInfo();

  MachineOperand &MO_def = MI.getOperand(0);
  if (SubSecOffset > 0) {
    if (SubSecOffset < ((1 << NUM_OF_IMM_BITS_ADDREF))) {
      /* Addref */
      Register VRegForAddref = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr *AddrefMI = BuildMI(MB, ++II, DL, TII->get(XVM::AddRef_ri), VRegForAddref)
                                .addReg(MO_def.getReg()).addImm(SubSecOffset);
      MachineInstrExceptionSet.insert(AddrefMI);
      MORegReplaceMap.insert(std::pair<Register, MachineOperand *>(MO_def.getReg(), &AddrefMI->getOperand(MO_FIRST)));
    } else if (SubSecOffset < ((1 << NUM_OF_IMM_BITS_MOV) -1)) {
      /* Mov */
      Register VRegForMov = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr *MovMI = BuildMI(MB, ++II, DL, TII->get(XVM::MOV_ri), VRegForMov).addImm(0);
      MachineInstrExceptionSet.insert(MovMI);
      /* Movk */
      Register VRegForMovk = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr *MovkMI = BuildMI(MB, II, DL, TII->get(XVM::MOVK_ri), VRegForMovk)
                              .addReg(VRegForMov).addImm(SubSecOffset).addImm(MOVK_SHIFT_0);
      MachineInstrExceptionSet.insert(MovkMI);
      /* Addref */
      Register VRegForAddref = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr *AddrefMI = BuildMI(MB, II, DL, TII->get(XVM::AddRef_rr), VRegForAddref)
                                .addReg(MO_def.getReg()).addReg(VRegForMovk);
      MachineInstrExceptionSet.insert(AddrefMI);
      MORegReplaceMap.insert(std::pair<Register, MachineOperand *>(MO_def.getReg(), &AddrefMI->getOperand(MO_FIRST)));
    } else if (SubSecOffset < ((1UL << NUM_OF_IMM_BITS_MOVK1) -1)) {
      /* Mov */
      Register VRegForMov = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr *MovMI = BuildMI(MB, ++II, DL, TII->get(XVM::MOV_ri), VRegForMov).addImm(0);
      MachineInstrExceptionSet.insert(MovMI);
      /* Movk */
      unsigned int  imm1 = SubSecOffset & 0X000000000000FFFF;
      Register VRegForMovk0 = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr *Movk0MI = BuildMI(MB, II, DL, TII->get(XVM::MOVK_ri), VRegForMovk0)
                                .addReg(VRegForMov).addImm(imm1).addImm(MOVK_SHIFT_0);
      MachineInstrExceptionSet.insert(Movk0MI);
      /* Movk */
      unsigned int  imm2 = (SubSecOffset & 0X00000000FFFF0000) >> NUM_OF_IMM_BITS_MOV;
      Register VRegForMovk1 = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr *Movk1MI = BuildMI(MB, II, DL, TII->get(XVM::MOVK_ri), VRegForMovk1)
                                .addReg(VRegForMovk0).addImm(imm2).addImm(MOVK_SHIFT_16);
      MachineInstrExceptionSet.insert(Movk1MI);
      /* Addref only*/
      Register VRegForAddref = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr *AddrefMI = BuildMI(MB, II, DL, TII->get(XVM::AddRef_rr), VRegForAddref)
                                .addReg(MO_def.getReg()).addReg(VRegForMovk1);
      MachineInstrExceptionSet.insert(AddrefMI);
      MORegReplaceMap.insert(std::pair<Register, MachineOperand *>(MO_def.getReg(), &AddrefMI->getOperand(MO_FIRST)));
    } else if (SubSecOffset < ((1UL << NUM_OF_IMM_BITS_MOVK2) -1)) {
      /* Mov */
      Register VRegForMov = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr *MovMI = BuildMI(MB, ++II, DL, TII->get(XVM::MOV_ri), VRegForMov).addImm(0);
      MachineInstrExceptionSet.insert(MovMI);
      /* Movk */
      unsigned int  imm1 = SubSecOffset & 0X000000000000FFFF;
      Register VRegForMovk0 = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr *MovkMIk0 = BuildMI(MB, II, DL, TII->get(XVM::MOVK_ri), VRegForMovk0)
                                .addReg(VRegForMov).addImm(imm1).addImm(MOVK_SHIFT_0);
      MachineInstrExceptionSet.insert(MovkMIk0);
      /* Movk */
      unsigned int  imm2 = (SubSecOffset & 0X00000000FFFF0000) >> NUM_OF_IMM_BITS_MOV;
      Register VRegForMovk1 = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr *Movk1MI = BuildMI(MB, II, DL, TII->get(XVM::MOVK_ri), VRegForMovk1)
                                .addReg(VRegForMovk0).addImm(imm2).addImm(MOVK_SHIFT_16);
      MachineInstrExceptionSet.insert(Movk1MI);
      /* Movk */
      unsigned int  imm3 = (SubSecOffset & 0X0000FFFF00000000) >> NUM_OF_IMM_BITS_MOVK1;
      Register VRegForMovk2 = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr *Movk2MI = BuildMI(MB, II, DL, TII->get(XVM::MOVK_ri), VRegForMovk2)
                                .addReg(VRegForMovk1).addImm(imm3).addImm(MOVK_SHIFT_32);
      MachineInstrExceptionSet.insert(Movk2MI);
      /* Addref only*/
      Register VRegForAddref = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr *AddrefMI = BuildMI(MB, II, DL, TII->get(XVM::AddRef_rr), VRegForAddref)
                                .addReg(MO_def.getReg()).addReg(VRegForMovk2);
      MachineInstrExceptionSet.insert(AddrefMI);
      MORegReplaceMap.insert(std::pair<Register, MachineOperand *>(MO_def.getReg(), &AddrefMI->getOperand(MO_FIRST)));
    } else {
      /* Mov */
      Register VRegForMov = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr *MovMI = BuildMI(MB, ++II, DL, TII->get(XVM::MOV_ri), VRegForMov).addImm(0);
      MachineInstrExceptionSet.insert(MovMI);
      /* Movk */
      unsigned int  imm1 = SubSecOffset & 0X000000000000FFFF;
      Register VRegForMovk0 = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr *Movk0MI = BuildMI(MB, II, DL, TII->get(XVM::MOVK_ri), VRegForMovk0)
                                .addReg(VRegForMov).addImm(imm1).addImm(MOVK_SHIFT_0);
      MachineInstrExceptionSet.insert(Movk0MI);
      /* Movk */
      unsigned int  imm2 = (SubSecOffset & 0X00000000FFFF0000) >> NUM_OF_IMM_BITS_MOV;
      Register VRegForMovk1 = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr *Movk1MI = BuildMI(MB, II, DL, TII->get(XVM::MOVK_ri), VRegForMovk1)
                                .addReg(VRegForMovk0).addImm(imm2).addImm(MOVK_SHIFT_16);
      MachineInstrExceptionSet.insert(Movk1MI);
      /* Movk */
      unsigned int  imm3 = (SubSecOffset & 0X0000FFFF00000000) >> NUM_OF_IMM_BITS_MOVK1;
      Register VRegForMovk2 = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr *Movk2MI = BuildMI(MB, II, DL, TII->get(XVM::MOVK_ri), VRegForMovk2)
                                .addReg(VRegForMovk1).addImm(imm3).addImm(MOVK_SHIFT_32);
      MachineInstrExceptionSet.insert(Movk2MI);
      /* Movk */
      unsigned int  imm4 = (SubSecOffset & 0XFFFF000000000000) >> NUM_OF_IMM_BITS_MOVK2;
      Register VRegForMovk3 = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr *Movk3MI = BuildMI(MB, II, DL, TII->get(XVM::MOVK_ri), VRegForMovk3)
                                .addReg(VRegForMovk2).addImm(imm4).addImm(MOVK_SHIFT_48);
      MachineInstrExceptionSet.insert(Movk3MI);
      /* Addref only*/
      Register VRegForAddref = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr *AddrefMI = BuildMI(MB, II, DL, TII->get(XVM::AddRef_rr), VRegForAddref)
                                .addReg(MO_def.getReg()).addReg(VRegForMovk3);
      MachineInstrExceptionSet.insert(AddrefMI);
      MORegReplaceMap.insert(std::pair<Register, MachineOperand *>(MO_def.getReg(), &AddrefMI->getOperand(MO_FIRST)));
    }
  }
}

static void updatePtrRegRefBasedGlobals(MachineInstr &MI) {
  switch (MI.getOpcode()) {
    case XVM::LD_global_imm64: {
      assert(MI.getNumOperands() >= NUM_MO_2);
      MachineOperand &MO_def = MI.getOperand(MO_FIRST);
      MachineOperand &MO_use = MI.getOperand(MO_SECOND);
      if (MO_use.isGlobal()) {
        const char *GlobalName = MO_use.getGlobal()->getName().data();
        LLVM_DEBUG(dbgs() << "Global:" << GlobalName << " to load.\n");
        SetRegTypeForMO(MO_def, XVM_SYM_REG_REF);
        unsigned int ptrLevel = GetPtrRegisterLevelBasedOnName(GlobalName);
        if (ptrLevel > 0) {
          MapPtrRegInFunc.insert(std::pair<Register, unsigned int>(MO_def.getReg(), ptrLevel));
        }
        // get the offset and add instructions of mov/movk/movk addref
        handleOffsetWithInstr(MI, GlobalName);
        MachineInstrExceptionSet.insert(&MI);
      }
      break;
    }
    case XVM::LDD: {
      assert(MI.getNumOperands() >= NUM_MO_2);
      MachineOperand &MO_def = MI.getOperand(MO_FIRST);
      MachineOperand &MO_use = MI.getOperand(MO_SECOND);
      if (MO_use.isReg()) {
        std::map<Register, unsigned int>::iterator I = MapPtrRegInFunc.find(MO_use.getReg());
        if (I != MapPtrRegInFunc.end() && I->second >= 1) {
          SetRegTypeForMO(MO_def, XVM_SYM_REG_REF);

          // check the flags to see if the def is a ref
          MachineOperand &MO_imm = MI.getOperand(MO_THIRD);
          if (MO_imm.isImm()) {
            int64_t imm = MO_imm.getImm();
            if (imm == 0) {
              std::map<Register, unsigned int>::iterator I1 = MapPtrRegInFunc.find(MO_use.getReg());
              if (I1 != MapPtrRegInFunc.end() && I1->second >= 1) {
                MapPtrRegInFunc.insert(std::pair<Register, unsigned int>(MO_def.getReg(), I1->second -1));
              }
            }
          }
        }
      }
      break;
    }
    case XVM::STD: {
      assert(MI.getNumOperands() >= NUM_MO_2);
      MachineOperand &MO_use2 = MI.getOperand(MO_SECOND);
      if (MO_use2.isReg()) {
        std::map<Register, unsigned int>::iterator I = MapPtrRegInFunc.find(MO_use2.getReg());
        if (I != MapPtrRegInFunc.end() && I->second >= 1) {
          SetRegTypeForMO(MO_use2, XVM_SYM_REG_REF);
        }
      }
      break;
    }
  }
}

static void checkStoreMIWithRef(MachineInstr &MI) {
  if (MI.getOpcode() == XVM::STB || MI.getOpcode() == XVM::STH ||
      MI.getOpcode() == XVM::STW || MI.getOpcode() == XVM::STD) {
    assert(MI.getNumOperands() == NUM_MO_3);
    MachineOperand &MO_use1 = MI.getOperand(MO_FIRST);
    MachineOperand &MO_use2 = MI.getOperand(MO_SECOND);
    assert(MO_use1.isUse());
    if (MO_use2.isReg()) {
      // STW killed %48:xvmgpr, killed %54:xvmgpr, 0 :: (store (s32) into %ir.arrayidx5)
      assert(MO_use2.isUse());
      // always be ref
      SetRegTypeForMO(MO_use2, XVM_SYM_REG_REF);
    } else if (MO_use2.isFI()) {
      /* Note: we might need a fix for FI scenario:
       * STB killed %6:xvmgpr, %stack.2.atomic-temp, 0 :: (store (s8) into %ir.atomic-temp)
       * It will be handled in eliminateFrameIndex.
       */
      ;
    }
    return;
  }
}

static bool updateStoreMIWithRef(MachineInstr &MI, const XVMInstrInfo *TII) {
  if (MI.getOpcode() == XVM::STD) {
    assert(MI.getNumOperands() == NUM_MO_3);
    MachineOperand &MO_use1 = MI.getOperand(MO_FIRST);
    Register regNo = MO_use1.getReg();
    std::map<Register, unsigned char>::iterator I = MapRefRegInFunc.find(regNo);
    if (I != MapRefRegInFunc.end()) {
      if (I->second == XVM_SYM_REG_REF &&
          SetNonRefRegInFunc.find(regNo) == SetNonRefRegInFunc.end()) {
        MI.setDesc(TII->get(XVM::StoreRef_ri));
        return true;
      }
    }
  }
  return false;
}

static void checkPhiMIWithRef(MachineInstr &MI) {
  if (MI.getOpcode() == XVM::PHI) {
    unsigned numOfFrom = MI.getNumOperands() / 2;
    assert(numOfFrom * NUM_MO_PER_PHI_BRANCH + 1 == MI.getNumOperands());
    MachineOperand &MO_def = MI.getOperand(MO_FIRST);
    for (unsigned idx = 0; idx < numOfFrom; idx++) {
      MachineOperand &MO_use = MI.getOperand(idx*2+1);
      setRefFlagFor2Ops(MO_def, MO_use);
    }
    return;
  }
}

static bool updatePhiMIWithRef(MachineInstr &MI, const XVMInstrInfo *TII) {
  /* No update for Phi*/
  return false;
}

static bool updateRegistersInMI(MachineInstr &MI, const XVMInstrInfo *TII) {
  SmallVector<MachineOperand, INIT_SMALL_VECTOR_SIZE> OperandsInMI;
  bool replaceOperand = false;
  if (MachineInstrExceptionSet.find(&MI) == MachineInstrExceptionSet.end()) {
    for (unsigned int i = 0; i < MI.getNumOperands(); i++) {
      MachineOperand &MO = MI.getOperand(i);
      if (MO.isReg()) {
        std::map<Register, MachineOperand *>::iterator I = MORegReplaceMap.find(MO.getReg());
        if (I == MORegReplaceMap.end()) {
          OperandsInMI.push_back(MO);
        } else {
          bool isDef = false;
          if (MO.isDef())
            isDef = true;
          MachineOperand NewMO = MachineOperand::CreateReg((I->second)->getReg(), isDef);
          // Update the ref information for replaced registers
          std::map<Register, unsigned char>::iterator IterTmp = MapRefRegInFunc.find(MO.getReg());
          if (IterTmp != MapRefRegInFunc.end()) {
              if (IterTmp->second == XVM_SYM_REG_REF &&
                SetNonRefRegInFunc.find(MO.getReg()) == SetNonRefRegInFunc.end()) {
                MapRefRegInFunc[NewMO.getReg()] = XVM_SYM_REG_REF;
              }
          }
          OperandsInMI.push_back(NewMO);
          replaceOperand = true;
        }
      } else {
        OperandsInMI.push_back(MO);
      }
    }
    if (replaceOperand) {
      MachineBasicBlock &MB = *MI.getParent();
      MachineBasicBlock::iterator II = MI.getIterator();
      MachineFunction *MF = MB.getParent();
      DebugLoc DL = MI.getDebugLoc();
      const TargetInstrInfo *TII = MF->getSubtarget().getInstrInfo();
      MachineRegisterInfo &MRI = MF->getRegInfo();
      MachineInstr *ReplaceMI = BuildMI(MB, II, DL, TII->get(MI.getOpcode()));
      for (MachineOperand pMO: OperandsInMI) {
        ReplaceMI->addOperand(pMO);
      }
      MB.remove_instr(&MI);
      MRI.verifyUseLists();
    }
  }
  return replaceOperand;
}

/**
 * Propogate the non ref registers via COPY and MOV statements
 *
 **/
static void propogateNonRefInfo(const MachineBasicBlock &MBB) {
  MachineBasicBlock::const_iterator MBBI = MBB.begin(), E = MBB.end();
  while (MBBI != E) {
    MachineBasicBlock::const_iterator NMBBI = std::next(MBBI);
    const MachineInstr &MI = *MBBI;
    MBBI = NMBBI;
    // if MO_use is in SetNonRefRegInFunc, then MO_def should be in SetNonRefRegInFunc
    if (MI.getOpcode() == XVM::COPY || MI.getOpcode() == XVM::MOV_rr) {
      assert(MI.getNumOperands() == NUM_MO_2);
      const MachineOperand &MO_def = MI.getOperand(MO_FIRST);
      const MachineOperand &MO_use = MI.getOperand(MO_SECOND);
      if (MO_def.isReg() && MO_use.isReg()) {
        if (SetNonRefRegInFunc.find(MO_use.getReg()) != SetNonRefRegInFunc.end()) {
          SetNonRefRegInFunc.insert(MO_def.getReg());
        }
      }
    }
  }
}

/**
 * Update SetNonRefRegInFunc based on function calls
 *
 * */
static void updateNonRefInfoViaCalls(const MachineBasicBlock &MBB,
                                     std::set<std::string> &FuncSet) {
  MachineBasicBlock::const_iterator MBBI = MBB.begin(), E = MBB.end();
  while (MBBI != E) {
    MachineBasicBlock::const_iterator NMBBI = std::next(MBBI);
    const MachineInstr &MI = *MBBI;
    MBBI = NMBBI;
    if (MI.getOpcode() == XVM::CALL_IMM) {
      assert(MI.getNumOperands() >= NUM_MO_3);
      const MachineOperand &MO_0 = MI.getOperand(MO_FIRST);
      const MachineOperand &MO_2 = MI.getOperand(MO_THIRD);
      if (!MO_0.isReg() && MO_0.isGlobal() &&
          MO_2.isReg() && MO_2.isImplicit() && MO_2.isDef() && !MO_2.isDead()) {
        // Function without ptr as return
        if (FuncSet.find(MO_0.getGlobal()->getName().str()) == FuncSet.end()) {
          if (MBBI == E) {
            return;
          }
          const MachineInstr &NextMI = *MBBI;
          if (NextMI.getOpcode() == XVM::ADJCALLSTACKUP) {
            // skip call stack up
            MBBI = std::next(MBBI);
            if (MBBI == E) {
              return;
            }
          }
          // save va reg from the copy with r0
          const MachineInstr &NextNextMI = *MBBI;
          if (NextNextMI.getOpcode() == XVM::COPY ||
              NextNextMI.getOpcode() == XVM::MOV_rr) {
            assert(NextNextMI.getNumOperands() == NUM_MO_2);
            const MachineOperand &MO_def = NextNextMI.getOperand(MO_FIRST);
            const MachineOperand &MO_use = NextNextMI.getOperand(MO_SECOND);
            if (MO_def.isReg() && MO_use.isReg()) {
              if (MO_use.getReg() == XVM::R0) {
                SetNonRefRegInFunc.insert(MO_def.getReg());
              }
            }
          }
        }
      }
    }
  }
}

/**
 * Find non ref registers via function calls
 * */
void XVMUpdateRefInstrForMI::FindNonRefRegInFunc(const MachineFunction &MF) {
  MachineModuleInfo &MMI = MF.getMMI();
  const Module *M = MMI.getModule();
  std::set<std::string> FuncSetWithRetPtr;

  if (M != NULL) {
    for (const Function &F : M->getFunctionList()) {
      Type *Ty = F.getReturnType();
      if (auto *PTy = dyn_cast<PointerType>(Ty)) {
        FuncSetWithRetPtr.insert(F.getName().str());
      }
    }
  }
  for (auto &MBB : MF) {
    updateNonRefInfoViaCalls(MBB, FuncSetWithRetPtr);
  }
  for (auto &MBB : MF) {
    propogateNonRefInfo(MBB);
  }
}

bool XVMUpdateRefInstrForMI::updateRegistersOfMIInMBB(MachineBasicBlock &MBB) {
  int InstNumber = 0;
  bool Modified = false;

  MachineBasicBlock::reverse_iterator MBBI = MBB.rbegin(), E = MBB.rend();
  // This needs  to be done seperately since it may cross BBs
  MBBI = MBB.rbegin(), E = MBB.rend();
  InstNumber = std::distance(MBB.begin(), MBB.end());
  for (int i = 0; i < InstNumber; i++) {
    MachineBasicBlock::reverse_iterator NMBBI = std::next(MBBI);
    MachineInstr &MI = *MBBI;
    Modified |= updateRegistersInMI(MI, TII);
    MBBI = NMBBI;
  }
  return Modified;
}

bool XVMUpdateRefInstrForMI::runOnMachineFunction(MachineFunction &MF) {
  TII = MF.getSubtarget<XVMSubtarget>().getInstrInfo();
  LLVM_DEBUG(dbgs() << "Check/update refs in fun:" << MF.getFunction().getName().data() << ".\n");

  MapMIToBeFixed.clear();
  MapRefRegInFunc.clear();
  MapPtrRegInFunc.clear();
  MORegReplaceMap.clear();
  MachineInstrExceptionSet.clear();
  SetNonRefRegInFunc.clear();
  CheckFunctionArgs(MF.getFunction());
  CheckFunctionReturn(MF.getFunction());
  FindNonRefRegInFunc(MF);

  bool Modified = false;
  // scan MBBs in MF
  for (auto &MBB : MF) {
    Modified |= scanRefInfoInMBB(MBB);
  }
  // update MBBs in MF
  for (auto &MBB : MF) {
    Modified |= updateRefInfoBasedInMBB(MBB);
  }

  for (auto &MBB : MF) {
    Modified |= checkAndUpdateOrInMBB(MBB);
    Modified |= updateRegistersOfMIInMBB(MBB);
  }

  for (auto &MBB : MF) {
    doubleCheckRefs(MBB);
  }
  Modified |= finalFixRefs();

  return Modified;
}

bool XVMUpdateRefInstrForMI::finalFixRefs(void) {
  bool Modified = false;
  std::map<MachineInstr *, MachineBasicBlock *>::iterator it;
  for (it = MapMIToBeFixed.begin(); it != MapMIToBeFixed.end(); it++) {
    MachineInstr *MI = it->first;
    MachineBasicBlock *MBB = it->second;

    MachineOperand &MO_def = MI->getOperand(MO_FIRST);
    if (!MO_def.isReg())
      return Modified;

    Register regNoDef = MO_def.getReg();
    std::map<Register, unsigned char>::iterator IForDef = MapRefRegInFunc.find(regNoDef);
    switch (MI->getOpcode()) {
    default:
      break;
    case XVM::ADD_ri: {
      // make def to be ref
      SetRegTypeForMO(MO_def, XVM_SYM_REG_REF);

      if (IForDef == MapRefRegInFunc.end()) {
        SetRegTypeForMO(MO_def, XVM_SYM_REG_REF);
      } else {
        MapRefRegInFunc[regNoDef] = XVM_SYM_REG_REF;
      }
      // change it to be AddRef_ri and
      MI->setDesc(TII->get(XVM::AddRef_ri));
      Modified = true;
      break;
    }
    case XVM::ADD_rr: {
      // make def to to ref
      SetRegTypeForMO(MO_def, XVM_SYM_REG_REF);
      Modified = true;
      // Switch the use 1 with use 2 if use 2 is a ref
      MachineOperand &MO_use2 = MI->getOperand(2);
      Register regNoUse2 = MO_use2.getReg();
      std::map<Register, unsigned char>::iterator IForUse2 = MapRefRegInFunc.find(regNoUse2);
      if (IForUse2 != MapRefRegInFunc.end()) {
        if (IForUse2->second == XVM_SYM_REG_REF &&
            SetNonRefRegInFunc.find(regNoUse2) == SetNonRefRegInFunc.end()) {
          // by create a new MI
          MachineBasicBlock::iterator II = MI->getIterator();
          MachineFunction *MF = MBB->getParent();
          DebugLoc DL = MI->getDebugLoc();
          MachineRegisterInfo &MRI = MF->getRegInfo();
          MachineInstr *ReplaceMI = BuildMI(*MBB, II, DL, TII->get(XVM::AddRef_rr));

          MachineOperand &NewMO_def = MI->getOperand(0);
          MachineOperand &NewMO_use1 = MI->getOperand(1);
          MachineOperand &NewMO_use2 = MI->getOperand(2);

          ReplaceMI->addOperand(NewMO_def);
          ReplaceMI->addOperand(NewMO_use2);
          ReplaceMI->addOperand(NewMO_use1);
          MBB->remove_instr(MI);
          MRI.verifyUseLists();
        }
      }
      break;
    }
    case XVM::AddRef_ri: {
      SetRegTypeForMO(MO_def, XVM_SYM_REG_REF);
      Modified = true;
      break;
    }
    case XVM::AddRef_rr: {
      MachineOperand &MO_use2 = MI->getOperand(MO_THIRD);
      if ((IForDef != MapRefRegInFunc.end() && IForDef->second != XVM_SYM_REG_REF) ||
           SetNonRefRegInFunc.find(regNoDef) != SetNonRefRegInFunc.end()) {
        SetRegTypeForMO(MO_def, XVM_SYM_REG_REF);
      }
      // Switch the use 1 with use 2 if use 2 is a ref
      Register regNoUse2 = MO_use2.getReg();
      std::map<Register, unsigned char>::iterator IForUse2 = MapRefRegInFunc.find(regNoUse2);
      if (IForUse2 != MapRefRegInFunc.end()) {
        if (IForUse2->second == XVM_SYM_REG_REF &&
            SetNonRefRegInFunc.find(regNoUse2) == SetNonRefRegInFunc.end()) {
          Modified = true;
          // by create a new MI
          MachineBasicBlock::iterator II = MI->getIterator();
          MachineFunction *MF = MBB->getParent();
          DebugLoc DL = MI->getDebugLoc();
          MachineRegisterInfo &MRI = MF->getRegInfo();
          MachineInstr *ReplaceMI = BuildMI(*MBB, II, DL, TII->get(XVM::AddRef_rr));

          MachineOperand &MO_def = MI->getOperand(MO_FIRST);
          MachineOperand &MO_use1 = MI->getOperand(MO_SECOND);
          MachineOperand &MO_use2 = MI->getOperand(MO_THIRD);

          ReplaceMI->addOperand(MO_def);
          ReplaceMI->addOperand(MO_use2);
          ReplaceMI->addOperand(MO_use1);
          MBB->remove_instr(MI);
          MRI.verifyUseLists();
        }
      }
      break;
    }
  }
  }
  return Modified;
}

void XVMUpdateRefInstrForMI::doubleCheckRefs(MachineBasicBlock &MBB) {
  MachineBasicBlock::iterator MBBI = MBB.begin(), E = MBB.end();
  int InstNumber = std::distance(MBB.begin(), MBB.end());
  for (int i = 0; i < InstNumber; i++) {
    MachineBasicBlock::iterator NMBBI = std::next(MBBI);
    MachineInstr &MI = *MBBI;

    if (MI.getNumOperands() < 2) {
      MBBI = NMBBI;
      continue;
    }
    MachineOperand &MO_def = MI.getOperand(MO_FIRST);
    MachineOperand &MO_use1 = MI.getOperand(MO_SECOND);
    if (!MO_def.isReg()) {
      MBBI = NMBBI;
      continue;
    }
    Register regNoDef = MO_def.getReg();
    std::map<Register, unsigned char>::iterator IForDef = MapRefRegInFunc.find(regNoDef);

    const MachineFunction *F = MI.getParent()->getParent();
    switch (MI.getOpcode()) {
    case XVM::ADD_ri: {
      MachineOperand &MO_use2 = MI.getOperand(MO_THIRD);
      if (!MO_use2.isImm()) {
        ExportFailMsg(F->getFunction(), MI.getDebugLoc(), "Error: use 2 of ADD_ri is not an imm", NULL);
        exit(1);
      }
      if (IForDef != MapRefRegInFunc.end()) {
        if (IForDef->second == XVM_SYM_REG_REF &&
            SetNonRefRegInFunc.find(regNoDef) == SetNonRefRegInFunc.end()) {
          LLVM_DEBUG(dbgs() << "To-be-fixed(ADD_ri): def is ref.\n");
          MapMIToBeFixed.insert(std::pair<MachineInstr *, MachineBasicBlock *>(&MI, &MBB));
        }
      }
      Register regNoUse1 = MO_use1.getReg();
      std::map<Register, unsigned char>::iterator IForUse1 = MapRefRegInFunc.find(regNoUse1);
      if (IForUse1 != MapRefRegInFunc.end()) {
        if (IForUse1->second == XVM_SYM_REG_REF &&
            SetNonRefRegInFunc.find(regNoUse1) == SetNonRefRegInFunc.end()) {
          LLVM_DEBUG(dbgs() << "To-be-fixed(ADD_ri): use 1 is ref.\n");
          MapMIToBeFixed.insert(std::pair<MachineInstr *, MachineBasicBlock *>(&MI, &MBB));
        }
      }
      break;
    }
    case XVM::ADD_rr: {
      MachineOperand &MO_use2 = MI.getOperand(MO_THIRD);
      if (!MO_use2.isReg()) {
        ExportFailMsg(F->getFunction(), MI.getDebugLoc(), "Error: use 2 of ADD_rr is not a reg", NULL);
        exit(1);
      }
      if (IForDef != MapRefRegInFunc.end()) {
        if (IForDef->second == XVM_SYM_REG_REF &&
            SetNonRefRegInFunc.find(regNoDef) == SetNonRefRegInFunc.end()) {
          LLVM_DEBUG(dbgs() << "To-be-fixed(ADD_rr): def is ref.\n");
          MapMIToBeFixed.insert(std::pair<MachineInstr *, MachineBasicBlock *>(&MI, &MBB));
        }
      }
      Register regNoUse1 = MO_use1.getReg();
      Register regNoUse2 = MO_use2.getReg();
      std::map<Register, unsigned char>::iterator IForUse1 = MapRefRegInFunc.find(regNoUse1);
      std::map<Register, unsigned char>::iterator IForUse2 = MapRefRegInFunc.find(regNoUse2);
      if (IForUse1 != MapRefRegInFunc.end()) {
        if (IForUse1->second == XVM_SYM_REG_REF &&
            SetNonRefRegInFunc.find(regNoUse1) == SetNonRefRegInFunc.end()) {
          if (IForUse2 != MapRefRegInFunc.end()) {
            if (IForUse2->second == XVM_SYM_REG_REF &&
                SetNonRefRegInFunc.find(regNoUse2) == SetNonRefRegInFunc.end()) {
              ExportFailMsg(F->getFunction(), MI.getDebugLoc(), "Error: both use 1 and use2 of ADD_rr are ref", NULL);
              exit(1);
            }
          }
        }
      } else {
        if (IForUse2 != MapRefRegInFunc.end()) {
          if (IForUse2->second == XVM_SYM_REG_REF &&
              SetNonRefRegInFunc.find(regNoUse2) == SetNonRefRegInFunc.end()) {
            LLVM_DEBUG(dbgs() << "To-be-fixed(ADD_ri): use 1 is not ref, use 2 is ref.\n");
            MapMIToBeFixed.insert(std::pair<MachineInstr *, MachineBasicBlock *>(&MI, &MBB));
          }
        }
      }
      break;
    }
    case XVM::SUB_ri: {
      MachineOperand &MO_use2 = MI.getOperand(MO_THIRD);
      if (!MO_use2.isImm()) {
        ExportFailMsg(F->getFunction(), MI.getDebugLoc(), "Error: use 2 of SUB_ri is not imm", NULL);
        exit(1);
      }
      if (IForDef != MapRefRegInFunc.end()) {
        if (IForDef->second == XVM_SYM_REG_REF &&
            SetNonRefRegInFunc.find(regNoDef) == SetNonRefRegInFunc.end()) {
          LLVM_DEBUG(dbgs() << "To-be-fixed(SUB_ri): def is ref.\n");
          MapMIToBeFixed.insert(std::pair<MachineInstr *, MachineBasicBlock *>(&MI, &MBB));
        }
      }
      Register regNoUse1 = MO_use1.getReg();
      std::map<Register, unsigned char>::iterator IForUse1 = MapRefRegInFunc.find(regNoUse1);
      if (IForUse1 != MapRefRegInFunc.end()) {
        if (IForUse1->second == XVM_SYM_REG_REF &&
            SetNonRefRegInFunc.find(regNoUse1) == SetNonRefRegInFunc.end()) {
          LLVM_DEBUG(dbgs() << "To-be-fixed(SUB_ri): use 1 is ref.\n");
          MapMIToBeFixed.insert(std::pair<MachineInstr *, MachineBasicBlock *>(&MI, &MBB));
        }
      }
      break;
    }
    case XVM::SUB_rr: {
      MachineOperand &MO_use2 = MI.getOperand(MO_THIRD);
      if (!MO_use2.isReg()) {
        ExportFailMsg(F->getFunction(), MI.getDebugLoc(), "Error: use 2 of SUB_rr is not a reg", NULL);
        exit(1);
      }
      if (IForDef != MapRefRegInFunc.end()) {
        if (IForDef->second == XVM_SYM_REG_REF &&
            SetNonRefRegInFunc.find(regNoDef) == SetNonRefRegInFunc.end()) {
          LLVM_DEBUG(dbgs() << "To-be-fixed(SUB_rr): def is ref.\n");
          MapMIToBeFixed.insert(std::pair<MachineInstr *, MachineBasicBlock *>(&MI, &MBB));
        }
      }
      Register regNoUse1 = MO_use1.getReg();
      std::map<Register, unsigned char>::iterator IForUse1 = MapRefRegInFunc.find(regNoUse1);
      if (IForUse1 != MapRefRegInFunc.end()) {
        if (IForUse1->second == XVM_SYM_REG_REF &&
            SetNonRefRegInFunc.find(regNoUse1) == SetNonRefRegInFunc.end()) {
          LLVM_DEBUG(dbgs() << "To-be-fixed(SUB_rr): use 1 is ref.\n");
          MapMIToBeFixed.insert(std::pair<MachineInstr *, MachineBasicBlock *>(&MI, &MBB));
        }
      }
      Register regNoUse2 = MO_use2.getReg();
      std::map<Register, unsigned char>::iterator IForUse2 = MapRefRegInFunc.find(regNoUse2);
      if (IForUse2 != MapRefRegInFunc.end()) {
        if (IForUse2->second == XVM_SYM_REG_REF &&
            SetNonRefRegInFunc.find(regNoUse2) == SetNonRefRegInFunc.end()) {
          LLVM_DEBUG(dbgs() << "To-be-fixed(SUB_rr): use 2 is ref.\n");
          MapMIToBeFixed.insert(std::pair<MachineInstr *, MachineBasicBlock *>(&MI, &MBB));
        }
      }
      break;
    }
    case XVM::SubRef_ri: {
      MachineOperand &MO_use2 = MI.getOperand(MO_THIRD);
      if (!MO_use2.isImm()) {
        ExportFailMsg(F->getFunction(), MI.getDebugLoc(), "Error: use 2 of SubRef_ri is not an imm", NULL);
        exit(1);
      }
      if ((IForDef != MapRefRegInFunc.end() && IForDef->second != XVM_SYM_REG_REF) ||
          SetNonRefRegInFunc.find(regNoDef) != SetNonRefRegInFunc.end()) {
        LLVM_DEBUG(dbgs() << "To-be-fixed(SubRef_ri): def is not ref.\n");
        MapMIToBeFixed.insert(std::pair<MachineInstr *, MachineBasicBlock *>(&MI, &MBB));
      }
      Register regNoUse1 = MO_use1.getReg();
      std::map<Register, unsigned char>::iterator IForUse1 = MapRefRegInFunc.find(regNoUse1);
      if ((IForUse1 != MapRefRegInFunc.end() && IForUse1->second != XVM_SYM_REG_REF) ||
          SetNonRefRegInFunc.find(regNoUse1) != SetNonRefRegInFunc.end()) {
          LLVM_DEBUG(dbgs() << "To-be-fixed(SubRef_ri): use 1 is not ref.\n");
          MapMIToBeFixed.insert(std::pair<MachineInstr *, MachineBasicBlock *>(&MI, &MBB));
      }
      break;
    }
    case XVM::AddRef_ri: {
      MachineOperand &MO_use2 = MI.getOperand(MO_THIRD);
      if (!MO_use2.isImm()) {
        LLVM_DEBUG(dbgs() << "To-be-fixed(AddRef_ri): use 2 is not imm.\n");
        MapMIToBeFixed.insert(std::pair<MachineInstr *, MachineBasicBlock *>(&MI, &MBB));
        exit(1);
      }
      if ((IForDef != MapRefRegInFunc.end() && IForDef->second != XVM_SYM_REG_REF) ||
          SetNonRefRegInFunc.find(regNoDef) != SetNonRefRegInFunc.end()) {
        LLVM_DEBUG(dbgs() << "To-be-fixed(AddRef_ri): def is not ref.\n");
        MapMIToBeFixed.insert(std::pair<MachineInstr *, MachineBasicBlock *>(&MI, &MBB));
      }
      Register regNoUse1 = MO_use1.getReg();
      std::map<Register, unsigned char>::iterator IForUse1 = MapRefRegInFunc.find(regNoUse1);
      if ((IForUse1 != MapRefRegInFunc.end() && IForUse1->second != XVM_SYM_REG_REF) ||
          SetNonRefRegInFunc.find(regNoUse1) != SetNonRefRegInFunc.end()) {
        LLVM_DEBUG(dbgs() << "To-be-fixed(AddRef_ri): use 1 is not ref.\n");
        MapMIToBeFixed.insert(std::pair<MachineInstr *, MachineBasicBlock *>(&MI, &MBB));
      }
      break;
    }
    case XVM::SubRef_rr: {
      MachineOperand &MO_use2 = MI.getOperand(MO_THIRD);
      if (!MO_use2.isReg()) {
        ExportFailMsg(F->getFunction(), MI.getDebugLoc(), "Error: use 2 of SubRef_rr is not a reg", NULL);
        exit(1);
      }
      if ((IForDef != MapRefRegInFunc.end() && IForDef->second != XVM_SYM_REG_REF) ||
          SetNonRefRegInFunc.find(regNoDef) != SetNonRefRegInFunc.end()) {
        LLVM_DEBUG(dbgs() << "To-be-fixed(SubRef_rr): def is not ref.\n");
        MapMIToBeFixed.insert(std::pair<MachineInstr *, MachineBasicBlock *>(&MI, &MBB));
      }
      Register regNoUse1 = MO_use1.getReg();
      std::map<Register, unsigned char>::iterator IForUse1 = MapRefRegInFunc.find(regNoUse1);
      if ((IForUse1 != MapRefRegInFunc.end() && IForUse1->second != XVM_SYM_REG_REF) ||
          SetNonRefRegInFunc.find(regNoUse1) != SetNonRefRegInFunc.end()) {
        LLVM_DEBUG(dbgs() << "To-be-fixed(SubRef_rr): use 1 is not ref.\n");
        MapMIToBeFixed.insert(std::pair<MachineInstr *, MachineBasicBlock *>(&MI, &MBB));
      }
      Register regNoUse2 = MO_use2.getReg();
      std::map<Register, unsigned char>::iterator IForUse2 = MapRefRegInFunc.find(regNoUse2);
      if (IForUse2 != MapRefRegInFunc.end()) {
        if (IForUse2->second == XVM_SYM_REG_REF &&
            SetNonRefRegInFunc.find(regNoUse2) == SetNonRefRegInFunc.end()) {
          LLVM_DEBUG(dbgs() << "To-be-fixed(SubRef_rr): use 2 is ref.\n");
          MapMIToBeFixed.insert(std::pair<MachineInstr *, MachineBasicBlock *>(&MI, &MBB));
        }
      }
      if ((IForUse1 != MapRefRegInFunc.end() && IForUse1->second == XVM_SYM_REG_REF &&
          SetNonRefRegInFunc.find(regNoUse1) == SetNonRefRegInFunc.end()) &&
          (IForUse2 != MapRefRegInFunc.end() && IForUse2->second == XVM_SYM_REG_REF &&
          SetNonRefRegInFunc.find(regNoUse2) == SetNonRefRegInFunc.end())) {
        ExportFailMsg(F->getFunction(), MI.getDebugLoc(), "Error: both use 1 and use 2 of SubRef_rr are ref", NULL);
        exit(1);
      }
      break;
    }
    case XVM::AddRef_rr: {
      MachineOperand &MO_use2 = MI.getOperand(MO_THIRD);
      if (!MO_use2.isReg()) {
        ExportFailMsg(F->getFunction(), MI.getDebugLoc(), "Error: use 2 of AddRef_rr is not a reg", NULL);
        exit(1);
      }
      if ((IForDef != MapRefRegInFunc.end() && IForDef->second != XVM_SYM_REG_REF) ||
          SetNonRefRegInFunc.find(regNoDef) != SetNonRefRegInFunc.end()) {
        LLVM_DEBUG(dbgs() << "To-be-fixed(AddRef_rr): def is not ref.\n");
        MapMIToBeFixed.insert(std::pair<MachineInstr *, MachineBasicBlock *>(&MI, &MBB));
      }
      Register regNoUse1 = MO_use1.getReg();
      std::map<Register, unsigned char>::iterator IForUse1 = MapRefRegInFunc.find(regNoUse1);
      if ((IForUse1 != MapRefRegInFunc.end() && IForUse1->second != XVM_SYM_REG_REF) ||
          SetNonRefRegInFunc.find(regNoUse1) != SetNonRefRegInFunc.end()) {
        LLVM_DEBUG(dbgs() << "To-be-fixed(AddRef_rr): use 1 is not ref.\n");
        MapMIToBeFixed.insert(std::pair<MachineInstr *, MachineBasicBlock *>(&MI, &MBB));
      }
      Register regNoUse2 = MO_use2.getReg();
      std::map<Register, unsigned char>::iterator IForUse2 = MapRefRegInFunc.find(regNoUse2);
      if (IForUse2 != MapRefRegInFunc.end()) {
        if (IForUse2->second == XVM_SYM_REG_REF &&
            SetNonRefRegInFunc.find(regNoUse2) == SetNonRefRegInFunc.end()) {
          LLVM_DEBUG(dbgs() << "\nTest To-be-fixed(AddRef_rr): use 2 is ref. Regno: " << regNoUse2 << " Reg1: " << regNoUse1 << "\n");
          MapMIToBeFixed.insert(std::pair<MachineInstr *, MachineBasicBlock *>(&MI, &MBB));
        }
      }
      break;
    }
    case XVM::MOV_rr: {
      if (MO_use1.isFI()) {
        // def has to be ref
        if ((IForDef != MapRefRegInFunc.end() && IForDef->second != XVM_SYM_REG_REF) ||
            SetNonRefRegInFunc.find(regNoDef) != SetNonRefRegInFunc.end()) {
          LLVM_DEBUG(dbgs() << "To-be-fixed(MOV_rr): def is not ref for the case of use=sp.\n");
          MapMIToBeFixed.insert(std::pair<MachineInstr *, MachineBasicBlock *>(&MI, &MBB));
        }
      } else {
        // if def is ref, then use1 is ref
        if (IForDef != MapRefRegInFunc.end()) {
          if (IForDef->second == XVM_SYM_REG_REF &&
              SetNonRefRegInFunc.find(regNoDef) == SetNonRefRegInFunc.end()) {
            // check to see if use1 is ref
            Register regNoUse1 = MO_use1.getReg();
            std::map<Register, unsigned char>::iterator IForUse1 = MapRefRegInFunc.find(regNoUse1);
            if ((IForUse1 != MapRefRegInFunc.end() && IForUse1->second != XVM_SYM_REG_REF) ||
                SetNonRefRegInFunc.find(regNoUse1) != SetNonRefRegInFunc.end()) {
              LLVM_DEBUG(dbgs() << "To-be-fixed(MOV_rr): def is ref while use is not ref.\n");
              MapMIToBeFixed.insert(std::pair<MachineInstr *, MachineBasicBlock *>(&MI, &MBB));
            }
          }
        }
      }
      break;
    }
    case XVM::MOV_ri: {
      if (!MO_use1.isImm()) {
        ExportFailMsg(F->getFunction(), MI.getDebugLoc(), "Error: use 1 of MOV_ri is not an imm", NULL);
        exit(1);
      }
      if (IForDef != MapRefRegInFunc.end()) {
        if (IForDef->second == XVM_SYM_REG_REF &&
            SetNonRefRegInFunc.find(regNoDef) == SetNonRefRegInFunc.end()) {
          MI.print(dbgs());
          LLVM_DEBUG(dbgs() << "To-be-fixed(MOV_ri): def is ref.\n");
          MapMIToBeFixed.insert(std::pair<MachineInstr *, MachineBasicBlock *>(&MI, &MBB));
        }
      }
      break;
    }
    default:
      break;
    }
    MBBI = NMBBI;
  }
}

void XVMUpdateRefInstrForMI::updatePtrRefInMBB(MachineBasicBlock &MBB) {
  MachineBasicBlock::iterator MBBI = MBB.begin();
  int InstNumber = std::distance(MBB.begin(), MBB.end());
  for (int i = 0; i < InstNumber; i++) {
    MachineBasicBlock::iterator NMBBI = std::next(MBBI);
    MachineInstr &MI = *MBBI;
    updatePtrRegRefBasedGlobals(MI);
    MBBI = NMBBI;
  }
}

/**
 * We need to consider the static recurisive case. Here is an example
 * %46:xvmgpr = ADD_ri %47:xvmgpr, 24
 *
 * %47:xvmgpr = PHI %44:xvmgpr, %bb.75, %46:xvmgpr, %bb.45
 *
 * In the above code, %47 depends on %44 and %46 while %46 depends on %47
 * Once we detect the %44 is ref, then we should conclude that both %46 and %47 are ref
 *
 * Also, we reverse scan the instructions: Phi instructions need double check. Here is an example:
 * %85:xvmgpr = LD_global_imm64 @crc32_tab
 *
 * %0:xvmgpr = PHI %85:xvmgpr, %bb.0, %3:xvmgpr, %bb.60 <-- checking of %85 is done after that of PHI
 **/
void XVMUpdateRefInstrForMI::doubleCheckPhiMIWithRef(MachineBasicBlock &MBB) {
  MachineBasicBlock::iterator MBBI = MBB.begin(), E = MBB.end();
  int InstNumber = std::distance(MBB.begin(), MBB.end());
  for (int i = 0; i < InstNumber; i++) {
    MachineBasicBlock::iterator NMBBI = std::next(MBBI);
    MachineInstr &MI = *MBBI;
    if (MI.getOpcode() != XVM::PHI) {
      MBBI = NMBBI;
      continue;
    }

    unsigned numOfFrom = MI.getNumOperands() / 2;
    assert(numOfFrom * NUM_MO_PER_PHI_BRANCH + 1 == MI.getNumOperands());
    MachineOperand &MO_def = MI.getOperand(MO_FIRST);
    Register regNoDef = MO_def.getReg();

    std::map<Register, unsigned char>::iterator IDef = MapRefRegInFunc.find(regNoDef);
    if (IDef != MapRefRegInFunc.end() && IDef->second == XVM_SYM_REG_REF) {
      // def is ref: all uses should be ref
      for (unsigned idx = 0; idx < numOfFrom; idx++) {
        MachineOperand &MO_use = MI.getOperand(idx*2+1);
        Register regNoUse = MO_use.getReg();
        std::map<Register, unsigned char>::iterator IUse = MapRefRegInFunc.find(regNoUse);
        if (IUse == MapRefRegInFunc.end() || IUse->second != XVM_SYM_REG_REF) {
          SetRegTypeForMO(MO_use, XVM_SYM_REG_REF);
          SetNonRefRegInFunc.erase(MO_use.getReg());
        }
      }
    } else {
      // if any (def and uses) is ref, then all should be ref
      bool anyIsRef = false;
      for (unsigned idx = 0; idx < numOfFrom; idx++) {
        MachineOperand &MO_use = MI.getOperand(idx*2+1);
        Register regNoUse = MO_use.getReg();
        std::map<Register, unsigned char>::iterator IUse = MapRefRegInFunc.find(regNoUse);
        if (IUse != MapRefRegInFunc.end() && IUse->second == XVM_SYM_REG_REF &&
            SetNonRefRegInFunc.find(regNoUse) == SetNonRefRegInFunc.end()) {
          anyIsRef = true;
          break;
        }
      }
      if (anyIsRef) {
        // make all reg to be ref
        SetRegTypeForMO(MO_def, XVM_SYM_REG_REF);
        for (unsigned idx = 0; idx < numOfFrom; idx++) {
          MachineOperand &MO_use = MI.getOperand(idx*2+1);
          SetRegTypeForMO(MO_use, XVM_SYM_REG_REF);
        }
      }
    }
    MBBI = NMBBI;
  }
}

static int ShiftAndGet16Bits(uint64_t num, int n) {
  return (num >> n) & 0xFFFF;
}

static void replaceImmWithMovk(MachineInstr &MI) {
  MachineBasicBlock &MB = *MI.getParent();
  MachineFunction *MF = MB.getParent();
  const TargetInstrInfo *TII = MF->getSubtarget().getInstrInfo();
  MachineRegisterInfo &MRI = MF->getRegInfo();
  DebugLoc DL = MI.getDebugLoc();
  int64_t imm32 = MI.getOperand(MO_THIRD).getImm();
  MachineBasicBlock::iterator II = MI.getIterator();

  Register ScratchReg;
  ScratchReg = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
  uint64_t MostSignificantBits = ShiftAndGet16Bits(imm32, MOST_SIGNIFICANT);
  uint64_t UpperMiddleBits = ShiftAndGet16Bits(imm32, UPPER_MIDDLE);
  uint64_t LowerMiddleBits = ShiftAndGet16Bits(imm32, LOWER_MIDDLE);
  uint64_t LeastSignificantBits = ShiftAndGet16Bits(imm32, LEAST_SIGNIFICANT);

  Register VRegForMov = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
  BuildMI(MB, II, DL, TII->get(XVM::MOV_ri), VRegForMov).addImm(0);
  Register PrevReg = VRegForMov;
  if (LeastSignificantBits) {
    Register VRegForMovk1 = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
    BuildMI(MB, II, DL, TII->get(XVM::MOVK_ri), VRegForMovk1)
            .addReg(PrevReg).addImm(LeastSignificantBits).addImm(MOVK_SHIFT_0);
    PrevReg = VRegForMovk1;
  }
  if (LowerMiddleBits) {
    Register VRegForMovk2 = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
    BuildMI(MB, II, DL, TII->get(XVM::MOVK_ri), VRegForMovk2)
            .addReg(PrevReg).addImm(LowerMiddleBits).addImm(MOVK_SHIFT_16);
    PrevReg = VRegForMovk2;
  }
  if (UpperMiddleBits) {
    Register VRegForMovk3 = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
    BuildMI(MB, II, DL, TII->get(XVM::MOVK_ri), VRegForMovk3)
            .addReg(PrevReg).addImm(UpperMiddleBits).addImm(MOVK_SHIFT_32);
    PrevReg = VRegForMovk3;
  }
  if (MostSignificantBits) {
    Register VRegForMovk4 = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
    BuildMI(MB, II, DL, TII->get(XVM::MOVK_ri), VRegForMovk4)
            .addReg(PrevReg).addImm(MostSignificantBits).addImm(MOVK_SHIFT_48);
    PrevReg = VRegForMovk4;
  }

  BuildMI(MB, II, DL, TII->get(XVM::AddRef_rr)).addReg(ScratchReg, RegState::Define)
                                               .addReg(MI.getOperand(MO_SECOND).getReg())
                                               .addReg(PrevReg);
  MI.getOperand(MO_SECOND).setReg(ScratchReg);
  MI.getOperand(MO_SECOND).setIsKill();
  MI.getOperand(MO_THIRD).setImm(0);
}

bool XVMUpdateRefInstrForMI::checkAndUpdateOrInMBB(MachineBasicBlock &MBB) {
  int InstNumber = 0;
  bool Modified = false;
  /* Note: the two passes may be merged for efficiency */
  MachineBasicBlock::reverse_iterator MBBI = MBB.rbegin(), E = MBB.rend();
  InstNumber = std::distance(MBB.begin(), MBB.end());
  for (int i = 0; i < InstNumber; i++) {
    MachineBasicBlock::reverse_iterator NMBBI = std::next(MBBI);
    MachineInstr &MI = *MBBI;
    checkOrXorAndMIWithRef(MI);
    MBBI = NMBBI;
  }
  /* update the instructions */
  MBBI = MBB.rbegin(), E = MBB.rend();
  InstNumber = std::distance(MBB.begin(), MBB.end());
  for (int i = 0; i < InstNumber; i++) {
    MachineBasicBlock::reverse_iterator NMBBI = std::next(MBBI);
    MachineInstr &MI = *MBBI;
    Modified |= updateOrXorAndMIWithRef(MBB, MI, TII);
    MBBI = NMBBI;
  }
  return Modified;
}

bool XVMUpdateRefInstrForMI::scanRefInfoInMBB(MachineBasicBlock &MBB) {
  int InstNumber = 0;
  bool Modified = false;
  updatePtrRefInMBB(MBB);

  /* Note: the two passes may be merged for efficiency */
  // reverse order
  MachineBasicBlock::reverse_iterator R_MBBI = MBB.rbegin();
  InstNumber = std::distance(MBB.begin(), MBB.end());
  for (int i = 0; i < InstNumber; i++) {
    MachineBasicBlock::reverse_iterator NMBBI = std::next(R_MBBI);
    MachineInstr &MI = *R_MBBI;
    checkSimpleMIWithRef(MI);
    checkMovMIWithRef(MI);
    checkLoadMIWithRef(MI);
    checkStoreMIWithRef(MI);
    checkAddSubMIWithRef(MI);
    checkPhiMIWithRef(MI);
    R_MBBI = NMBBI;
  }
  // normal order
  MachineBasicBlock::iterator MBBI = MBB.begin();
  InstNumber = std::distance(MBB.begin(), MBB.end());
  for (int i = 0; i < InstNumber; i++) {
    MachineBasicBlock::iterator NMBBI = std::next(MBBI);
    MachineInstr &MI = *MBBI;
    checkSimpleMIWithRef(MI);
    checkMovMIWithRef(MI);
    checkLoadMIWithRef(MI);
    checkStoreMIWithRef(MI);
    checkAddSubMIWithRef(MI);
    checkPhiMIWithRef(MI);
    MBBI = NMBBI;
  }
  return Modified;
}

bool XVMUpdateRefInstrForMI::updateRefInfoBasedInMBB(MachineBasicBlock &MBB) {
  int InstNumber = 0;
  bool Modified = false;
  updatePtrRefInMBB(MBB);
  /* Note: the two passes may be merged for efficiency */
  doubleCheckPhiMIWithRef(MBB);
  /* update the instructions */
  MachineBasicBlock::reverse_iterator MBBI = MBB.rbegin(), E = MBB.rend();
  InstNumber = std::distance(MBB.begin(), MBB.end());
  for (int i = 0; i < InstNumber; i++) {
    MachineBasicBlock::reverse_iterator NMBBI = std::next(MBBI);
    MachineInstr &MI = *MBBI;
    Modified |= updateCopyMIWithRef(MI, TII);
    Modified |= updateMovMIWithRef(MI, TII);
    Modified |= updateLoadMIWithRef(MI, TII);
    Modified |= updateStoreMIWithRef(MI, TII);
    Modified |= updateAddSubMIWithRef(MI, TII);
    // Modified |= updateOrXorAndMIWithRef(MI, TII);
    Modified |= updatePhiMIWithRef(MI, TII);
    MBBI = NMBBI;
  }

  MBBI = MBB.rbegin(), E = MBB.rend();
  InstNumber = std::distance(MBB.begin(), MBB.end());
  for (MachineBasicBlock::iterator MBBI = MBB.begin(), MBBE = MBB.end(); MBBI != MBBE; MBBI++) {
    MachineInstr &MI = *MBBI;
    switch (MI.getOpcode()) {
    case XVM::STW:
    case XVM::STH:
    case XVM::STB:
    case XVM::STD:
    case XVM::LDW_z:
    case XVM::LDH_z:
    case XVM::LDB_z:
    case XVM::LDW:
    case XVM::LDH:
    case XVM::LDB:
    case XVM::LDD:
      if (MI.getOperand(MO_THIRD).isImm()) {
        int64_t imm32 = MI.getOperand(MO_THIRD).getImm();
        if (imm32 > MAX_IMM || imm32 < -(MAX_IMM + 1)) {
          if (MI.getOperand(MO_SECOND).isFI()) {
            MachineFunction *MF = MBB.getParent();
            const Function &F = MF->getFunction();
            ExportFailMsg(F.getFunction(), MI.getDebugLoc(), "Error: Max stack size (1024) reached", (void*)&imm32);
            exit(1);
          }
          Modified = true;
          replaceImmWithMovk(MI);
        }
      }
    default:
      continue;
    }
  }
  
  return Modified;
}

INITIALIZE_PASS(XVMUpdateRefInstrForMI, "xvm-Ref-Determine-pseudo",
                XVM_REF_DETERMINE_NAME, false, false)
namespace llvm {
FunctionPass *createXVMUpdateRefInstrForMIPass() { return new XVMUpdateRefInstrForMI(); }
}

#endif
