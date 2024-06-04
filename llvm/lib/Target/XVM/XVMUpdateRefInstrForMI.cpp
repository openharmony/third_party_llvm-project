#ifdef XVM_DYLIB_MODE

#include "XVM.h"
#include "XVM_def.h"
#include "XVMInstrInfo.h"
#include "XVMTargetMachine.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "MCTargetDesc/XVMInstPrinter.h"
#include "llvm/CodeGen/MachineModuleInfo.h"

#define DEBUG_TYPE "xvm-ref-trace"

using namespace llvm;

#define XVM_REF_DETERMINE_NAME "XVM pseudo instruction ref determine pass"
#define XVM_SYM_REG_NON_REF 0b00000000
#define XVM_SYM_REG_REF     0b00000001

namespace {
class XVMUpdateRefInstrForMI : public MachineFunctionPass {
public:
  static char ID;
  const XVMInstrInfo * TII = nullptr;
  XVMUpdateRefInstrForMI() : MachineFunctionPass(ID) {
    initializeXVMUpdateRefInstrForMIPass(*PassRegistry::getPassRegistry());
  }
  bool runOnMachineFunction(MachineFunction &MF) override;
  StringRef getPassName() const override { return XVM_REF_DETERMINE_NAME; }
private:
  bool updateRefInfoInMBB(MachineBasicBlock &MBB);
  void updatePtrRefInMBB(MachineBasicBlock &MBB);
  void FindNonRefRegInFunc(const MachineFunction &MF);
};
  char XVMUpdateRefInstrForMI::ID = 0;
}

static std::map<Register, unsigned char> MapRefRegInFunc;
static std::map<Register, unsigned int> MapPtrRegInFunc;
static std::set<Register> SetNonRefRegInFunc;

static void CheckFunctionReturn(Function & F) {
  Type *Ty = F.getReturnType();
  /* Return is always r0 for xvm */
  if (auto *PTy = dyn_cast<PointerType>(Ty)) {
    MapRefRegInFunc.insert(std::pair<Register, unsigned char>(XVM::R0, XVM_SYM_REG_REF));
 // } else if (Ty->isIntegerTy(64) || Ty->isIntegerTy(32) || Ty->isIntegerTy(16) || Ty->isIntegerTy(8)  || Ty->isIntegerTy(1)) {
   } else if (Ty->isIntegerTy()) {
    MapRefRegInFunc.insert(std::pair<Register, unsigned char>(XVM::R0, XVM_SYM_REG_NON_REF));
  //} else if (Ty->isVoidTy() || Ty->isIntegerTy()){
  } else if (Ty->isVoidTy()){
    ;
  } else {
    llvm_unreachable("Invalid return type");
  }
}

