// swift-tools-version: 5.10

import PackageDescription

let package = Package(
    name: "PallasSpec",
    
    // platforms: [ .macOS(.v13)],

    products: [
        .library(
            name: "PallasSpec",
            targets: ["PallasSpec"]),
    ],

    targets: [
        
        // Specification-library
        .target(
            name: "PallasSpec"
        ),
    ]
)
