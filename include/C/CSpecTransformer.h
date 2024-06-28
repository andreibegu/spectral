#ifndef PALLAS_CSPECTRANSFORMER_H
#define PALLAS_CSPECTRANSFORMER_H

#include "transform/SourceSpecTransformer.h"

namespace sourcespec2ir
{

    /**
     * Specialization of the SourceSpecTransformer for C.
     */
    class CSpecTransformer : public SourceSpecTransformer
    {
    public:
        CSpecTransformer(
            std::unique_ptr<SourceCompilerDriver> compilerDriver,
            std::unique_ptr<SpecUnifier> specUnifier,
            std::string srcInPath, std::string outPath,
            const utils::OutputConfig &outputConfig);
    protected:
        std::unique_ptr<SourceWrapperGenerator> createWrapperGenerator() override;
    };

}

#endif // PALLAS_CSPECTRANSFORMER_H