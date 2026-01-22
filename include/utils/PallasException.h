#ifndef PALLAS_EXCEPTION_H
#define PALLAS_EXCEPTION_H

#include <exception>
#include <string>

namespace sourcespec2ir
{
    namespace utils
    {
        /**
         * Super-class for exceptions that are thrown by the Pallas
         * Spec2IR-translation.
         */
        class PallasException : std::exception
        {
        public:
            PallasException(const std::string &msg) : msg(msg) {}

            const char *what() const noexcept override
            {
                return msg.c_str();
            }

        private:
            std::string msg;
        };

        class PallasNotImplementedException : PallasException
        {
        public:
            PallasNotImplementedException()
                : PallasException("Not Implemented!")
            {
            }
        };

    }
}

#endif // PALLAS_EXCEPTION_H