static Register inline getPhysicalRegister(unsigned index) {
    switch(index) {
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

static void CheckFunctionArgs(Function & F) {
  int idx = 0;
  /* Here we assume r0, ..., r5 are the registers for input parameters
   * We only need to check the register ones: for others in stack,
   * it is handled with check/update steps followed.
   */
  for (Function::const_arg_iterator I = F.arg_begin(), E = F.arg_end(); I != E; ++I) {
    if (idx > 5) {
      break;
    }
    Type *Ty = I->getType();
    std::string regName = "r" + itostr(idx);
    if (auto *PTy = dyn_cast<PointerType>(Ty)) {
      LLVM_DEBUG(dbgs() << "arg[" << idx << "]=" << I->getName().data() << ' is ref.\n');
      if (!I->hasAttribute(Attribute::StructRet)) {
        MapRefRegInFunc.insert(std::pair<Register, unsigned char>(getPhysicalRegister(idx), XVM_SYM_REG_REF));
      } else {
        // R6 is used to pass the sret return
        MapRefRegInFunc.insert(std::pair<Register, unsigned char>(getPhysicalRegister(6), XVM_SYM_REG_REF));
      }
    } else if (Ty->isIntegerTy()) {
      MapRefRegInFunc.insert(std::pair<Register, unsigned char>(getPhysicalRegister(idx), XVM_SYM_REG_NON_REF));
      LLVM_DEBUG(dbgs() << "arg[" << idx << "]=" << I->getName().data() << ' is not ref.\n');
    } else {
      llvm_unreachable("Invalid param type");
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
            MapRefRegInFunc.insert(std::pair<Register, unsigned char>(MO_def.getReg(), XVM_SYM_REG_NON_REF));
        } else {
            MapRefRegInFunc.insert(std::pair<Register, unsigned char>(MO_def.getReg(),  I->second));
        }
    }
}

static inline void updateRefMapForRefInst(MachineOperand &MO_use, unsigned char flag) {
  std::map<Register, unsigned char>::iterator I1 = MapRefRegInFunc.find(MO_use.getReg());
  if (I1 == MapRefRegInFunc.end()) {
    MapRefRegInFunc.insert(std::pair<Register, unsigned char>(MO_use.getReg(), flag));
  }
  else
    I1->second = flag;
}

static void checkSimpleMIWithRef(MachineInstr &MI) {
  if (MI.getOpcode() == XVM::COPY) {
    assert(MI.getNumOperands() == 2);
    MachineOperand &MO_def = MI.getOperand(0);
    MachineOperand &MO_use = MI.getOperand(1);
    setRefFlagFor2Ops(MO_def, MO_use);
  }
}

static bool updateCopyMIWithRef(MachineInstr &MI, const XVMInstrInfo * TII) {
  /* No update for Copy */
  return false;
}

static void checkMovMIWithRef(MachineInstr &MI) {
  if (MI.getOpcode() == XVM::MOV_rr) {
    assert(MI.getNumOperands() == 2);
    MachineOperand &MO_def = MI.getOperand(0);
    MachineOperand &MO_use = MI.getOperand(1);
    setRefFlagFor2Ops(MO_def, MO_use);
  }
  if (MI.getOpcode() == XVM::MOV_ri) {
    assert(MI.getNumOperands() == 2);
    MachineOperand &MO_def = MI.getOperand(0);
    MapRefRegInFunc.insert(std::pair<Register, unsigned char>(MO_def.getReg(), XVM_SYM_REG_NON_REF));
  }
}

static bool updateMovMIWithRef(MachineInstr &MI, const XVMInstrInfo * TII) {
  /* No update for Move */
  return false;
}

static void checkLoadMIWithRef(MachineInstr &MI) {
  if (MI.getOpcode() == XVM::LDB   || MI.getOpcode() == XVM::LDH   || MI.getOpcode() == XVM::LDW   ||
      MI.getOpcode() == XVM::LDB_z || MI.getOpcode() == XVM::LDH_z || MI.getOpcode() == XVM::LDW_z ||
      MI.getOpcode() == XVM::LDD) {
    assert(MI.getNumOperands() == 3);
    MachineOperand &MO_def = MI.getOperand(0);
    MachineOperand &MO_use = MI.getOperand(1);
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
        MapRefRegInFunc.insert(std::pair<Register, unsigned char>(MO_use.getReg(), XVM_SYM_REG_REF));
    }
  }

  if (MI.getOpcode() == XVM::LD_global_imm64) {
    assert(MI.getNumOperands() == 2);
    MachineOperand &MO_def = MI.getOperand(0);
    MachineOperand &MO_use = MI.getOperand(1);
    if (MO_use.isGlobal()) {
      LLVM_DEBUG(dbgs() << "Global:" << MO_use.getGlobal()->getName().data() << ' to load.\n');
      MapRefRegInFunc.insert(std::pair<Register, unsigned char>(MO_def.getReg(), XVM_SYM_REG_REF));
    }
  }
}

static bool updateLoadMIWithRef(MachineInstr &MI, const XVMInstrInfo * TII) {
  if (MI.getOpcode() == XVM::LDD) {
    assert(MI.getNumOperands() == 3);
    MachineOperand &MO_def = MI.getOperand(0);
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
      MI.getOpcode() == XVM::SUB_ri || MI.getOpcode() == XVM::SUB_rr ) {
    assert(MI.getNumOperands() == 3);
    MachineOperand &MO_def = MI.getOperand(0);
    MachineOperand &MO_use = MI.getOperand(1);
    setRefFlagFor2Ops(MO_def, MO_use);
    return;
  }
}

static void checkOrXorAndMIWithRef(MachineInstr &MI) {
  if (MI.getOpcode() == XVM::OR_ri ||
      MI.getOpcode() == XVM::XOR_ri||
      MI.getOpcode() == XVM::AND_ri) {
    assert(MI.getNumOperands() == 3);
    MachineOperand &MO_def = MI.getOperand(0);
    MachineOperand &MO_use = MI.getOperand(1);
    setRefFlagFor2Ops(MO_def, MO_use);
    return;
  }
  if (MI.getOpcode() == XVM::OR_rr ||
      MI.getOpcode() == XVM::XOR_rr||
      MI.getOpcode() == XVM::AND_rr) {
    assert(MI.getNumOperands() == 3);
    MachineOperand &MO_def = MI.getOperand(0);
    MachineOperand &MO_use = MI.getOperand(1);
    setRefFlagFor2Ops(MO_def, MO_use);
    // MO_use2 should not be ref;
    MachineOperand &MO_use2 = MI.getOperand(2);
    updateRefMapForRefInst(MO_use2, XVM_SYM_REG_NON_REF);
    return;
  }
}

static inline bool updateAddSubWithSubAddForImm(MachineInstr &MI, const XVMInstrInfo * TII) {
  if (MI.getOpcode() == XVM::ADD_ri || MI.getOpcode() == XVM::SUB_ri) {
    assert(MI.getOperand(2).isImm());
    int64_t imm = MI.getOperand(2).getImm();
    if (imm < 0) {
      imm = -imm;
      MI.getOperand(2).setImm(imm);
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

static inline void updateAddRiWithRef(MachineInstr &MI, const XVMInstrInfo * TII) {
  assert(MI.getOperand(2).isImm());
  int64_t imm = MI.getOperand(2).getImm();
  if (imm < 0) {
    imm = -imm;
    MI.getOperand(2).setImm(imm);
    MI.setDesc(TII->get(XVM::SubRef_ri));
  } else {
    MI.setDesc(TII->get(XVM::AddRef_ri));
  }
}

static inline bool updateAddMIWithRef(MachineInstr &MI, const XVMInstrInfo * TII) {
  assert(MI.getNumOperands() == 3);
  MachineOperand &MO_def = MI.getOperand(0);
  MachineOperand &MO_use = MI.getOperand(1);
  /* if MO_def is a ref (it is determined when it is used somewhere), then it should be addref */
  Register regNo = MO_def.getReg();
  std::map<Register, unsigned char>::iterator I = MapRefRegInFunc.find(regNo);
  // Update instr if MO_def is ref
  if (I != MapRefRegInFunc.end()) {
      if (I->second == XVM_SYM_REG_REF &&
          SetNonRefRegInFunc.find(regNo) == SetNonRefRegInFunc.end()) {
        // Update MO_use to be ref if MO_def is ref
        updateRefMapForRefInst(MO_use, XVM_SYM_REG_REF);
        if(MI.getOpcode() == XVM::ADD_ri)
          updateAddRiWithRef(MI, TII);
        else
          MI.setDesc(TII->get(XVM::AddRef_rr));
        return true;
      }
  }
  // Update instr if MO_use if ref
  regNo = MO_use.getReg();
  I = MapRefRegInFunc.find(regNo);
  if (I != MapRefRegInFunc.end()) {
    if (I->second == XVM_SYM_REG_REF &&
        SetNonRefRegInFunc.find(regNo) == SetNonRefRegInFunc.end()) {
        if(MI.getOpcode() == XVM::ADD_ri)
          updateAddRiWithRef(MI, TII);
        else
          MI.setDesc(TII->get(XVM::AddRef_rr));
        return true;
      }
  }
  return false;
}

static inline bool updateSubriMIWithRef(MachineInstr &MI, const XVMInstrInfo * TII) {
  assert(MI.getNumOperands() == 3);
  MachineOperand &MO_def = MI.getOperand(0);
  MachineOperand &MO_use = MI.getOperand(1);
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
        if(MI.getOpcode() == XVM::SUB_ri)
          MI.setDesc(TII->get(XVM::SubRef_ri));
        else
          MI.setDesc(TII->get(XVM::SubRef_rr));
        return true;
      }
  }
  return false;
}

static inline bool updateSubrrMIWithRef(MachineInstr &MI, const XVMInstrInfo * TII) {
  assert(MI.getNumOperands() == 3);
  MachineOperand &MO_def = MI.getOperand(0);
  MachineOperand &MO_use1 = MI.getOperand(1);
  MachineOperand &MO_use2 = MI.getOperand(2);
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
  // Update add/sub with sub/add if negative imm


static bool updateAddSubMIWithRef(MachineInstr &MI, const XVMInstrInfo * TII) {
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

static inline bool updateOrMIWithRef(MachineInstr &MI, const XVMInstrInfo * TII) {
  assert(MI.getNumOperands() == 3);
  MachineOperand &MO_def = MI.getOperand(0);
  MachineOperand &MO_use = MI.getOperand(1);
  /* if MO_def is a ref (it is determined when it is used somewhere), then it should be orref */
  Register regNo = MO_def.getReg();
  std::map<Register, unsigned char>::iterator I = MapRefRegInFunc.find(regNo);
  // Update instr if MO_def is ref
  if (I != MapRefRegInFunc.end()) {
      if (I->second == XVM_SYM_REG_REF &&
          SetNonRefRegInFunc.find(regNo) == SetNonRefRegInFunc.end()) {
        // Update MO_use to be ref if MO_def is ref
        updateRefMapForRefInst(MO_use, XVM_SYM_REG_REF);
        if(MI.getOpcode() == XVM::OR_ri)
          MI.setDesc(TII->get(XVM::OrRef_ri));
        else
          MI.setDesc(TII->get(XVM::OrRef_rr));
        return true;
      }
  }
  // Update instr if MO_use if ref
  regNo = MO_use.getReg();
  I = MapRefRegInFunc.find(regNo);
  if (I != MapRefRegInFunc.end()) {
    if (I->second == XVM_SYM_REG_REF &&
          SetNonRefRegInFunc.find(regNo) == SetNonRefRegInFunc.end()) {
        if(MI.getOpcode() == XVM::OR_ri)
          MI.setDesc(TII->get(XVM::OrRef_ri));
        else
          MI.setDesc(TII->get(XVM::OrRef_rr));
        return true;
      }
  }
  return false;
}

static inline bool updateXorMIWithRef(MachineInstr &MI, const XVMInstrInfo * TII) {
  assert(MI.getNumOperands() == 3);
  MachineOperand &MO_def = MI.getOperand(0);
  MachineOperand &MO_use = MI.getOperand(1);
  /* if MO_def is a ref (it is determined when it is used somewhere), then it should be xorref */
  Register regNo = MO_def.getReg();
  std::map<Register, unsigned char>::iterator I = MapRefRegInFunc.find(regNo);
  // Update instr if MO_def is ref
  if (I != MapRefRegInFunc.end()) {
      if (I->second == XVM_SYM_REG_REF &&
          SetNonRefRegInFunc.find(regNo) == SetNonRefRegInFunc.end()) {
        // Update MO_use to be ref if MO_def is ref
        updateRefMapForRefInst(MO_use, XVM_SYM_REG_REF);
        if(MI.getOpcode() == XVM::XOR_ri)
          MI.setDesc(TII->get(XVM::XorRef_ri));
        else
          MI.setDesc(TII->get(XVM::XorRef_rr));
        return true;
      }
  }
  // Update instr if MO_use if ref
  regNo = MO_use.getReg();
  I = MapRefRegInFunc.find(regNo);
  if (I != MapRefRegInFunc.end() &&
          SetNonRefRegInFunc.find(regNo) == SetNonRefRegInFunc.end()) {
    if (I->second == XVM_SYM_REG_REF) {
        if(MI.getOpcode() == XVM::XOR_ri)
          MI.setDesc(TII->get(XVM::XorRef_ri));
        else
          MI.setDesc(TII->get(XVM::XorRef_rr));
        return true;
      }
  }
  return false;
}

static inline bool updateAndMIWithRef(MachineInstr &MI, const XVMInstrInfo * TII) {
  assert(MI.getNumOperands() == 3);
  MachineOperand &MO_def = MI.getOperand(0);
  MachineOperand &MO_use = MI.getOperand(1);
  /* if MO_def is a ref (it is determined when it is used somewhere), then it should be andref */
  Register regNo = MO_def.getReg();
  std::map<Register, unsigned char>::iterator I = MapRefRegInFunc.find(regNo);
  // Update instr if MO_def is ref
  if (I != MapRefRegInFunc.end()) {
      if (I->second == XVM_SYM_REG_REF &&
          SetNonRefRegInFunc.find(regNo) == SetNonRefRegInFunc.end()) {
        // Update MO_use to be ref if MO_def is ref
        updateRefMapForRefInst(MO_use, XVM_SYM_REG_REF);
        if(MI.getOpcode() == XVM::AND_ri)
          MI.setDesc(TII->get(XVM::AndRef_ri));
        else
          MI.setDesc(TII->get(XVM::AndRef_rr));
        return true;
      }
  }
  // Update instr if MO_use if ref
  regNo = MO_use.getReg();
  I = MapRefRegInFunc.find(regNo);
  if (I != MapRefRegInFunc.end() &&
          SetNonRefRegInFunc.find(regNo) == SetNonRefRegInFunc.end()) {
    if (I->second == XVM_SYM_REG_REF) {
        if(MI.getOpcode() == XVM::AND_ri)
          MI.setDesc(TII->get(XVM::AndRef_ri));
        else
          MI.setDesc(TII->get(XVM::AndRef_rr));
        return true;
      }
  }
  return false;
}

static bool updateOrXorAndMIWithRef(MachineInstr &MI, const XVMInstrInfo * TII) {
  switch (MI.getOpcode())
  {
  case XVM::OR_ri:
  case XVM::OR_rr:
    return updateOrMIWithRef(MI, TII);
  case XVM::XOR_ri:
  case XVM::XOR_rr:
    return updateXorMIWithRef(MI, TII);
  case XVM::AND_ri:
  case XVM::AND_rr:
    return updateAndMIWithRef(MI, TII);
  default:
    return false;
  }
}

static std::map<Register, MachineOperand *> MachineOperandRegisterReplacementMap;
static std::set<MachineInstr *> MachineInstrExceptionSet;
/** mov rd #simm (16 bits)
  * movk rd, #uimm (16 bits), #shift (0:no 1:16bits 2:32bits 3:48bits)
  * addref rd, rn, #uimm (14bits)
  */
#define NUM_OF_IMM_BITS_ADDREF 14
#define NUM_OF_IMM_BITS_MOV    16
#define NUM_OF_IMM_BITS_MOVK1  32
#define NUM_OF_IMM_BITS_MOVK2  48
#define NUM_OF_IMM_BITS_MOVK3  64

static void handleOffsetWithInstr(MachineInstr &MI, const char * GlobalName) {
  uint64_t SubSecOffset = GetSubSecOffsetForGlobal(GlobalName);
  MachineBasicBlock &MB = *MI.getParent();
  MachineBasicBlock::iterator II = MI.getIterator();
  MachineFunction * MF = MB.getParent();
  DebugLoc DL = MI.getDebugLoc();
  const TargetInstrInfo *TII = MF->getSubtarget().getInstrInfo();
  MachineRegisterInfo &MRI = MF->getRegInfo();

  MachineOperand &MO_def = MI.getOperand(0);
  if (SubSecOffset > 0) {
    if (SubSecOffset < ((1 << NUM_OF_IMM_BITS_ADDREF))) {
      /* Addref */
      Register VRegForAddref = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr * AddrefMI = BuildMI(MB, ++II, DL, TII->get(XVM::AddRef_ri), VRegForAddref)
                            .addReg(MO_def.getReg()).addImm(SubSecOffset);
      MachineInstrExceptionSet.insert(AddrefMI);
      MachineOperandRegisterReplacementMap.insert(std::pair<Register, MachineOperand *>(MO_def.getReg(), &AddrefMI->getOperand(0)));
    } else if (SubSecOffset < ((1 << NUM_OF_IMM_BITS_MOV) -1)) {
      /* Mov */
      Register VRegForMov = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr * MovMI = BuildMI(MB, ++II, DL, TII->get(XVM::MOV_ri), VRegForMov)
                            .addImm(SubSecOffset);
      MachineInstrExceptionSet.insert(MovMI);
      /* Addref */
      Register VRegForAddref = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr * AddrefMI = BuildMI(MB, II, DL, TII->get(XVM::AddRef_rr), VRegForAddref)
                            .addReg(MO_def.getReg()).addReg(VRegForMov);
      MachineInstrExceptionSet.insert(AddrefMI);
      MachineOperandRegisterReplacementMap.insert(std::pair<Register, MachineOperand *>(MO_def.getReg(), &AddrefMI->getOperand(0)));
    } else if (SubSecOffset < ((1 << NUM_OF_IMM_BITS_MOVK1) -1)) {
      /* Mov */
      unsigned int  imm1 = SubSecOffset & 0X000000000000FFFF;
      Register VRegForMov = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr * MovMI = BuildMI(MB, ++II, DL, TII->get(XVM::MOV_ri), VRegForMov)
                            .addImm(imm1);
      MachineInstrExceptionSet.insert(MovMI);
      /* Movk */
      unsigned int  imm2 = (SubSecOffset & 0X00000000FFFF0000)>>16;
      Register VRegForMovk1 = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr * MovkMI = BuildMI(MB, II, DL, TII->get(XVM::MOVK_ri), VRegForMovk1)
                            .addReg(VRegForMov).addImm(imm2).addImm(1);
      MachineInstrExceptionSet.insert(MovkMI);
      /* Addref only*/
      Register VRegForAddref = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr * AddrefMI = BuildMI(MB, II, DL, TII->get(XVM::AddRef_rr), VRegForAddref)
                            .addReg(MO_def.getReg()).addReg(VRegForMovk1);
      MachineInstrExceptionSet.insert(AddrefMI);
      MachineOperandRegisterReplacementMap.insert(std::pair<Register, MachineOperand *>(MO_def.getReg(), &AddrefMI->getOperand(0)));
    } else if (SubSecOffset < ((1 << NUM_OF_IMM_BITS_MOVK2) -1)) {
      /* Mov */
      unsigned int  imm1 = SubSecOffset & 0X000000000000FFFF;
      Register VRegForMov = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr * MovMI = BuildMI(MB, ++II, DL, TII->get(XVM::MOV_ri), VRegForMov)
                            .addImm(imm1);
      MachineInstrExceptionSet.insert(MovMI);
      /* Movk */
      unsigned int  imm2 = (SubSecOffset & 0X00000000FFFF0000)>>16;
      Register VRegForMovk1 = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr * Movk1MI = BuildMI(MB, II, DL, TII->get(XVM::MOVK_ri), VRegForMovk1)
                            .addReg(VRegForMov).addImm(imm2).addImm(1);
      MachineInstrExceptionSet.insert(Movk1MI);
      /* Movk */
      unsigned int  imm3 = (SubSecOffset & 0X0000FFFF00000000)>>32;
      Register VRegForMovk2 = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr * Movk2MI = BuildMI(MB, II, DL, TII->get(XVM::MOVK_ri), VRegForMovk2)
                            .addReg(VRegForMovk1).addImm(imm3).addImm(2);
      MachineInstrExceptionSet.insert(Movk2MI);
      /* Addref only*/
      Register VRegForAddref = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr * AddrefMI = BuildMI(MB, II, DL, TII->get(XVM::AddRef_rr), VRegForAddref)
                            .addReg(MO_def.getReg()).addReg(VRegForMovk2);
      MachineInstrExceptionSet.insert(AddrefMI);
      MachineOperandRegisterReplacementMap.insert(std::pair<Register, MachineOperand *>(MO_def.getReg(), &AddrefMI->getOperand(0)));
    } else if (SubSecOffset < ((1 << NUM_OF_IMM_BITS_MOVK3) -1)) {
      /* Mov */
      unsigned int  imm1 = SubSecOffset & 0X000000000000FFFF;
      Register VRegForMov = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr * MovMI = BuildMI(MB, ++II, DL, TII->get(XVM::MOV_ri), VRegForMov)
                            .addImm(imm1);
      MachineInstrExceptionSet.insert(MovMI);
      /* Movk */
      unsigned int  imm2 = (SubSecOffset & 0X00000000FFFF0000)>>16;
      Register VRegForMovk1 = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr * Movk1MI = BuildMI(MB, II, DL, TII->get(XVM::MOVK_ri), VRegForMovk1)
                            .addReg(VRegForMov).addImm(imm2).addImm(1);
      MachineInstrExceptionSet.insert(Movk1MI);
      /* Movk */
      unsigned int  imm3 = (SubSecOffset & 0X0000FFFF00000000)>>32;
      Register VRegForMovk2 = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr * Movk2MI = BuildMI(MB, II, DL, TII->get(XVM::MOVK_ri), VRegForMovk2)
                            .addReg(VRegForMovk1).addImm(imm3).addImm(2);
      MachineInstrExceptionSet.insert(Movk2MI);
      /* Movk */
      unsigned int  imm4 = (SubSecOffset & 0XFFFF000000000000)>>48;
      Register VRegForMovk3 = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr * Movk3MI = BuildMI(MB, II, DL, TII->get(XVM::MOVK_ri), VRegForMovk3)
                            .addReg(VRegForMovk2).addImm(imm4).addImm(3);
      MachineInstrExceptionSet.insert(Movk3MI);
      /* Addref only*/
      Register VRegForAddref = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
      MachineInstr * AddrefMI = BuildMI(MB, II, DL, TII->get(XVM::AddRef_rr), VRegForAddref)
                            .addReg(MO_def.getReg()).addReg(VRegForMovk3);
      MachineInstrExceptionSet.insert(AddrefMI);
      MachineOperandRegisterReplacementMap.insert(std::pair<Register, MachineOperand *>(MO_def.getReg(), &AddrefMI->getOperand(0)));
    }
  }
}

static void updatePtrRegRefBasedGlobals(MachineInstr &MI) {
  switch(MI.getOpcode()) {
    case XVM::LD_global_imm64: {
      assert(MI.getNumOperands() >= 2);
      MachineOperand &MO_def = MI.getOperand(0);
      MachineOperand &MO_use = MI.getOperand(1);
      if (MO_use.isGlobal()) {
        const char * GlobalName = MO_use.getGlobal()->getName().data();
        LLVM_DEBUG(dbgs() << "Global:" << GlobalName << ' to load.\n');
        MapRefRegInFunc.insert(std::pair<Register, unsigned char>(MO_def.getReg(), XVM_SYM_REG_REF));
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
      assert(MI.getNumOperands() >= 2);
      MachineOperand &MO_def = MI.getOperand(0);
      MachineOperand &MO_use = MI.getOperand(1);
      if (MO_use.isReg()) {
        std::map<Register, unsigned int>::iterator I = MapPtrRegInFunc.find(MO_use.getReg());
        if (I != MapPtrRegInFunc.end() && I->second >= 1) {
          MapRefRegInFunc.insert(std::pair<Register, unsigned char>(MO_def.getReg(), XVM_SYM_REG_REF));

          // check the flags to see if the def is a ref
          MachineOperand &MO_imm = MI.getOperand(2);
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
      assert(MI.getNumOperands() >= 2);
      MachineOperand &MO_use1 = MI.getOperand(0);
      MachineOperand &MO_use2 = MI.getOperand(1);
      if (MO_use2.isReg()) {
        std::map<Register, unsigned int>::iterator I = MapPtrRegInFunc.find(MO_use2.getReg());
        if (I != MapPtrRegInFunc.end() && I->second >= 1) {
          MapRefRegInFunc.insert(std::pair<Register, unsigned char>(MO_use1.getReg(), XVM_SYM_REG_REF));
        }
      }
      break;
    }
  }
}

static void checkStoreMIWithRef(MachineInstr &MI) {
  if (MI.getOpcode() == XVM::STB || MI.getOpcode() == XVM::STH ||
      MI.getOpcode() == XVM::STW || MI.getOpcode() == XVM::STD) {
    assert(MI.getNumOperands() == 3);
    MachineOperand &MO_use1 = MI.getOperand(0);
    MachineOperand &MO_use2 = MI.getOperand(1);
    assert(MO_use1.isUse());
    if (MO_use2.isReg()) {
      // STW killed %48:xvmgpr, killed %54:xvmgpr, 0 :: (store (s32) into %ir.arrayidx5)
      assert(MO_use2.isUse());
      // always be ref
      MapRefRegInFunc.insert(std::pair<Register, unsigned char>(MO_use2.getReg(), XVM_SYM_REG_REF));
    } else if (MO_use2.isFI()) {
      /* FIXME: we might need a fix for FI scenario:
         STB killed %6:xvmgpr, %stack.2.atomic-temp, 0 :: (store (s8) into %ir.atomic-temp)
         It will be handled in eliminateFrameIndex.
      */
      ;
    }
    return;
  }
}

static bool updateStoreMIWithRef(MachineInstr &MI, const XVMInstrInfo * TII) {
  if (MI.getOpcode() == XVM::STD) {
    assert(MI.getNumOperands() == 3);
    MachineOperand &MO_use1 = MI.getOperand(0);
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
    assert(numOfFrom * 2 + 1 == MI.getNumOperands());
    MachineOperand &MO_def = MI.getOperand(0);
    for(unsigned idx = 0; idx < numOfFrom; idx++) {
        MachineOperand &MO_use = MI.getOperand(idx*2+1);
        setRefFlagFor2Ops(MO_def, MO_use);
    }
    return;
  }
}

static bool updatePhiMIWithRef(MachineInstr &MI, const XVMInstrInfo * TII) {
  /* No update for Phi*/
  return false;
}

static bool updateRegistersInMI(MachineInstr &MI, const XVMInstrInfo * TII) {
  SmallVector<MachineOperand, 4> OperandsInMI;
  bool replaceOperand = false;
  if ( MachineInstrExceptionSet.find(&MI) == MachineInstrExceptionSet.end()) {
    for(unsigned int i = 0; i < MI.getNumOperands(); i++) {
      MachineOperand &MO = MI.getOperand(i);
      if (MO.isReg()) {
        std::map<Register, MachineOperand *>::iterator I = MachineOperandRegisterReplacementMap.find(MO.getReg());
        if (I == MachineOperandRegisterReplacementMap.end()) {
          OperandsInMI.push_back(MO);
        } else {
          bool isDef = false;
          if (MO.isDef())
            isDef = true;
          MachineOperand NewMO = MachineOperand::CreateReg((I->second)->getReg(), isDef);
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
      MachineFunction * MF = MB.getParent();
      DebugLoc DL = MI.getDebugLoc();
      const TargetInstrInfo *TII = MF->getSubtarget().getInstrInfo();
      MachineRegisterInfo &MRI = MF->getRegInfo();
      MachineInstr * ReplaceMI = BuildMI(MB, II, DL, TII->get(MI.getOpcode()));
      for( MachineOperand pMO: OperandsInMI) {
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
 * */
static void propogateNonRefInfo(const MachineBasicBlock &MBB) {
  MachineBasicBlock::const_iterator MBBI = MBB.begin(), E = MBB.end();
  while(MBBI != E) {
      MachineBasicBlock::const_iterator NMBBI = std::next(MBBI);
      const MachineInstr &MI = *MBBI;
      MBBI = NMBBI;
      // if MO_use is in SetNonRefRegInFunc, then MO_def should be in SetNonRefRegInFunc
      if (MI.getOpcode() == XVM::COPY || MI.getOpcode() == XVM::MOV_rr) {
        assert(MI.getNumOperands() == 2);
        const MachineOperand &MO_def = MI.getOperand(0);
        const MachineOperand &MO_use = MI.getOperand(1);
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
  while(MBBI != E) {
      MachineBasicBlock::const_iterator NMBBI = std::next(MBBI);
      const MachineInstr &MI = *MBBI;
      MBBI = NMBBI;
      if (MI.getOpcode() == XVM::CALL_IMM) {
        assert(MI.getNumOperands() >= 3);
        const MachineOperand &MO_0 = MI.getOperand(0);
        const MachineOperand &MO_2 = MI.getOperand(2);
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
              const MachineInstr &NextNextMI =*MBBI;
              if (NextNextMI.getOpcode() == XVM::COPY ||
                  NextNextMI.getOpcode() == XVM::MOV_rr) {
                assert(NextNextMI.getNumOperands() == 2);
                const MachineOperand &MO_def = NextNextMI.getOperand(0);
                const MachineOperand &MO_use = NextNextMI.getOperand(1);
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

bool XVMUpdateRefInstrForMI::runOnMachineFunction(MachineFunction &MF) {
  TII = MF.getSubtarget<XVMSubtarget>().getInstrInfo();
  LLVM_DEBUG(dbgs() << "Check/update refs in fun:" << MF.getFunction().getName().data() << '.\n');

  MapRefRegInFunc.clear();
  MapPtrRegInFunc.clear();
  MachineOperandRegisterReplacementMap.clear();
  MachineInstrExceptionSet.clear();
  SetNonRefRegInFunc.clear();
  CheckFunctionArgs(MF.getFunction());
  CheckFunctionReturn(MF.getFunction());
  FindNonRefRegInFunc(MF);

  bool Modified = false;
  for (auto &MBB : MF) {
    Modified |= updateRefInfoInMBB(MBB);
  }
  return Modified;
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

bool XVMUpdateRefInstrForMI::updateRefInfoInMBB(MachineBasicBlock &MBB) {
  int InstNumber = 0;
  bool Modified = false;
  updatePtrRefInMBB(MBB);

  /* FIXME: the two passes may be merged for efficiency */
  MachineBasicBlock::reverse_iterator MBBI = MBB.rbegin(), E = MBB.rend();
  InstNumber = std::distance(MBB.begin(), MBB.end());
  for (int i = 0; i < InstNumber; i++) {
    MachineBasicBlock::reverse_iterator NMBBI = std::next(MBBI);
    MachineInstr &MI = *MBBI;
    checkSimpleMIWithRef(MI);
    checkMovMIWithRef(MI);
    checkLoadMIWithRef(MI);
    checkStoreMIWithRef(MI);
    checkAddSubMIWithRef(MI);
    checkOrXorAndMIWithRef(MI);
    checkPhiMIWithRef(MI);
    MBBI = NMBBI;
  }
  /* update the instructions */
  MBBI = MBB.rbegin(), E = MBB.rend();
  InstNumber = std::distance(MBB.begin(), MBB.end());
  for (int i = 0; i < InstNumber; i++) {
    MachineBasicBlock::reverse_iterator NMBBI = std::next(MBBI);
    MachineInstr &MI = *MBBI;
    Modified |= updateCopyMIWithRef(MI, TII);
    Modified |= updateMovMIWithRef(MI, TII);
    Modified |= updateLoadMIWithRef(MI, TII);
    Modified |= updateStoreMIWithRef(MI, TII);
    Modified |= updateAddSubMIWithRef(MI, TII);
    Modified |= updateOrXorAndMIWithRef(MI, TII);
    Modified |= updatePhiMIWithRef(MI, TII);
    Modified |= updateRegistersInMI(MI, TII);
    MBBI = NMBBI;
  }
  return Modified;
}

INITIALIZE_PASS(XVMUpdateRefInstrForMI, "xvm-Ref-Determine-pseudo",
                XVM_REF_DETERMINE_NAME, false, false)
namespace llvm {

FunctionPass *createXVMUpdateRefInstrForMIPass() { return new XVMUpdateRefInstrForMI(); }

}

#endif
