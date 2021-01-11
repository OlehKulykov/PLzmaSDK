import XCTest
@testable import PLzmaSDK
@testable import libplzma
@testable import plzma_test_files

final class Compress: XCTestCase, EncoderDelegate {
    
    func encoder(encoder: Encoder, path: String, progress: Double) {
        print("Progress, path: \(path), value: \(progress)")
    }
    
    func testCompress() {
        do {
            
        } catch let exception as Exception {
            print("Exception: \(exception)")
        } catch let error {
            print("Exception error: \(error)")
        }
    }
    
    static var allTests = [
        ("testCompress", testCompress)
    ]
}

