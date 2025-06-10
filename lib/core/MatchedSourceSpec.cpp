#include "MatchedSourceSpec.h"

#include <iterator>

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
    }

}