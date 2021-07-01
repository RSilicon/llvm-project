//===- ScarrLoaCollector.h - Collect LoA from BasicBlock --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
///===---------------------------------------------------------------------===//
//
// Collect LoA from BasicBlock. Must be run after scarr-cp-marker pass.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SCARR_SCARRLOACOLLECTOR_H
#define LLVM_TRANSFORMS_SCARR_SCARRLOACOLLECTOR_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class ScarrLoaCollectorPass : public PassInfoMixin<ScarrLoaCollectorPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SCARR_SCARRLOACOLLECTOR_H
