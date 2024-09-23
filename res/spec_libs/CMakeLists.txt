# Target that rebuilds the specification-library for swift
add_custom_target(BuildSwiftLib # ALL
    COMMAND swift build --package-path ${CMAKE_SOURCE_DIR}/res/spec_libs/swift/PallasSpec
    COMMENT "Building swift specification library"
)

# Alternatively, we could use swiftc directly
# COMMAND swiftc -emit-library -emit-module -module-name PallasSpec -o MyModule.a MyModule/Sources/*.swift
