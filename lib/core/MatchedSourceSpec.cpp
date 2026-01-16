#include "MatchedSourceSpec.h"
#include "SrcLookup.h"

#include <iterator>

#include <llvm/ADT/SmallSet.h>
#include <llvm/Support/raw_ostream.h>

namespace sourcespec2ir
{

    void MatchedSpecifications::merge(std::unique_ptr<MatchedSpecifications> specs2)
    {
        matchedFContracts.append(
            std::make_move_iterator(specs2->matchedFContracts.begin()),
            std::make_move_iterator(specs2->matchedFContracts.end()));
        matchedLoopInvariants.append(
            std::make_move_iterator(specs2->matchedLoopInvariants.begin()),
            std::make_move_iterator(specs2->matchedLoopInvariants.end()));
        matchedStmntBlocks.append(
            std::make_move_iterator(specs2->matchedStmntBlocks.begin()),
            std::make_move_iterator(specs2->matchedStmntBlocks.end()));
        matchedGhostContracts.append(
            std::make_move_iterator(specs2->matchedGhostContracts.begin()),
            std::make_move_iterator(specs2->matchedGhostContracts.end()));
        ghostMethods.append(
            std::make_move_iterator(specs2->ghostMethods.begin()),
            std::make_move_iterator(specs2->ghostMethods.end()));
    }

    bool MatchedSpecifications::hasDuplicateMappings()
    {
        bool hasDuplicates = false;

        // Check function contracts:
        llvm::SmallSet<llvm::Function *, 8> matchedFunctions;
        for (const auto &contract : matchedFContracts)
        {
            auto *f = contract->function;
            if (matchedFunctions.contains(f))
            {
                hasDuplicates = true;
                llvm::outs()
                    << "WARNING: Multiple contracts were matched to function "
                    << utils::getSrcNameOfFunction(*f) << "\n";
            }
            matchedFunctions.insert(f);
        }

        llvm::SmallSet<llvm::MDTuple *, 8> matchedLoops;
        for (const auto &invBlock : matchedLoopInvariants)
        {
            auto *l = invBlock->loopID;
            if (matchedLoops.contains(l))
            {
                hasDuplicates = true;
                // TODO: Make location more precise
                llvm::outs() << "WARNING: Multiple invariants were matched to "
                             << "the same loop in function "
                             << utils::getSrcNameOfFunction(*invBlock->parentFunction)
                             << "\n";
            }
            matchedLoops.insert(l);
        }

        llvm::SmallSet<llvm::Instruction *, 8> matchedInsts;
        for (const auto &stmntBlock : matchedStmntBlocks)
        {
            auto *i = stmntBlock->inst;
            if (matchedInsts.contains(i))
            {
                hasDuplicates = true;
                // TODO: Make location more precise
                llvm::outs() << "WARNING: Multiple statement blocks were "
                             << "matched to the same instruction in function "
                             << utils::getSrcNameOfFunction(
                                    *i->getParent()->getParent())
                             << "\n";
            }
            matchedInsts.insert(i);
        }

        llvm::SmallSet<GhostMethod *, 8> matchedGMethods;
        for (const auto &contr : matchedGhostContracts)
        {
            auto *m = contr->method;
            if (matchedGMethods.contains(m))
            {
                hasDuplicates = true;
                llvm::outs() << "WARNING: Multiple contracts were "
                             << "matched to the same ghost method "
                             << m->name
                             << "\n";
            }
            matchedGMethods.insert(m);
        }

        return hasDuplicates;
    }

    llvm::SmallVector<GhostMethod *> MatchedSpecifications::getGhostMethods() 
    {
        llvm::SmallVector<GhostMethod *> res;
        for (auto &g : ghostMethods) 
            res.push_back(g.get());
        return res;
    }

}