#ifndef PALLAS_SRCWRAPPERFUNCTION_H
#define PALLAS_SRCWRAPPERFUNCTION_H

#include <optional>
#include <string>
#include <llvm/ADT/MapVector.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/DebugInfoMetadata.h>

namespace sourcespec2ir
{

    /**
     * A struct that bundles the name and the string representation of a
     * wrapper-function along with the variables from the parent function that 
     * correspond to the argument-list.
     * The variables from the parent-function are mpped to the name of the 
     * corresponding argument in the wrapper-function.  
     * Further contains line-information on where the wrapper should be inserted
     * in the source-file. 
     */
    class SrcWrapperFunction
    {
        public: 
        
        /**
         * Creates a SrcWrapperFunction where the names of the wrapper-arguments 
         * exactly match the names of the variabes in the parent method.   
         */
        SrcWrapperFunction(std::string name, std::string stringRepr,
                           llvm::ArrayRef<llvm::DILocalVariable *> argList, 
                           llvm::ArrayRef<std::string> givenArgs,
                           llvm::ArrayRef<std::string> yieldsArgs,
                           size_t insertLoc);

        /**
         * Creates a SrcWrapperFunction where the names of the wrapper-arguments 
         * do not necessarily match the names of the variable in the associated 
         * function.
         * 'argList' and 'argNames' are required to have the same length.
         */
        SrcWrapperFunction(std::string name, std::string stringRepr,
                           llvm::ArrayRef<llvm::DILocalVariable *> argList, 
                           llvm::ArrayRef<std::string> argNames,
                           llvm::ArrayRef<std::string> givenArgs,
                           llvm::ArrayRef<std::string> yieldsArgs,
                           size_t insertLoc);
        
        const std::string& getName() const {return name; };

        const std::string& getStringRepr() const {return stringRepr; };

        size_t getInsertLoc() const { return insertLoc; };

        llvm::ArrayRef<std::string> getGivenArgs() const { return givenArgs; };

        llvm::ArrayRef<std::string> getYieldsArgs() const { return yieldsArgs; };

        auto getPVars() const { return llvm::make_first_range(pVarsToArgNames); }

        void addArgs(llvm::ArrayRef<llvm::DILocalVariable *> args) {
            for (auto *a : args) {
                pVarsToArgNames.insert({a, a->getName().str()});
            }
        }

        std::optional<std::string> 
        getArgNameForVar(llvm::DILocalVariable &v) const { 
            auto r = pVarsToArgNames.find(&v);
            if (r == pVarsToArgNames.end()) {
                return std::nullopt;
            } else {
                return std::make_optional(r->second);
            }
        };

        private: 
        std::string name;
        std::string stringRepr;

        // Mapping of variables in the associated function to the names of the 
        // wrappers arguments. 
        llvm::MapVector<llvm::DILocalVariable *, std::string> pVarsToArgNames;
        // llvm::SmallVector<llvm::DILocalVariable *> argList;
        llvm::SmallVector<std::string> givenArgs;
        llvm::SmallVector<std::string> yieldsArgs;
        size_t insertLoc;

    };
}

#endif // PALLAS_SRCWRAPPERFUNCTION_H