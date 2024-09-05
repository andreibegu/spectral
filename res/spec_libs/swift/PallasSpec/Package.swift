// swift-tools-version: 5.10

import PackageDescription
import CompilerPluginSupport

let package = Package(
    name: "PallasSpec",
    
    // platforms: [ .macOS(.v13)],

    products: [
        // Products define the executables and libraries a package produces, making them visible to other packages.
        .library(
            name: "PallasSpec",
            targets: ["PallasSpec", "PallasSpecMacroInterface"]),
    ],

    dependencies: [
        .package(url: "https://github.com/apple/swift-syntax", from: "510.0.0")
    ],

    targets: [
        
        // Implementation of the specification-library's macros
        .macro(
            name: "PallasSpecMacroImpl",
            dependencies: [
                .product(name: "SwiftSyntaxMacros", package: "swift-syntax"),
                .product(name: "SwiftCompilerPlugin", package: "swift-syntax")
            ]
        ),

        // Declarations of the specification-library's macros
        .target(
            name: "PallasSpecMacroInterface",
            dependencies: ["PallasSpecMacroImpl"]
        ),
        
        // Specification-library
        .target(
            name: "PallasSpec", 
            dependencies: ["PallasSpecMacroInterface"]
        ),
    ]
)
