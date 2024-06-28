#include "MetadataUtils.h"
#include "IrSpecConstants.h"

namespace sourcespec2ir
{
    namespace utils
    {

        void markGeneratedSpecFunction(llvm::Function &f, llvm::LLVMContext &context)
        {
            auto idStr = llvm::MDString::get(context, "");
            f.setMetadata(PALLAS_GENERATED_MARKER, llvm::MDNode::get(context, idStr));
        }

        bool isGeneratedSpecFunction(const llvm::Function &f) 
        {
            return f.getMetadata(PALLAS_GENERATED_MARKER) != nullptr;
        }

    }

}