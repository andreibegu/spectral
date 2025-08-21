# Script to measure the lines of code, split by the  language frontends 
# Exclude:
# - Build-system files & (environment) configurations
# - Generated files
# - Temporary files
# - Documentation
# - Examples 
# - Dependencies
# - Tests

CLOC="cloc --exclude-lang=CMake"

echo "========================================================================="
echo "========================================================================="
echo "Counting LOC for C frontend"

# Compiler Driver
echo "Compiler driver: "
$CLOC \
    include/C/CCompilerDriver.h \
    lib/C/CCompilerDriver.cpp

# Wrapper Generator
echo "Wrapper Generator: "
$CLOC \
    include/C/CWrapperGenerator.h \
    lib/C/CWrapperGenerator.cpp

# Spec Unifier 
echo "Spec Unifier: "
$CLOC \
    include/C/CSpecUnifier.h \
    lib/C/CSpecUnifier.cpp

# Other 
echo "Other: "
$CLOC \
    include/C/CSpecTransformer.h \
    lib/C/CSpecTransformer.cpp

# Spec library 
echo "Spec. library: "
$CLOC \
    res/spec_libs/c


echo "========================================================================="
echo "========================================================================="
echo "Counting LOC for C++ frontend"

# Compiler Driver
echo "Compiler driver: "
$CLOC \
    include/Cpp/CppCompilerDriver.h \
    lib/Cpp/CppCompilerDriver.cpp

# Wrapper Generator
echo "Wrapper Generator: "
$CLOC \
    include/Cpp/CppWrapperGenerator.h \
    lib/Cpp/CppWrapperGenerator.cpp

# Spec Unifier 
echo "Spec Unifier: "
$CLOC \
    include/Cpp/CppSpecUnifier.h \
    lib/Cpp/CppSpecUnifier.cpp

# Other 
echo "Other: "
$CLOC \
    include/Cpp/CppSpecTransformer.h \
    lib/Cpp/CppSpecTransformer.cpp

# Spec library 
echo "Spec. library: "
$CLOC \
    res/spec_libs/cpp


echo "========================================================================="
echo "========================================================================="
echo "Counting LOC for Swift frontend"

# Compiler Driver
echo "Compiler driver: "
$CLOC \
    include/swift/SwiftCompilerDriver.h \
    lib/swift/SwiftCompilerDriver.cpp

# Wrapper Generator
echo "Wrapper Generator: "
$CLOC \
    include/swift/SwiftWrapperGenerator.h \
    lib/swift/SwiftWrapperGenerator.cpp

# Spec Unifier 
echo "Spec Unifier: "
$CLOC \
    include/swift/SwiftSpecUnifier.h \
    lib/swift/SwiftSpecUnifier.cpp

# Other 
echo "Other: "
$CLOC \
    include/swift/SwiftNameConstants.h \
    include/swift/SwiftSpecTransformer.h \
    lib/swift/SwiftSpecTransformer.cpp

# Spec library 
echo "Spec. library: "
$CLOC \
    res/spec_libs/swift/PallasSpec/Package.swift \
    res/spec_libs/swift/PallasSpec/Sources


echo "========================================================================="
echo "========================================================================="
echo "Counting LOC for core framework"

# Compiler Driver
echo "Compiler driver: "
$CLOC \
    include/core/SourceCompilerDriver.h \
    lib/core/SourceCompilerDriver.cpp

# Wrapper Generator
echo "Wrapper Generator: "
$CLOC \
    include/core/SourceWrapperGenerator.h \
    lib/core/SourceWrapperGenerator.cpp

# Spec Unifier 
echo "Spec Unifier: "
$CLOC \
    include/core/SpecUnifier.h \
    lib/core/SpecUnifier.cpp

# Other 
echo "Other: "
$CLOC \
    grammar/SpecLexer.g4 \
    grammar/SpecParser.g4 \
    include/core/passes \
    include/core/wrapper \
    include/core/IRBasedSourceSpecMatcher.h \
    include/core/MatchedSourceSpec.h \
    include/core/SourceSpecMatcher.h \
    include/core/SourceSpecTransformer.h \
    include/core/SpecDeclaration.h \
    include/core/SpecDeclarationCollector.h \
    include/utils \
    lib/core/passes \
    lib/core/wrapper \
    lib/core/IRBasedSourceSpecMatcher.cpp \
    lib/core/SourceSpecMatcher.cpp \
    lib/core/SourceSpecTransformer.cpp \
    lib/core/SpecDeclarationCollector.cpp \
    lib/utils \
    tools


echo "WARNING: LOC for core are incomplete!"
# TODO: Make sure that all files are included! 