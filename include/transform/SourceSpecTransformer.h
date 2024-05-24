#ifndef PALLAS_SOURCESPECTRANSFORMER_H
#define PALLAS_SOURCESPECTRANSFORMER_H

#include <string>
#include <iostream>
#include <memory>
#include <optional>

#include <llvm/ADT/StringMap.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/SourceMgr.h>

#include "antlr4-runtime.h"
#include "SpecLexer.h"
#include "SpecParser.h"
#include "SpecParserBaseVisitor.h"

#include "SourceSpecContextProvider.h"
#include "SourceCompilerDriver.h"

namespace sourcespec2ir
{

    class SourceSpecTransformer : SpecParserBaseVisitor
    {
    public:
        SourceSpecTransformer(std::unique_ptr<SourceSpecContextProvider> specCtxProvider, 
                              std::unique_ptr<SourceCompilerDriver> compilerDriver, char *irInputPath, char *srcInputPath);

        void transformModule();

        void printSpecifications(char *srcInputPath, char *irInputPath);

        // Overwritten visitor-methods
        std::any visitSpecContractBlock(SpecParser::SpecContractBlockContext *ctx) override;
        std::any visitRequiresClause(SpecParser::RequiresClauseContext *ctx) override;
        std::any visitEnsuresClause(SpecParser::EnsuresClauseContext *ctx) override;
        std::any visitInvariantClause(SpecParser::InvariantClauseContext *ctx) override;

    private:
        /* Counter for generating names for the specification functions.
         * TODO: Replace with something more elaborate that takes exisiting names into account.
         */
        int specFunCnt = 0;

        std::unique_ptr<SourceSpecContextProvider> specContextProvider;

        std::unique_ptr<SourceCompilerDriver> srcCompilerDriver;

        // TODO: Replace with unique_ptr to ModuleWithContext
        // Pointer to the context of the loaded llvm module
        std::unique_ptr<llvm::LLVMContext> irContext;

        // Pointer to the SMDagnostics associated with the loaded llvm module
        std::unique_ptr<llvm::SMDiagnostic> irSMDiag;

        // Pointer to the loaded llvm-ir module
        std::unique_ptr<llvm::Module> irModule;

        /**
         * Pointers to stuff that is required to keep the parse-tree of the source-file in scope.
         */
        std::unique_ptr<std::ifstream> srcFileStream;
        std::unique_ptr<antlr4::ANTLRInputStream> srcInStream;
        std::unique_ptr<SpecLexer> srcLexer;
        std::unique_ptr<antlr4::CommonTokenStream> srcTokens;
        std::unique_ptr<SpecParser> srcParser;
        SpecParser::FileContext *srcParseTree;

        /*
         * Name of the function in the source-language to which the currently visited contract belongs.
         */
        std::optional<std::string> currentSrcContext = std::nullopt;


        void transformExpression(std::string expr, std::string srcFunctionName);


        /**
         * Retrieving the textual representation of the specification expressions using getText() removes the whitespaces.
         * This method returns the full textual representatoin including whitespaces by using the underlying token-stream.
         */
        std::string getExprAsStr(const SpecParser::SpecExpressionContext &exprCtx);

        /**
         * Takes a string that contains a source-language expression along with a map that maps the names of source-variables 
         * to their types and generates a string-representation of a function in the source-language.
         * 
         * TODO: Move somewhere else as this is source-language specific
         */
        std::string getWrappingFunction(const llvm::StringMap<std::string> &srcVarsToTypes, const std::string &expr);

        /**
         * Takes a string-representation of a source-labguage function and turns it into a full compilation unit in 
         * the source language.
         * E.g. adds imports for the specification library, ...
         * 
         * TODO: Move somewhere else as this is source-language specific
         */
        std::string assembleSrcCompilationUnit(std::string wrapperFunction);
    };

}

#endif // PALLAS_SOURCESPECTRANSFORMER_H