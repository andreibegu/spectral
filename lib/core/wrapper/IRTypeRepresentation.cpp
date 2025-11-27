#include <wrapper/IRTypeRepresentation.h>

#include <sstream>
#include <utility>

#include <llvm/ADT/STLExtras.h>
#include <llvm/Support/raw_ostream.h>

#include "PallasException.h"

namespace sourcespec2ir
{

    std::unique_ptr<IRTypeRepresentation>
    IRTypeRepresentation::getArgReprFromCall(
        const llvm::CallInst &call, unsigned int startIdx, unsigned int numElem)
    {
        if (numElem == 0 || startIdx + numElem > call.arg_size())
            throw utils::PallasException("Invalid argument-indices");

        auto repr = std::make_unique<IRTypeRepresentation>();
        for (unsigned int i = 0; i < numElem; ++i)
        {
            auto argIdx = startIdx + i;
            auto arg = call.getArgOperand(argIdx);
            auto argT = arg->getType();
            auto argAttrs = call.getAttributes().getParamAttrs(argIdx);
            repr->addType(argT, argAttrs);
        }
        return repr;
    }

    std::unique_ptr<IRTypeRepresentation>
    IRTypeRepresentation::getRetReprFromCall(const llvm::CallInst &call)
    {
        // Check if it is return by sret-parameter or nomral return.
        if (call.getCalledFunction()->getReturnType()->isVoidTy())
        {
            // Return by sret-param
            if (!call.hasStructRetAttr())
                throw utils::PallasException(
                    "Failed to build return-representation: Missing sret-attribute");
            auto repr = std::make_unique<IRTypeRepresentation>();
            repr->addType(call.getCalledFunction()->getReturnType(),
                          call.getAttributes().getParamAttrs(0));
            return repr;
        }
        else
        {
            // Normal return
            auto repr = std::make_unique<IRTypeRepresentation>();
            repr->addType(call.getCalledFunction()->getReturnType(),
                          call.getAttributes().getRetAttrs());
            return repr;
        }
    }

    IRTypeRepresentation::IRTypeRepresentation() : types(),
                                                   attributes()
    {
    }

    void IRTypeRepresentation::addType(llvm::Type *t, llvm::AttributeSet attrs)
    {
        types.push_back(t);
        attributes.push_back(attrs);
    }

    llvm::ArrayRef<llvm::Type *> IRTypeRepresentation::getTypes() const
    {
        return llvm::ArrayRef(types);
    }

    llvm::AttributeSet
    IRTypeRepresentation::getAttributes(unsigned int idx) const
    {
        if (idx >= attributes.size())
            throw utils::PallasException("Invalid type-index");

        return attributes[idx];
    }

    size_t IRTypeRepresentation::getNumElems() const
    {
        return types.size();
    }

    bool IRTypeRepresentation::isReturnByParam() const
    {
        return types.size() == 1 && types.front()->isVoidTy();
    }

    std::string IRTypeRepresentation::asString() const
    {
        std::string sRepr;
        llvm::raw_string_ostream s(sRepr);
        for (auto [idx, ty, attrSet] : llvm::enumerate(types, attributes))
        {
            for (auto &attr : attrSet)
                s << attr.getAsString() << " ";
            ty->print(s, false, true);
            if (idx != types.size() - 1) 
                s << ",";
        }
        return sRepr;
    }

}