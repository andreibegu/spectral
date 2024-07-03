#ifndef PALLAS_CPPSPECTRANSFORMER_H
#define PALLAS_CPPSPECTRANSFORMER_H

#include "SourceSpecTransformer.h"

namespace sourcespec2ir
{

    /**
     * Specialization of the SourceSpecTransformer for C++.
     */
    class CppSpecTransformer : public SourceSpecTransformer
    {
    public:
        CppSpecTransformer(
            std::string specLibPath,
            std::string srcInPath, std::string outPath,
            const utils::OutputConfig &outputConfig);
    protected:
        std::unique_ptr<SourceWrapperGenerator> createWrapperGenerator() override;
    };

}

#endif // PALLAS_CPPSPECTRANSFORMER_H