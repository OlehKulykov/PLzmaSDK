// swift-tools-version: 6.1

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
                    .define("LIBPLZMA_VERSION_BUILD", to: "1356"),
                    .unsafeFlags(["-fPIC"])
                ],
                cxxSettings: [
                    .define("LIBPLZMA_VERSION_BUILD", to: "1356"),
                    .unsafeFlags(["-fPIC", "-fno-rtti"])
                ]),
        .target(name: "PLzmaSDK",
                dependencies: [
                    .target(name: "libplzma")
                ],
                path: "swift",
                resources: [
                    .copy("../resources/PrivacyInfo.xcprivacy"),
                ]),
        .target(name: "plzma_test_files",
                path: "test_files",
                cSettings: [
                    .unsafeFlags(["-fPIC"])
                ],
                cxxSettings: [
                    .unsafeFlags(["-fPIC", "-fno-rtti"])
                ]),
        .testTarget(
            name: "PLzmaSDKTests",
            dependencies: [
                .target(name: "PLzmaSDK"),
                .target(name: "libplzma"),
                .target(name: "plzma_test_files")
            ],
            path: "swift_pm_tests")
    ],
    swiftLanguageModes: [.v5, .v6],
    cLanguageStandard: .c11,
    cxxLanguageStandard: .cxx11
)
