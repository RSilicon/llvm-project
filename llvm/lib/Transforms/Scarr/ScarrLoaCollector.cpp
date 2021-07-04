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
            std::vector<BasicBlock *> LoA,
            std::array<BasicBlock *, 2> &checkpointPair) {
  // This checkpoint is branch, hence we need to collect LoA
  if (firstCp->getTerminator()->getNumSuccessors() > 1) {
    // We only add firstCp to LoA if LoA is still empty
    if (LoA.empty()) {
      LoA.push_back(firstCp);
    }
  }
  for (auto succ : successors(successor)) {
    // We need a copy of LoA for every branch out
    auto succLoA = LoA;
    // Successor is a checkpoint
    if (succ->getCheckpoint() != Checkpoint::NA) {
      if (succLoA.size() == 1) {
        succLoA.push_back(succ);
      }
      auto cp = std::make_pair(firstCp, succ);
      measurements[cp] = succLoA;
    } else {
      if (!succLoA.empty() &&
          succLoA.back()->getCheckpoint() != Checkpoint::NA) {
        succLoA.push_back(succ);
      }
      handle(firstCp, succ, measurements, succLoA, checkpointPair);
    }
  }
}

void collectListOfActions(DominatorTree &DT, Function &function) {
  // We will put BasicBlock in vector for easy reference
  std::vector<BasicBlock *> basicBlockCheckpoints;
  if (function.getName() == "main") {
    for (auto it : depth_first(&function.getEntryBlock())) {
      if (it->getCheckpoint() != Checkpoint::NA) {
        basicBlockCheckpoints.push_back(it);
      }
    }

    // Pair of Checkpoint_A and Checkpoint_B
    std::array<BasicBlock *, 2> checkpointPair = {nullptr, nullptr};
    // List of Action
    std::vector<BasicBlock *> LoA;
    // We will store the measurement here
    MeasurementMap measurements;
    for (auto cp : basicBlockCheckpoints) {
      handle(cp, cp, measurements, LoA, checkpointPair);
    }

    auto loaCount = 0;
    for (auto measurement : measurements) {
      auto listOfActions = measurement.second;
      loaCount+= listOfActions.size();
    }

    // Printing the result
    std::cout << "=============================================================" << std::endl;
    std::cout << "ScaRR Offline Measurement Statistics" << std::endl;
    std::cout << "=============================================================" << std::endl;
    std::cout << "Offline Measurement Size: " << measurements.size() << std::endl;
    std::cout << "Number of Checkpoints: " << basicBlockCheckpoints.size() << std::endl;
    std::cout << "Number of List of Actions: " << loaCount << std::endl;
    std::cout << "=============================================================" << std::endl;
    std::cout << "Checkpoints and LoA Details: " << std::endl;
    auto mIndex = 0;
    for (auto measurement : measurements) {
      auto cpPair = measurement.first;
      auto listOfActions = measurement.second;
      std::cout << "=============================================================" << std::endl;
      std::cout << "Checkpoint " << mIndex << std::endl;
      std::cout << "LoA Size: " << listOfActions.size() << std::endl;
      outs() << "Checkpoint_A: " << *cpPair.first << "\n";
      outs() << "Checkpoint_B: " << *cpPair.second << "\n";

      if (!listOfActions.empty()) {
        std::cout << "LoA Details: " << std::endl;
      }
      int idx = 0;
      for (auto loa : listOfActions) {
        outs() << "LOA_" << idx << ": " << *loa << "\n";
        idx++;
      }
      std::cout << "=============================================================" << std::endl;
      mIndex++;
    }
  }


}

PreservedAnalyses ScarrLoaCollectorPass::run(Function &F, FunctionAnalysisManager &AM) {
  DominatorTree *DT = new DominatorTree();
  DT->recalculate(F);
  collectListOfActions(*DT, F);
  return PreservedAnalyses::all();
}
