#include <wrapper/MatchedWrapperFunctions.h>

namespace sourcespec2ir
{

    MatchedWrapperFunctions::MatchedWrapperFunctions() : fContractWrappers(),
                                                         loopInvWrappers(),
                                                         statementWrappers()
    {
    }

    std::unique_ptr<llvm::SmallVector<SrcWrapperFunction *>> 
    MatchedWrapperFunctions::getWrappers() 
    {
        auto wrappers = std::make_unique<llvm::SmallVector<SrcWrapperFunction *>>();
        for (auto &[contr, wrapper] : fContractWrappers)
            wrappers->push_back(wrapper.get());
        for (auto &[inv, wrapper] : loopInvWrappers)
            wrappers->push_back(wrapper.get());
        for (auto &[stmnt, wrapper] : statementWrappers)
            wrappers->push_back(wrapper.get());
        return wrappers;
    }

}