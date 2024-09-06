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

    dependencies: [
        .package(url: "https://github.com/apple/swift-syntax", from: "510.0.0")
    ],

    targets: [
        
        // Specification-library
        .target(
            name: "PallasSpec"
        ),
    ]
)
