//===- ScarrLoaCollector.cpp - Collect LoA from BasicBlock ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scarr/ScarrLoaCollector.h"
#include "llvm/ADT/BreadthFirstIterator.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/IR/CFG.h"
#include <iostream>
#include <vector>

using namespace llvm;

static std::string cpToString(Checkpoint cp) {
  switch(cp) {
  case Checkpoint::NA:
    return "";
  case Checkpoint::ThreadStart:
    return "ThreadStart";
  case Checkpoint::ThreadEnd:
    return "ThreadEnd";
  case Checkpoint::ExitPoint:
    return "ExitPoint";
  case Checkpoint::Virtual:
    return "Virtual";
  default:
    return "Unknown";
  }
}

using MeasurementMap = std::map<std::pair<BasicBlock *, BasicBlock *>, std::vector<BasicBlock *>>;

void handle(BasicBlock *bb,
            MeasurementMap &measurements,
            std::vector<BasicBlock *> &loas,
            std::array<BasicBlock *, 2> &bbPair) {

  if (bbPair[0] == nullptr) {
    if (bb->getCheckpoint() != Checkpoint::NA) {
      bbPair[0] = bb;
      loas = std::vector<BasicBlock *>();
      loas.push_back(bb);
    }
  } else if (bbPair[1] == nullptr) {
    if (bb->getCheckpoint() != Checkpoint::NA) {
      bbPair[1] = bb;
      if (loas.size() < 2) {
        loas.push_back(bb);
      }
      std::pair<BasicBlock *, BasicBlock *> key = {bbPair[0], bbPair[1]};
      measurements[key] = loas;
      // Current checkpoint will be the next checkpoint
      bbPair = {nullptr, nullptr};
      if (bb->getCheckpoint() != Checkpoint::NA) {
        bbPair[0] = bb;
        loas = std::vector<BasicBlock *>();
        loas.push_back(bb);
      }
    } else {
      if (loas.size() < 2) {
        loas.push_back(bb);
      }
    }
  } else {
    bbPair = {nullptr, nullptr};
    if (bb->getCheckpoint() != Checkpoint::NA) {
      bbPair[0] = bb;
      loas = std::vector<BasicBlock *>();
      loas.push_back(bb);
    }
  }
  if (bb->getTerminator()->getNumSuccessors() > 1) {
    for (auto succ : successors(bb)) {
      std::array<BasicBlock*, 2> bbPairNested = {bbPair[0], nullptr};
      std::vector<BasicBlock*> nestedLoas = loas;
      handle(succ, measurements, nestedLoas, bbPairNested);
    }
  }
}


void markLoa(Function &function) {
  // Put in vector for easy reference
  std::vector<BasicBlock *> bbs;
  if (function.getName() == "main") {
    for (auto it : depth_first(&function.getEntryBlock())) {
      bbs.push_back(it);
    }
  }

  std::array<BasicBlock *, 2> bbPair = {nullptr, nullptr};
  std::vector<BasicBlock *> loas;
  MeasurementMap measurements;
  for (auto bb : depth_first(bbs[0])) {
    handle(bb, measurements, loas, bbPair);
  }


  std::cout << "Results size is " << measurements.size() << std::endl;
  for (auto iter: measurements) {
    auto key = iter.first;
    auto value = iter.second;
    outs() << "CP1: " << *key.first << "\n";
    outs() << "CP2: " << *key.second << "\n";
    int idx = 0;
    for (auto loa : value) {
      outs() << "LOA" << idx << ": " << *loa << "\n";
      idx++;
    }
  }
}

PreservedAnalyses ScarrLoaCollectorPass::run(Function &F, FunctionAnalysisManager &AM) {
  if (F.getName() == "main") {
    outs() << "==================================================\n";
  }
  outs() << "Function '" << F.getName() << "'\n";
  markLoa(F);
  return PreservedAnalyses::all();
}
