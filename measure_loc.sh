# Script to measure the lines of code, split by the  language frontends 

echo "========================================================================="
echo "========================================================================="
echo "Counting LOC for C frontend"

# Compiler Driver
echo "Compiler driver: "
cloc \
    include/C/CCompilerDriver.h \
    lib/C/CCompilerDriver.cpp

# Wrapper Generator
echo "Wrapper Generator: "
cloc \
    include/C/CWrapperGenerator.h \
    lib/C/CWrapperGenerator.cpp

# Spec Unifier 
echo "Spec Unifier: "
cloc \
    include/C/CSpecUnifier.h \
    lib/C/CSpecUnifier.cpp

# Other 
echo "Other: "
cloc \
    include/C/CSpecTransformer.h \
    lib/C/CSpecTransformer.cpp

# Spec library 
echo "Spec. library: "
cloc \
    res/spec_libs/c


echo "========================================================================="
echo "========================================================================="
echo "Counting LOC for C++ frontend"

# Compiler Driver
echo "Compiler driver: "
cloc \
    include/Cpp/CppCompilerDriver.h \
    lib/Cpp/CppCompilerDriver.cpp

# Wrapper Generator
echo "Wrapper Generator: "
cloc \
    include/Cpp/CppWrapperGenerator.h \
    lib/Cpp/CppWrapperGenerator.cpp

# Spec Unifier 
echo "Spec Unifier: "
cloc \
    include/Cpp/CppSpecUnifier.h \
    lib/Cpp/CppSpecUnifier.cpp

# Other 
echo "Other: "
cloc \
    include/Cpp/CppSpecTransformer.h \
    lib/Cpp/CppSpecTransformer.cpp

# Spec library 
echo "Spec. library: "
cloc \
    res/spec_libs/cpp


echo "========================================================================="
echo "========================================================================="
echo "Counting LOC for Swift frontend"

# Compiler Driver
echo "Compiler driver: "
cloc \
    include/swift/SwiftCompilerDriver.h \
    lib/swift/SwiftCompilerDriver.cpp

# Wrapper Generator
echo "Wrapper Generator: "
cloc \
    include/swift/SwiftWrapperGenerator.h \
    lib/swift/SwiftWrapperGenerator.cpp

# Spec Unifier 
echo "Spec Unifier: "
cloc \
    include/swift/SwiftSpecUnifier.h \
    lib/swift/SwiftSpecUnifier.cpp

# Other 
echo "Other: "
cloc \
    include/swift/SwiftNameConstants.h \
    include/swift/SwiftSpecTransformer.h \
    lib/swift/SwiftSpecTransformer.cpp

# Spec library 
echo "Spec. library: "
cloc \
    res/spec_libs/swift/PallasSpec/Package.swift \
    res/spec_libs/swift/PallasSpec/Sources

