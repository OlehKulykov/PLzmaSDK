// swift-tools-version:5.0

import PackageDescription

let package = Package(
    name: "PLzmaSDK",
    products: [
        .library(name: "PLzmaSDK", targets: ["PLzmaSDK"]),
        .library(name: "PLzmaSDK-Static", type: .static, targets: ["PLzmaSDK"]),
        .library(name: "PLzmaSDK-Dynamic", type: .dynamic, targets: ["PLzmaSDK"])
    ],
    targets: [
        .target(name: "libplzma",
                path: "src",
                cSettings: [
                    .define("LIBPLZMA_VERSION_BUILD", to: "368")
                ],
                cxxSettings: [
                    .define("LIBPLZMA_VERSION_BUILD", to: "368")
        ]),
        .target(name: "PLzmaSDK",
                dependencies: [
                    .target(name: "libplzma")
                ],
                path: "swift"),
        .target(name: "plzma_test_files",
                path: "test_files"),
        .testTarget(
            name: "PLzmaSDKTests",
            dependencies: [
                .target(name: "PLzmaSDK"),
                .target(name: "libplzma"),
                .target(name: "plzma_test_files")
            ],
            path: "swift_pm_tests")
    ],
    swiftLanguageVersions: [.v5],
    cLanguageStandard: .c11,
    cxxLanguageStandard: .cxx11
)
