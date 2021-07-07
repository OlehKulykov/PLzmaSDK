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
            //let archivePath = try Path("path/to/archive")
            //let archivePathInStream = try InStream(path: archivePath)
            
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
            let selectedItemsToStreams = try ItemOutStreamArray()
            for itemIndex in 0..<numberOfArchiveItems {
                let item = try decoder.item(at: itemIndex)
                try selectedItemsDuringIteration.add(item: item)
                try selectedItemsToStreams.add(item: item, stream: OutStream())
            }
            XCTAssertTrue(numberOfArchiveItems == allArchiveItems.count)
            XCTAssertTrue(numberOfArchiveItems == selectedItemsDuringIteration.count)
            
            let extracted = try decoder.extract(itemsToStreams: selectedItemsToStreams)
            XCTAssertTrue(extracted)
            
        } catch let exception as Exception {
            print("Exception: \(exception)")
            XCTAssertTrue(false)
        } catch let error {
            print("Exception error: \(error)")
            XCTAssertTrue(false)
        }
    }
    
    func testMultiVolume() {
        do {
            var streams: [InStream] = [
                try InStream(dataNoCopy: Data(bytesNoCopy: FILE__18_7z_001_PTR, count: Int(FILE__18_7z_001_SIZE), deallocator: .none))
            ]
            
            let secondPartPath = try Path.tmpDir().appendingRandomComponent()
            XCTAssertFalse(try secondPartPath.exists())
            let secondPartData = Data(bytesNoCopy: FILE__18_7z_002_PTR, count: Int(FILE__18_7z_002_SIZE), deallocator: .none)
            try secondPartData.write(to: URL(fileURLWithPath: secondPartPath.description))
            XCTAssertTrue(try secondPartPath.exists())
            
            streams.append(try InStream(path: secondPartPath))
            streams.append(try InStream(dataNoCopy: Data(bytesNoCopy: FILE__18_7z_003_PTR, count: Int(FILE__18_7z_003_SIZE), deallocator: .none)))
            
            XCTAssertTrue(streams.count == 3)
            
            let decoder = try Decoder(stream: try InStream(streams: streams), fileType: .sevenZ, delegate: self)
            try decoder.setPassword("1234")
            
            XCTAssertTrue(try decoder.open())
            
            let numberOfArchiveItems = try decoder.count()
            XCTAssertTrue(numberOfArchiveItems == 5)

            let memoryMap = try ItemOutStreamArray(capacity: 5)
            let fileMap = try ItemOutStreamArray(capacity: 5)
            
            for itemIndex in 0..<numberOfArchiveItems {
                let item = try decoder.item(at: itemIndex)
                try memoryMap.add(item: item, stream: try OutStream())
                try fileMap.add(item: item, stream: try OutStream())
            }
            
            XCTAssertTrue(memoryMap.count == 5)
            XCTAssertTrue(fileMap.count == 5)

            var extracted = try decoder.extract(itemsToStreams: memoryMap)
            XCTAssertTrue(extracted)
            
            extracted = try decoder.extract(itemsToStreams: fileMap)
            XCTAssertTrue(extracted)
            
            for i in Size(0)..<Size(10) {
                let pair = i < 5 ? try memoryMap.pair(at: i) : try fileMap.pair(at: i - 5)
                let content = try pair.1.copyContent()
                XCTAssertFalse(content.isEmpty)
                XCTAssertTrue(content.count > 0)
                let erased = try pair.1.erase(erase: .zero)
                XCTAssertTrue(erased)
                let itemPathString = try pair.0.path().description
                switch itemPathString {
                case "shutuptakemoney.jpg":
                    XCTAssertTrue(pair.0.size == FILE__shutuptakemoney_jpg_SIZE)
                    XCTAssertTrue(content.count == FILE__shutuptakemoney_jpg_SIZE)
                    XCTAssertTrue(content == Data(bytesNoCopy: FILE__shutuptakemoney_jpg_PTR, count: Int(FILE__shutuptakemoney_jpg_SIZE), deallocator: .none))
                case "SouthPark.jpg":
                    XCTAssertTrue(pair.0.size == FILE__southpark_jpg_SIZE)
                    XCTAssertTrue(content.count == FILE__southpark_jpg_SIZE)
                    XCTAssertTrue(content == Data(bytesNoCopy: FILE__southpark_jpg_PTR, count: Int(FILE__southpark_jpg_SIZE), deallocator: .none))
                case "zombies.jpg":
                    XCTAssertTrue(pair.0.size == FILE__zombies_jpg_SIZE)
                    XCTAssertTrue(content.count == FILE__zombies_jpg_SIZE)
                    XCTAssertTrue(content == Data(bytesNoCopy: FILE__zombies_jpg_PTR, count: Int(FILE__zombies_jpg_SIZE), deallocator: .none))
                case "Мюнхен.jpg", "München.jpg":
                    XCTAssertTrue(pair.0.size == FILE__munchen_jpg_SIZE)
                    XCTAssertTrue(content.count == FILE__munchen_jpg_SIZE)
                    XCTAssertTrue(content == Data(bytesNoCopy: FILE__munchen_jpg_PTR, count: Int(FILE__munchen_jpg_SIZE), deallocator: .none))
                default:
                    XCTAssertTrue(false)
                }
            }

            try secondPartPath.remove()

            XCTAssertTrue(try secondPartPath.remove())
            XCTAssertFalse(try secondPartPath.exists())
        } catch let exception as Exception {
            print("Exception: \(exception)")
            XCTAssertTrue(false)
        } catch let error {
            print("Exception error: \(error)")
            XCTAssertTrue(false)
        }
    }
    
    static var allTests = [
        ("testOpen", testOpen),
        ("testMultivolume", testMultiVolume)
    ]
}
