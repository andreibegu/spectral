#ifndef PALLAS_LOOP_ID_PASS_H
#define PALLAS_LOOP_ID_PASS_H

#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"

namespace sourcespec2ir
{

  /**
   * Implements a pass that adds a LoopID (i.e. the !llvm.loop-node) to every
   * loop that does not yet have one.
   */
  class LoopIDPass : public llvm::PassInfoMixin<LoopIDPass>
  {
  public:
    llvm::PreservedAnalyses run(llvm::Loop &l, llvm::LoopAnalysisManager &lam,
                                llvm::LoopStandardAnalysisResults &lar, 
                                llvm::LPMUpdater &u);
  };

} // namespace llvm

#endif // PALLAS_LOOP_ID_PASS_H