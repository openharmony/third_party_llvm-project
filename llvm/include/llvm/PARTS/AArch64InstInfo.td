//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans@liljestrand.dev>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

let isPseudo = 1, hasNoSchedulingInfo = 1 in {
    def PARTS_PACIA : Pseudo<(outs GPR64:$dst),
                             (ins GPR64:$ptr, GPR64sp:$mod),
                             [(set i64:$dst, (int_pa_pacia GPR64:$ptr, GPR64sp:$mod))],
                             "$dst = $ptr">;
    def PARTS_AUTIA : Pseudo<(outs GPR64:$dst),
                             (ins GPR64:$ptr, GPR64sp:$mod),
                             [(set i64:$dst, (int_pa_autia GPR64:$ptr, GPR64sp:$mod))],
                             "$dst = $ptr">;
    def PARTS_AUTCALL : Pseudo<(outs GPR64:$dst),
                             (ins GPR64:$ptr, GPR64sp:$mod),
                             [(set i64:$dst, (int_pa_autcall GPR64:$ptr, GPR64sp:$mod))],
                             "$dst = $ptr">;
}

let isCall = 1, isTerminator = 1, isReturn = 1, isBarrier = 1, Uses = [SP] in {
    def TCRETURNriAA : Pseudo<(outs), (ins tcGPR64:$dst, i32imm:$FPDiff, GPR64sp:$mod), []>,
        Sched<[WriteBrReg]>;
}
