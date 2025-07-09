import XCTest
@testable import PLzmaSDK
@testable import libplzma

final class TypesTests: XCTestCase {
    
    func testErrorCode() {
        // unknown
        XCTAssertEqual(ErrorCode.unknown.type, plzma_error_code_unknown)
        XCTAssertEqual(UInt32(ErrorCode.unknown.rawValue), plzma_error_code_unknown.rawValue)
        XCTAssertEqual(ErrorCode.unknown, plzma_error_code(rawValue: UInt32(ErrorCode.unknown.rawValue)).type)
        
        // invalidArguments
        XCTAssertEqual(ErrorCode.invalidArguments.type, plzma_error_code_invalid_arguments)
        XCTAssertEqual(UInt32(ErrorCode.invalidArguments.rawValue), plzma_error_code_invalid_arguments.rawValue)
        XCTAssertEqual(ErrorCode.invalidArguments, plzma_error_code(rawValue: UInt32(ErrorCode.invalidArguments.rawValue)).type)
        
        // notEnoughMemory
        XCTAssertEqual(ErrorCode.notEnoughMemory.type, plzma_error_code_not_enough_memory)
        XCTAssertEqual(UInt32(ErrorCode.notEnoughMemory.rawValue), plzma_error_code_not_enough_memory.rawValue)
        XCTAssertEqual(ErrorCode.notEnoughMemory, plzma_error_code(rawValue: UInt32(ErrorCode.notEnoughMemory.rawValue)).type)
        
        // io
        XCTAssertEqual(ErrorCode.io.type, plzma_error_code_io)
        XCTAssertEqual(UInt32(ErrorCode.io.rawValue), plzma_error_code_io.rawValue)
        XCTAssertEqual(ErrorCode.io, plzma_error_code(rawValue: UInt32(ErrorCode.io.rawValue)).type)
                
        // internal
        XCTAssertEqual(ErrorCode.internal.type, plzma_error_code_internal)
        XCTAssertEqual(UInt32(ErrorCode.internal.rawValue), plzma_error_code_internal.rawValue)
        XCTAssertEqual(ErrorCode.internal, plzma_error_code(rawValue: UInt32(ErrorCode.internal.rawValue)).type)
    }
    
    func testErase() {
        // none
        XCTAssertEqual(Erase.none.type, plzma_erase_none)
        XCTAssertEqual(UInt32(Erase.none.rawValue), plzma_erase_none.rawValue)
        XCTAssertEqual(Erase.none, plzma_erase(rawValue: UInt32(Erase.none.rawValue)).type)
        
        // zero
        XCTAssertEqual(Erase.zero.type, plzma_erase_zero)
        XCTAssertEqual(UInt32(Erase.zero.rawValue), plzma_erase_zero.rawValue)
        XCTAssertEqual(Erase.zero, plzma_erase(rawValue: UInt32(Erase.zero.rawValue)).type)
    }
    
    func testFileType() {
        // 7z
        XCTAssertEqual(FileType.sevenZ.type, plzma_file_type_7z)
        XCTAssertEqual(UInt32(FileType.sevenZ.rawValue), plzma_file_type_7z.rawValue)
        XCTAssertEqual(FileType.sevenZ, plzma_file_type(rawValue: UInt32(FileType.sevenZ.rawValue)).type)
        
        // xz
        XCTAssertEqual(FileType.xz.type, plzma_file_type_xz)
        XCTAssertEqual(UInt32(FileType.xz.rawValue), plzma_file_type_xz.rawValue)
        XCTAssertEqual(FileType.xz, plzma_file_type(rawValue: UInt32(FileType.xz.rawValue)).type)
        
        // tar
        XCTAssertEqual(FileType.tar.type, plzma_file_type_tar)
        XCTAssertEqual(UInt32(FileType.tar.rawValue), plzma_file_type_tar.rawValue)
        XCTAssertEqual(FileType.tar, plzma_file_type(rawValue: UInt32(FileType.tar.rawValue)).type)
    }
    
    static let allTests = [
        ("ErrorCode enum tests", testErrorCode),
        ("Erase enum tests", testErase),
        ("FileType enum tests", testFileType)
    ]
}
