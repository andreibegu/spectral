#include "passes/LoopIDPass.h"

#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Metadata.h>

namespace sourcespec2ir
{

  llvm::PreservedAnalyses LoopIDPass::run(
      llvm::Loop &l, llvm::LoopAnalysisManager &lam,
      llvm::LoopStandardAnalysisResults &lar, llvm::LPMUpdater &u)
  {

    // Skip loops that already have an ID
    if (l.getLoopID() != nullptr)
      return llvm::PreservedAnalyses::all(); 

    // Build a new Loop-ID Node
    llvm::SmallVector<llvm::Metadata *> mdElems;
    // Will be replaced later for self-reference
    mdElems.push_back(nullptr); 

    // TODO: It would probably be mor accurate to iterate over the instructions
    //       to obtain the line-numbers and the scope. 
    // Insert loop start- and end-location
    if (l.getLocRange().getStart())
      mdElems.push_back(l.getLocRange().getStart().get()); 
    if (l.getLocRange().getEnd())
      mdElems.push_back(l.getLocRange().getEnd().get()); 

    auto &irContext = l.getHeader()->getContext();
    auto loopId = llvm::MDNode::getDistinct(irContext, mdElems);
    // Build the self-reference
    loopId->replaceOperandWith(0, loopId);

    l.setLoopID(loopId);

    // TODO: We can probably be less pessimistic here, since we only 
    //       modify the Loop-IDs
    return llvm::PreservedAnalyses::none();
  }

} // namespace llvm
