
#include "llvm/CodeGen/MIRParser/MIRParser.h"
#include "llvm/CodeGen/MachineDominanceFrontier.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/ADT/Twine.h"
#include "gtest/gtest.h"
#include "AArch64PARTS/AArch64EarlyPartsCpiPass.h"

using namespace llvm;
namespace {
std::unique_ptr<LLVMTargetMachine> createTargetMachine() {
  auto TT(Triple::normalize("aarch64--"));
  std::string CPU("");
  std::string FS("+pauth");

  LLVMInitializeAArch64TargetInfo();
  LLVMInitializeAArch64Target();
  LLVMInitializeAArch64TargetMC();

  std::string Error;
  const Target *TheTarget = TargetRegistry::lookupTarget(TT, Error);

  return std::unique_ptr<LLVMTargetMachine>(static_cast<LLVMTargetMachine*>(
      TheTarget->createTargetMachine(TT, CPU, FS, TargetOptions(), None, None,
                                     CodeGenOpt::Default)));
}

std::unique_ptr<Module> parseMIR(LLVMContext &Context,
                                 std::unique_ptr<MIRParser> &MIR,
                                 const TargetMachine &TM, StringRef MIRCode,
                                 const char *FuncName, MachineModuleInfo &MMI) {
  SMDiagnostic Diagnostic;
  std::unique_ptr<MemoryBuffer> MBuffer = MemoryBuffer::getMemBuffer(MIRCode);
  MIR = createMIRParser(std::move(MBuffer), Context);
  if (!MIR)
    return nullptr;

  std::unique_ptr<Module> M = MIR->parseIRModule();
  if (!M)
    return nullptr;

  M->setDataLayout(TM.createDataLayout());
  if (MIR->parseMachineFunctions(*M, MMI))
    return nullptr;

  return M;
}

TEST(EarlyPartsCpi, PhiBlr) {
  std::unique_ptr<LLVMTargetMachine> TM = createTargetMachine();
  ASSERT_TRUE(TM);

  MachineModuleInfo MMI(TM.get());
  SmallString<5000> S;
  StringRef MIRString = Twine(R"MIR(
--- |
  define i32 @pac_test(ptr nocapture noundef readonly %0, i64 noundef %1, ptr nocapture noundef readonly %2, ptr noundef %3) {
    unreachable
  }
...
---
name: pac_test
tracksRegLiveness: true
body: |
  bb.0:
    liveins: $x0, $x1, $x2, $x3
    successors: %bb.1
    %59:gpr64 = COPY $x3
    %58:gpr64 = COPY $x2
    %57:gpr64 = COPY $x1
    %56:gpr64common = COPY $x0
    %62:gpr32all = COPY $wzr
    %60:gpr32all = COPY %62:gpr32all
    B %bb.1

  bb.1:
    ; predecessors: %bb.0
    successors: %bb.2, %bb.3
    %99:gpr32 = MOVi32imm 22545
    %100:gpr64sp = SUBREG_TO_REG 0, killed %99:gpr32, %subreg.sub_32
    %101:gpr64 = PARTS_AUTCALL %58:gpr64(tied-def 0), killed %100:gpr64sp
    ADJCALLSTACKDOWN 0, 0, implicit-def dead $sp, implicit $sp
    $x0 = COPY %57:gpr64
    $x1 = COPY %59:gpr64
    BLR killed %101:gpr64
    ADJCALLSTACKUP 0, 0, implicit-def dead $sp, implicit $sp
    CBNZX %57:gpr64, %bb.3
    B %bb.2

  bb.2:
    ; predecessors: %bb.1
    successors: %bb.4
    %131:gpr32 = MOVi32imm 22545
    %132:gpr64sp = SUBREG_TO_REG 0, killed %131:gpr32, %subreg.sub_32
    %133:gpr64 = PARTS_AUTCALL %58:gpr64(tied-def 0), killed %132:gpr64sp
    %31:gpr64 = COPY %133:gpr64
    B %bb.4

  bb.3:
    ; predecessors: %bb.1
    successors: %bb.4
    %125:gpr32 = MOVi32imm 22545
    %126:gpr64sp = SUBREG_TO_REG 0, killed %125:gpr32, %subreg.sub_32
    %127:gpr64 = PARTS_AUTCALL %58:gpr64(tied-def 0), killed %126:gpr64sp
    %32:gpr64 = COPY %127:gpr64
    ADJCALLSTACKDOWN 0, 0, implicit-def dead $sp, implicit $sp
    $x0 = COPY %57:gpr64
    $x1 = COPY %59:gpr64
    BLR %127:gpr64
    ADJCALLSTACKUP 0, 0, implicit-def dead $sp, implicit $sp
    B %bb.4

  bb.4:
    ; predecessors: %bb.2, %bb.3
    successors: %bb.5
    %35:gpr64 = PHI %31:gpr64, %bb.2, %32:gpr64, %bb.3
    B %bb.5

  bb.5:
    ; predecessors: %bb.4
    ADJCALLSTACKDOWN 0, 0, implicit-def dead $sp, implicit $sp
    $x0 = COPY %57:gpr64
    $x1 = COPY %59:gpr64
    BLR %35:gpr64
    ADJCALLSTACKUP 0, 0, implicit-def dead $sp, implicit $sp
    RET_ReallyLR implicit $w0
)MIR").toNullTerminatedStringRef(S);;

  LLVMContext Context;
  std::unique_ptr<MIRParser> MIR;
  std::unique_ptr<Module> M = parseMIR(Context, MIR, *TM, MIRString, "pac_test", MMI);
  ASSERT_TRUE(M);

  Function *F = M->getFunction("pac_test");
  auto *MF = MMI.getMachineFunction(*F);
  ASSERT_TRUE(MF);
  std::unique_ptr<AArch64EarlyPartsCpiPass> funcPass(new AArch64EarlyPartsCpiPass());
  ASSERT_TRUE(funcPass);

  bool ret = funcPass->runOnMachineFunction(*MF);
  ASSERT_TRUE(ret);
}
}