import XCTest
@testable import PLzmaSDK
@testable import libplzma
@testable import plzma_test_files

final class OpenAndList: XCTestCase, DecoderDelegate {

    func decoder(decoder: Decoder, path: String, progress: Double) {
        print("Progress, path: \(path), value: \(progress)")
    }

    func testOpen() {
        do {
            let archivePath = try Path("path/to/archive")
            let archivePathInStream = try InStream(path: archivePath)
            
            let archiveData = Data(bytesNoCopy: FILE__7_7z_PTR!, count: Int(FILE__7_7z_SIZE), deallocator: .none)
            let archiveDataInStream = try InStream(dataNoCopy: archiveData)
            
            let decoder = try Decoder(stream: archiveDataInStream /* archivePathInStream */, fileType: .sevenZ, delegate: self)
            try decoder.setPassword("1234")
            let opened = try decoder.open()
            XCTAssertTrue(opened)
            let numberOfArchiveItems = try decoder.count()
            XCTAssertTrue(numberOfArchiveItems == 5)
            let allArchiveItems = try decoder.items()
            let selectedItemsDuringIteration = try ItemArray(capacity: numberOfArchiveItems)
            for itemIndex in 0..<numberOfArchiveItems {
                let item = try decoder.item(at: itemIndex)
                try selectedItemsDuringIteration.add(item: item)
            }
            XCTAssertTrue(numberOfArchiveItems == allArchiveItems.count)
            XCTAssertTrue(numberOfArchiveItems == selectedItemsDuringIteration.count)
            
        } catch let exception as Exception {
            print("Exception: \(exception)")
        } catch let error {
            print("Exception error: \(error)")
        }
    }
    
    static var allTests = [
        ("testOpen", testOpen)
    ]
}
