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
#include <iostream>
#include <vector>

using namespace llvm;

// We will store ScaRR measurements in this type
using MeasurementMap = std::map<std::pair<BasicBlock *, BasicBlock *>, std::vector<BasicBlock *>>;

// For each BasicBlock find checkpoint and collect LoA that direct
// control flow from previous Checkpoint to the next one.
void handle(BasicBlock *basicBlock,
            MeasurementMap &measurements,
            std::vector<BasicBlock *> &LoA,
            std::array<BasicBlock *, 2> &checkpointPair) {

  if (checkpointPair[0] == nullptr) {
    if (basicBlock->getCheckpoint() != Checkpoint::NA) {
      checkpointPair[0] = basicBlock;
      LoA = std::vector<BasicBlock *>();
      LoA.push_back(basicBlock);
    }
  } else if (checkpointPair[1] == nullptr) {
    if (basicBlock->getCheckpoint() != Checkpoint::NA) {
      checkpointPair[1] = basicBlock;
      if (LoA.size() < 2) {
        LoA.push_back(basicBlock);
      }
      std::pair<BasicBlock *, BasicBlock *> key = {checkpointPair[0],
                                                   checkpointPair[1]};
      measurements[key] = LoA;
      // Current checkpoint will be the next checkpoint
      checkpointPair = {nullptr, nullptr};
      if (basicBlock->getCheckpoint() != Checkpoint::NA) {
        checkpointPair[0] = basicBlock;
        LoA = std::vector<BasicBlock *>();
        LoA.push_back(basicBlock);
      }
    } else {
      if (LoA.size() < 2) {
        LoA.push_back(basicBlock);
      }
    }
  } else {
    checkpointPair = {nullptr, nullptr};
    if (basicBlock->getCheckpoint() != Checkpoint::NA) {
      checkpointPair[0] = basicBlock;
      LoA = std::vector<BasicBlock *>();
      LoA.push_back(basicBlock);
    }
  }
  if (basicBlock->getTerminator()->getNumSuccessors() > 1) {
    for (auto succ : successors(basicBlock)) {
      std::array<BasicBlock*, 2> bbPairNested = {checkpointPair[0], nullptr};
      std::vector<BasicBlock*> nestedLoas = LoA;
      handle(succ, measurements, nestedLoas, bbPairNested);
    }
  }
}


void collectListOfActions(Function &function) {
  // We will put BasicBlock in vector for easy reference
  std::vector<BasicBlock *> basicBlocks;
  if (function.getName() == "main") {
    for (auto it : depth_first(&function.getEntryBlock())) {
      basicBlocks.push_back(it);
    }
  }

  // Pair of Checkpoint_A and Cheeckpont_B
  std::array<BasicBlock *, 2> checkpointPair = {nullptr, nullptr};
  // List of Action
  std::vector<BasicBlock *> LoA;
  // We will store the measurement here
  MeasurementMap measurements;
  // Do DFS and collect List of Actions as we traverse BasicBlock and its checkpoints
  for (auto basicBlock : depth_first(basicBlocks[0])) {
    handle(basicBlock, measurements, LoA, checkpointPair);
  }

  // Printing the result
  std::cout << "The size of the measurement: " << measurements.size() << std::endl;
  for (auto iter: measurements) {
    auto key = iter.first;
    auto value = iter.second;
    outs() << "Checkpoint_A: " << *key.first << "\n";
    outs() << "Checkpoint_B: " << *key.second << "\n";

    int idx = 0;
    for (auto loa : value) {
      outs() << "LOA" << idx << ": " << *loa << "\n";
      idx++;
    }
    outs() << "=====================================\n";
  }
}

PreservedAnalyses ScarrLoaCollectorPass::run(Function &F, FunctionAnalysisManager &AM) {
  collectListOfActions(F);
  return PreservedAnalyses::all();
}
