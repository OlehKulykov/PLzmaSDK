import XCTest
@testable import PLzmaSDK
@testable import libplzma

final class Global: XCTestCase {
    
    func testReadWriteGlobalProperties() {
        XCTAssertTrue(PLzmaSDK.streamReadSize > 0, "Wrong initial 'streamReadSize' value.")
        XCTAssertTrue(PLzmaSDK.streamWriteSize > 0, "Wrong initial 'streamWriteSize' value.")
        XCTAssertTrue(PLzmaSDK.decoderReadSize > 0, "Wrong initial 'decoderReadSize' value.")
        XCTAssertTrue(PLzmaSDK.decoderWriteSize > 0, "Wrong initial 'decoderWriteSize' value.")
        
        PLzmaSDK.streamReadSize = 3456
        XCTAssertTrue(PLzmaSDK.streamReadSize == 3456, "Wrong 'streamReadSize' value after update.")

        PLzmaSDK.streamWriteSize = 3456
        XCTAssertTrue(PLzmaSDK.streamWriteSize == 3456, "Wrong 'streamWriteSize' value after update.")
        
        PLzmaSDK.decoderReadSize = 3456
        XCTAssertTrue(PLzmaSDK.decoderReadSize == 3456, "Wrong 'decoderReadSize' value after update.")
        
        PLzmaSDK.decoderWriteSize = 3456
        XCTAssertTrue(PLzmaSDK.decoderWriteSize == 3456, "Wrong 'streamReadSize' value after update.")
    }
    
    static var allTests = [
        ("Retrieve and change global properties", testReadWriteGlobalProperties)
    ]
}
