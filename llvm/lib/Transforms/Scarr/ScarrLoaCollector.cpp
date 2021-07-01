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

void traverseCFG(Function &F) {
  outs() << "===============================================\n";
  outs() << "Basic blocks of " << F.getName() << " in df_iterator:\n";
//  auto counter = 0;
  for (auto iterator = df_begin(&F.getEntryBlock()),
           IE = df_end(&F.getEntryBlock());
       iterator != IE; ++iterator) {
    outs() << iterator->getName() << "\n";

//    std::string name = iterator->getName().str();
//    name.append("\\l df: ").append(std::to_string(counter));
//    iterator->setName(name);
//    counter++;
    for (auto &instruction : **iterator) {
      outs() << instruction << "\n";
    }
  }
  outs() << "\n\n";

  outs() << "===============================================\n";
  outs() << "Basic blocks of " << F.getName() << " in idf_iterator:\n";
//  counter = 0;
  for (auto iterator = idf_begin(&F.getEntryBlock()),
           IE = idf_end(&F.getEntryBlock());
       iterator != IE; ++iterator) {
    outs() << iterator->getName() << "\n";

//    std::string name = iterator->getName().str();
//    name.append("\\l idf: ").append(std::to_string(counter));
//    iterator->setName(name);
//    counter++;
    for (auto &instruction : **iterator) {
      outs() << instruction << "\n";
    }
  }
  outs() << "\n\n";

  outs() << "===============================================\n";
  outs() << "Basic blocks of " << F.getName() << " in bf_iterator:\n";
//  counter = 0;
  for (auto iterator = bf_begin(&F.getEntryBlock()),
           IE = bf_end(&F.getEntryBlock());
       iterator != IE; ++iterator) {
    outs() << iterator->getName() << "\n";

//    std::string name = iterator->getName().str();
//    name.append("\\l bf: ").append(std::to_string(counter));
//    iterator->setName(name);
//    counter++;
    for (auto &instruction : **iterator) {
      outs() << instruction << "\n";
    }
  }
  outs() << "\n\n";

  outs() << "===============================================\n";
  outs() << "Basic blocks of " << F.getName() << " in scc_iterator:\n";
//  counter = 0;
  for (auto iterator = scc_begin(&F.getEntryBlock()),
           IE = scc_end(&F.getEntryBlock());
       iterator != IE; ++iterator) {
//    outs() << iterator->getName() << "\n";
//
//    std::string name = iterator->getName().str();
//    name.append("\\l scc: ").append(std::to_string(counter));
//    iterator->setName(name);
//    counter++;
    for (auto &instruction : *iterator) {
      outs() << *instruction << "\n";
    }
  }
  outs() << "\n\n";

  outs() << "===============================================\n";
  outs() << "Basic blocks of " << F.getName() << " in po_iterator:\n";
//  counter = 0;
  for (auto iterator = po_begin(&F.getEntryBlock()),
           IE = po_end(&F.getEntryBlock());
       iterator != IE; ++iterator) {
    outs() << iterator->getName() << "\n";

//    std::string name = iterator->getName().str();
//    name.append("\\l po: ").append(std::to_string(counter));
//    iterator->setName(name);
//    counter++;
    for (auto &instruction : **iterator) {
      outs() << instruction << "\n";
    }
  }
  outs() << "\n\n";

  outs() << "===============================================\n";
  outs() << "Basic blocks of " << F.getName() << " in pred_iterator:\n";
  for (auto iterator = pred_begin(&F.getEntryBlock()), IE = pred_end(&F.getEntryBlock());
       iterator != IE; ++iterator) {
    outs() << *iterator << "\n";
    for (auto &instruction : **iterator) {
      outs() << instruction << "\n";
    }
  }
  outs() << "\n\n";

  outs() << "===============================================\n";
  outs() << "Basic blocks of " << F.getName() << " in succ_iterator:\n";
//  counter = 0;
  for (auto iterator = succ_begin(&F.getEntryBlock()),
           IE = succ_end(&F.getEntryBlock());
       iterator != IE; ++iterator) {
    outs() << iterator->getName() << "\n";

//    std::string name = iterator->getName().str();
//    name.append(" \\l succ: ").append(std::to_string(counter));
//    iterator->setName(name);
//    counter++;
    for (auto &instruction : **iterator) {
      outs() << instruction << "\n";
    }
  }
  outs() << "\n\n";
}

void dfs(Function &F) {
  outs() << "===============================================\n";
  outs() << "Basic blocks of " << F.getName() << " in df_iterator:\n";
  for (auto iterator = df_begin(&F.getEntryBlock()),
           IE = df_end(&F.getEntryBlock());
       iterator != IE; ++iterator) {
    outs() << **iterator << "\n";
    std::cout << cpToString(iterator->getCheckpoint()) << std::endl;
  }
  outs() << "\n\n";
}

void bfs(Function &F) {
  outs() << "===============================================\n";
  outs() << "Basic blocks of " << F.getName() << " in bf_iterator:\n";
  for (auto iterator = bf_begin(&F.getEntryBlock()),
           IE = bf_end(&F.getEntryBlock());
       iterator != IE; ++iterator) {
    outs() << **iterator << "\n";
    std::cout << cpToString(iterator->getCheckpoint()) << std::endl;
  }
  outs() << "\n\n";
}

PreservedAnalyses ScarrLoaCollectorPass::run(Function &F, FunctionAnalysisManager &AM) {
  if (F.getName() == "main") {
    outs() << "==================================================\n";
  }
  outs() << "Function '" << F.getName() << "'\n";
  bfs(F);
  return PreservedAnalyses::all();
}
