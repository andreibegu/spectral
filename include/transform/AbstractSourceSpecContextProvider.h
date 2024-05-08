#ifndef PALLAS_SOURCESPECCONTEXTPROVIDER_H
#define PALLAS_SOURCESPECCONTEXTPROVIDER_H

#include <string>

namespace sourcespec2ir
{

    /**
     * Abstract class that defines the interface for providing the context for
     * specifications in the source file.
     * The class has to be implemented for every supported source language.
     */
    class AbstractSourceSpecContextProvider
    {
    public:
        /**
         * Given the location of a specContractBlock in the source-file, this mehtod
         * returns the name of the funcion/ method that the specification refers to. 
         */
        virtual std::string getContractBlockContext(size_t startLine, size_t startCol, size_t endLine, size_t endCol) = 0;

        // TODO: Extend to include statement-blocks etc.

    };

}

#endif // PALLAS_SOURCESPECCONTEXTPROVIDER_H