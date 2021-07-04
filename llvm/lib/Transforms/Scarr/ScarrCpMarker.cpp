//===-- ScarrCpMarker.cpp - Transform IR with Checkpoint Info ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
///===----------------------------------------------------------------------===//
//
// Mark Basic Blocks into different ScaRR checkpoint types.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scarr/ScarrCpMarker.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"

using namespace llvm;

void findVirtualCheckpoint(DominatorTree &DT, Function &F) {
  DT.recalculate(F);
  // generate the LoopInfoBase for the current function
  LoopInfoBase<BasicBlock, Loop>* KLoop = new LoopInfoBase<BasicBlock, Loop>();
  KLoop->releaseMemory();
  KLoop->analyze(DT);
  for (auto &bb : F) {
    // Since the BasicBlock would have been inlined, just traverse from main function
    if (F.getName() == "main") {
      auto loop = KLoop->getLoopFor(&bb);
      if (loop != nullptr) {
        loop->getHeader()->setCheckpoint(Checkpoint::Virtual);
      }
    }
  }
}

void findCheckpoints(DominatorTree &DT, Function &F, int nestedLevel) {
  bool isThreadStartCheckpoint = F.getName() == "main";
  for (auto &bb : F) {
    bool isThreadEndCheckpoint = false;
    bool isExitPointCheckpoint = false;
    for (auto &i : bb) {
      // if instruction is last in the block and has no more successor,
      // then this will be thread end checkpoint
      if (i.isTerminator()) {
        // Thread end only in the original function (main)
        if (i.getNumSuccessors() == 0 && nestedLevel == 0 && F.getName() == "main") {
          isThreadEndCheckpoint |= true;
        }
      }
      // Check if instruction is calling a function
      if (isa<CallInst>(i)) {
        auto *call = &cast<CallBase>(i);
        // use this hack to check if function is external
        if (call != nullptr && call->getCalledFunction() != nullptr && !call->getCalledFunction()->empty()) {
          auto calledFunction = call->getCalledFunction()->getName();
          if (calledFunction == F.getName()) {
            // Recursion is detected
            continue;
          }
          findCheckpoints(DT, *(call->getCalledFunction()), nestedLevel + 1);
        } else {
          // The function is outside of the translation unit, hence it is an exit point
          if (!isThreadStartCheckpoint && !isThreadEndCheckpoint) {
            isExitPointCheckpoint |= true;
          }
        }
      }
    }

    if (isThreadStartCheckpoint) {
      isThreadStartCheckpoint = false;
      bb.setCheckpoint(Checkpoint::ThreadStart);
    } else if (isThreadEndCheckpoint) {
      bb.setCheckpoint(Checkpoint::ThreadEnd);
    } else if (isExitPointCheckpoint) {
      bb.setCheckpoint(Checkpoint::ExitPoint);
    }
  }

  findVirtualCheckpoint(DT,F);
}


PreservedAnalyses ScarrCpMarkerPass::run(Function &F, FunctionAnalysisManager &AM) {
  DominatorTree DT;
  findCheckpoints(DT, F, 0);
  return PreservedAnalyses::all();
}
