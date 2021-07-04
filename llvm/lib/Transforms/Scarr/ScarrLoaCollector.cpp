//===- ScarrLoaCollector.cpp - Collect LoA from BasicBlock ------*- C++ -*-===//
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

#include "llvm/Transforms/Scarr/ScarrLoaCollector.h"
#include "llvm/ADT/BreadthFirstIterator.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Dominators.h"
#include <iostream>
#include <vector>

using namespace llvm;

// We will store ScaRR measurements in this type
using MeasurementMap = std::map<std::pair<BasicBlock *, BasicBlock *>, std::vector<BasicBlock *>>;

// For each BasicBlock find checkpoint and collect LoA that direct
// control flow from previous Checkpoint to the next one.
void handle(BasicBlock *firstCp,
            BasicBlock *successor,
            MeasurementMap &measurements,
            std::vector<BasicBlock *> &LoA,
            std::array<BasicBlock *, 2> &checkpointPair) {
  for (auto bb : successors(successor)) {
    if (bb->getCheckpoint() != Checkpoint::NA) {
      outs() << "=====================\n";
      outs() << "CP_A:" << *firstCp << "\n";
      outs() << "CP_B:" << *bb << "\n";
    } else {
      handle(firstCp, bb, measurements, LoA, checkpointPair);
    }
  }
}

void collectListOfActions(DominatorTree &DT, Function &function) {
  // We will put BasicBlock in vector for easy reference
  std::vector<BasicBlock *> basicBlocks;
  std::vector<BasicBlock *> basicBlockCheckpoints;
  if (function.getName() == "main") {
    for (auto it : depth_first(&function.getEntryBlock())) {
      basicBlocks.push_back(it);
      if (it->getCheckpoint() != Checkpoint::NA) {
        basicBlockCheckpoints.push_back(it);
      }
    }
  }

  // Pair of Checkpoint_A and Cheeckpont_B
  std::array<BasicBlock *, 2> checkpointPair = {nullptr, nullptr};
  // List of Action
  std::vector<BasicBlock *> LoA;
  // We will store the measurement here
  MeasurementMap measurements;
  for (auto cp : basicBlockCheckpoints) {
    handle(cp, cp, measurements, LoA, checkpointPair);
  }

  // Printing the result
  std::cout << "=============================" << measurements.size() << std::endl;
  std::cout << "The size of the measurement: " << measurements.size() << std::endl;
  for (auto iter: measurements) {
    auto key = iter.first;
    auto value = iter.second;
    outs() << "Checkpoint_A: " << *key.first << "\n";
    outs() << "Checkpoint_B: " << *key.second << "\n";

    int idx = 0;
    for (auto loa : value) {
      outs() << "LOA_" << idx << ": " << *loa << "\n";
      idx++;
    }
    outs() << "=====================================\n";
  }
}

PreservedAnalyses ScarrLoaCollectorPass::run(Function &F, FunctionAnalysisManager &AM) {
  DominatorTree *DT = new DominatorTree();
  DT->recalculate(F);
  collectListOfActions(*DT, F);
  return PreservedAnalyses::all();
}
