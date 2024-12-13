import XCTest
@testable import PLzmaSDK
@testable import libplzma
@testable import plzma_test_files

final class Compress: XCTestCase, EncoderDelegate, DecoderDelegate {
    
    func decoder(decoder: Decoder, path: String, progress: Double) {
        print("Progress, path: \(path), value: \(progress)")
    }
    
    func encoder(encoder: Encoder, path: String, progress: Double) {
        print("Progress, path: \(path), value: \(progress)")
    }
    
    enum TestMultiVolumeCase: Int {
        case fileNoPassword = 0 , memNoPassword, fileWithPassword, memWithPassword
    };
    
    func testMultiVolume() {
        do {
            for useCaseIndex in 0..<4 {
                guard let testCase = TestMultiVolumeCase(rawValue: useCaseIndex) else { continue }
                
                let path = try Path.tmpDir()
                try path.appendRandomComponent()
                let partSize = Size(32 * 1024)
                let multiStream: OutMultiStream
                switch testCase {
                case .fileNoPassword, .fileWithPassword:
                    multiStream = try OutMultiStream(directoryPath: path,
                                                     partName: "file",
                                                     partExtension: "7z",
                                                     format: .nameExt00x,
                                                     partSize: partSize)
                case .memNoPassword, .memWithPassword:
                    multiStream = try OutMultiStream(partSize: partSize)
                }
                
                let encoder = try Encoder(stream: multiStream, fileType: .sevenZ, method: .LZMA, delegate: self)
                switch testCase {
                case .fileWithPassword, .memWithPassword:
                    try encoder.setPassword("hello")
                default:
                    break
                }
                
                try encoder.add(stream: try InStream(dataNoCopy: Data(bytesNoCopy: FILE__shutuptakemoney_jpg_PTR, count: Int(FILE__shutuptakemoney_jpg_SIZE), deallocator: .none)),
                                archivePath: try Path("shutuptakemoney.jpg"))
                try encoder.add(stream: try InStream(dataNoCopy: Data(bytesNoCopy: FILE__southpark_jpg_PTR, count: Int(FILE__southpark_jpg_SIZE), deallocator: .none)),
                                archivePath: try Path("SouthPark.jpg"))
                try encoder.add(stream: try InStream(dataNoCopy: Data(bytesNoCopy: FILE__zombies_jpg_PTR, count: Int(FILE__zombies_jpg_SIZE), deallocator: .none)),
                                archivePath: try Path("zombies.jpg"))
                try encoder.add(stream: try InStream(dataNoCopy: Data(bytesNoCopy: FILE__munchen_jpg_PTR, count: Int(FILE__munchen_jpg_SIZE), deallocator: .none)),
                                archivePath: try Path("Мюнхен.jpg"))
                try encoder.add(stream: try InStream(dataNoCopy: Data(bytesNoCopy: FILE__munchen_jpg_PTR, count: Int(FILE__munchen_jpg_SIZE), deallocator: .none)),
                                archivePath: try Path("München.jpg"))
                
                var opened = try encoder.open()
                XCTAssertTrue(opened)
                
                let compressed = try encoder.compress()
                XCTAssertTrue(compressed)
                
                try encoder.abort()
                
                let content = try multiStream.copyContent()
                var totalPartsSize = UInt64(0)
                switch testCase {
                case .fileNoPassword, .fileWithPassword:
                    let pathIterator = try path.openDir()
                    while try pathIterator.next() {
                        let fullPath = try pathIterator.fullPath()
                        print("\(fullPath.description)")
                        let partStat = try fullPath.stat()
                        totalPartsSize += partStat.size
                    }
                    XCTAssertTrue(content.count == totalPartsSize)
                default:
                    break
                }
                
                let outMultiStreams = try multiStream.streams()
                XCTAssertTrue(outMultiStreams.count > 0)

                totalPartsSize = 0
                var i = 0
                for outMultiStream in outMultiStreams {
                    let partContent = try outMultiStream.copyContent()
                    if i + 1 == outMultiStreams.count {
                        XCTAssertTrue(partContent.count <= partSize)
                    } else {
                        XCTAssertTrue(partContent.count == partSize)
                    }
                    totalPartsSize += UInt64(partContent.count)
                    i += 1
                }
                
                XCTAssertTrue(content.count == totalPartsSize)
                var erased = try multiStream.erase()
                XCTAssertTrue(erased)
                
                let decoder = try Decoder(stream: try InStream(dataNoCopy: content), fileType: .sevenZ, delegate: self)
                switch testCase {
                case .fileWithPassword, .memWithPassword:
                    try decoder.setPassword("hello")
                    break
                default:
                    break
                }
                
                opened = try decoder.open()
                XCTAssertTrue(opened)
                
                let itemsCount = try decoder.count()
                XCTAssertTrue(itemsCount == 5)
                
                let memoryMap = try ItemOutStreamArray(capacity: 5)
                let fileMap = try ItemOutStreamArray(capacity: 5)
                i = 0
                while i < 5 {
                    try memoryMap.add(item: try decoder.item(at: Size(i)), stream: try OutStream())
                    let tmpPath = try Path.tmpDir()
                    try tmpPath.appendRandomComponent()
                    try fileMap.add(item: try decoder.item(at: Size(i)), stream: try OutStream(movablePath: tmpPath))
                    i += 1
                }
                XCTAssertTrue(memoryMap.count == 5)
                XCTAssertTrue(fileMap.count == 5)
                
                var extracted = try decoder.extract(itemsToStreams: memoryMap)
                XCTAssertTrue(extracted)
                extracted = try decoder.extract(itemsToStreams: fileMap)
                XCTAssertTrue(extracted)
                
                i = 0
                while i < 10 {
                    let pair = i < 5 ? try memoryMap.pair(at: Size(i)) : try fileMap.pair(at: Size(i - 5))
                    let content = try pair.1.copyContent()
                    erased = try pair.1.erase(erase: .zero)
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
                    i += 1
                }
               
                try decoder.abort()
                try path.remove()
                
                i = 0
                while i < 5 {
                    erased = try memoryMap.pair(at: Size(i)).1.erase()
                    XCTAssertTrue(erased)
                    erased = try fileMap.pair(at: Size(i)).1.erase()
                    XCTAssertTrue(erased)
                    i += 1
                }
            }
        } catch let exception as Exception {
            print("Exception: \(exception)")
        } catch let error {
            print("Exception error: \(error)")
        }
    }
    
    static var allTests = [
        ("testMultiVolume", testMultiVolume)
    ]
}
