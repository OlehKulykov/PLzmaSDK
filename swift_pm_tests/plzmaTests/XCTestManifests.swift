import XCTest

#if !canImport(ObjectiveC)
public func allTests() -> [XCTestCaseEntry] {
    return [
        testCase(plzmaTests.allTests),
        testCase(TypesTests.allTests),
        testCase(OpenAndList.allTests)
    ]
}
#endif
