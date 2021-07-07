import XCTest
@testable import PLzmaSDK
@testable import libplzma
@testable import plzma_test_files

final class plzmaTests: XCTestCase, DecoderDelegate {
    let queue = OperationQueue()
    
    func decoder(decoder: Decoder, path: String, progress: Double) {
        print("Progress, path: \(path), value: \(progress)")
        if progress > 0.1 {
//            try! decoder.abort()
        }
    }
    
    func testOpen1() {
        var stream = plzma_in_stream_create_with_memory(FILE__1_7z_PTR!, Int(FILE__1_7z_SIZE)) { _ in
            
        }
        //TODO: continue
        plzma_in_stream_release(&stream)
    }
    
    func testExample() {
        print("Version: \(PLzmaSDK.version)")
        do {
            var path = try Path("1\\3")
            try path &= "5\\6"
            path = try Path(path: path)
            path = try Path("2")
            path = try Path("")
            try path += "4"
            try path += Path("5")
            try path += ""
            try path += "6"
            path = try Path.tmpDir()
            var isDir = false
            try path.exists(isDir: &isDir)
            var _ = try! path.stat()
            let iterator = try! path.openDir(mode: .followSymlinks)
            let p1 = try! iterator.path()
            print("path: \(p1)")
            while try! iterator.next() {
                print("path: \(try! iterator.path())")
            }
            try path.appendRandomComponent()
            try path += ""
            var item = try Item("hello\\world", index: 1)
            var items = [item]
            item = try Item(movablePath: path, index: 0)
            items.append(item)
            let _ = try ItemArray(items: items)
            item = try Item(path: path, index: 0)
            var data = Data()
            data.append(1)
            let data1 = Data(bytesNoCopy: FILE__1_7z_PTR!, count: Int(FILE__1_7z_SIZE), deallocator: .none)
            //data1 = try Data(contentsOf: try URL(fileURLWithPath: "install-x86-minimal-20190510T214503Z.iso.7z"))
            let decoder = try Decoder(stream: try InStream(dataNoCopy: data1), fileType: .sevenZ, delegate: self)
            let _ = try decoder.open()
            var time = Date().timeIntervalSince1970
            let blockOperation = BlockOperation(block: {
                let _ = try? decoder.test()
                return
            })
            queue.addOperation(blockOperation)
            queue.waitUntilAllOperationsAreFinished()
            time = Date().timeIntervalSince1970 - time
            print("Work time: \(time)")
        } catch let exception as Exception {
            print("Exception: \(exception)")
        } catch let error {
            print("Exception error: \(error)")
        }
    }

    func testEncoder() {
        do {
            let stream = try OutStream()
            let encoder = try Encoder(stream: stream, fileType: .sevenZ, method: .LZMA2)
            var boolVal = try encoder.shouldCreateSolidArchive()
            XCTAssertTrue(boolVal)
            boolVal = try encoder.shouldCreateSolidArchive()
            XCTAssertTrue(boolVal)
        } catch let exception as Exception {
            print("Exception: \(exception)")
        } catch let error {
            print("Exception error: \(error)")
        }
    }
    
    static var allTests = [
        ("testExample", testExample),
        ("testOpen1", testOpen1),
        ("testEncoder", testEncoder)
    ]
}
