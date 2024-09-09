# By using this Software, you are accepting original [LZMA SDK] and MIT license below:
#
# The MIT License (MIT)
#
# Copyright (c) 2015 - 2024 Oleh Kulykov <olehkulykov@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.


cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

project(libplzma LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)

set(PACKAGE "libplzma")
set(CPACK_PACKAGE_NAME "${PACKAGE}")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "4")
set(CPACK_PACKAGE_VERSION_PATCH "7")
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
set(CPACK_PACKAGE_VENDOR "olehkulykov@gmail.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PACKAGE} ${PACKAGE_VERSION}")
set(SOVERSION "1.4.7")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set(VERSION "${CPACK_PACKAGE_VERSION}")

include(CheckLibraryExists)
include(CheckFunctionExists)
include(CheckSymbolExists)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(FindThreads)
include(CheckIncludeFile)

message("CMake version: ${CMAKE_VERSION}")
message("Install path: ${CMAKE_INSTALL_PREFIX}")
message("Binary directory: ${CMAKE_BINARY_DIR}")
message("Library output directory: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
message("Runtime output directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message("CTest binary directory: ${CTEST_BINARY_DIRECTORY}")

# COMMAND LINE OPTIONS
option(LIBPLZMA_OPT_SHARED "Build shared lib." ON)
option(LIBPLZMA_OPT_STATIC "Build static lib." ON)
option(LIBPLZMA_OPT_TESTS "Build libplzma tests." ON)
option(LIBPLZMA_OPT_BUILD_NUMBER "Number of the libplzma build." 0)
option(LIBPLZMA_OPT_ANDROID "Build for Android." OFF)

option(LIBPLZMA_OPT_NO_C_BINDINGS "Do not build C bindings.
The library can't be fully used in C environment." OFF)

option(LIBPLZMA_OPT_NO_CRYPTO "Disable internal crypto functionality.
The library can't read/write password protected archives.
Accessing to crypto functionality will throw an exception!
This option will define 'LIBPLZMA_NO_CRYPTO' preprocessor definition as 'LIBPLZMA_NO_CRYPTO=1'.
Not recommended! But possible. Do this only if you know what are you doing!" OFF)

option(LIBPLZMA_OPT_HAVE_STD "Use std in a private part of the library.
This option will not affects to the public exported interface.
It's recommented to use this option only in a case of building the dynamic library or any other bindings.
This option will define 'LIBPLZMA_HAVE_STD' preprocessor definition as 'LIBPLZMA_HAVE_STD=1',
but also might be defined automatically depending on the build type and/or platform." OFF)

option(LIBPLZMA_OPT_NO_TAR "Do not build tar(tarball) specific sources.
Accessing to Tar archive types will throw an exception! No Tar support." OFF)

option(LIBPLZMA_OPT_NO_PROGRESS "Build without encode/decode progressing.
During the execution, the progress will not be notified to the delegate and C callbacks." OFF)

option(LIBPLZMA_OPT_THREAD_UNSAFE "Removes all thread synchronization functionality from the library.
All properties and methods are thread unsafe." OFF)

option(LIBPLZMA_OPT_DISABLE_RUNTIME_TYPE_INFORMATION "Disable generation of run-time type information about every class.
More info: https://docs.microsoft.com/en-us/cpp/build/reference/gr-enable-run-time-type-information
More info: https://gcc.gnu.org/onlinedocs/gcc-9.2.0/gcc/C_002b_002b-Dialect-Options.html#C_002b_002b-Dialect-Options" ON)

option(LIBPLZMA_OPT_MSVC_MULTITHREAD_STATIC "Microsoft C++ (MSVC) C and C++ compilers and linker uses the multithread,
static version of the run-time library. Adds '/MT' or '/MTd' if selected, otherwise '/MD' or '/MDd'.
More info: https://docs.microsoft.com/en-us/cpp/build/reference/md-mt-ld-use-run-time-library?view=vs-2019" ON)


# ---- definitions ----
if (WIN32)
  add_definitions(-DUNICODE=1)
  add_definitions(-D_UNICODE=1)
endif()

if (MINGW)
  add_definitions(-DLIBPLZMA_MINGW=1)
  message("MinGW")
endif()

if (MSVC)
  # Run-Time Library
  # https://learn.microsoft.com/en-us/cpp/build/reference/md-mt-ld-use-run-time-library
  if (LIBPLZMA_OPT_MSVC_MULTITHREAD_STATIC)
    if (CMAKE_BUILD_TYPE MATCHES Debug)
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MTd")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MTd")
    else()
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MT")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MT")
    endif()
  else()
    if (CMAKE_BUILD_TYPE MATCHES Debug)
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MDd")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MDd")
    else()
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MD")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MD")
    endif()
  endif()

  # Always use UTF8 for sources and execution
  # https://docs.microsoft.com/en-us/cpp/build/reference/utf-8-set-source-and-executable-character-sets-to-utf-8
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")

  if (LIBPLZMA_OPT_DISABLE_RUNTIME_TYPE_INFORMATION)
    # Disable Run-Time Type Information
    # Defined as 1 if the /GR (Enable Run-Time Type Information) compiler option is set. Otherwise, undefined.
    # When /GR is on, the compiler defines the _CPPRTTI preprocessor macro.
    # By default, /GR is on. /GR- disables run-time type information.
    # However, /GR increases the size of the .rdata sections of your image.
    # If your code does not use dynamic_cast or typeid, /GR- may produce a smaller image.
    # https://learn.microsoft.com/en-us/cpp/build/reference/gr-enable-run-time-type-information
    # https://docs.microsoft.com/en-us/cpp/build/reference/gr-enable-run-time-type-information
    # Check vs version: ?view=vs-2015
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /GR-")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-")
    add_definitions(-DLIBPLZMA_NO_CPP_RTTI=1)
  endif()

else()

  # C with -fPIC
  check_c_compiler_flag("-fPIC" WITH_FPIC_C)
  if (WITH_FPIC_C)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
  endif()

  # CXX with -fPIC
  check_cxx_compiler_flag("-fPIC" WITH_FPIC_CXX)
  if (WITH_FPIC_CXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  endif()

  if (LIBPLZMA_OPT_DISABLE_RUNTIME_TYPE_INFORMATION)
    # CXX with -fno-rtti
    check_cxx_compiler_flag("-fno-rtti" WITH_FNO_RTTI_CXX)
    if (WITH_FNO_RTTI_CXX)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
      add_definitions(-DLIBPLZMA_NO_CPP_RTTI=1)
    endif()
  endif()

endif()


if (LIBPLZMA_OPT_TESTS)
  add_definitions(-DCMAKE_BUILD_WITH_TESTS=1)
endif()

if (CMAKE_BUILD_TYPE MATCHES Debug)
  add_definitions(-DDEBUG=1)
endif()

if (LIBPLZMA_OPT_BUILD_NUMBER)
  add_definitions(-DLIBPLZMA_VERSION_BUILD=${LIBPLZMA_OPT_BUILD_NUMBER})
endif()

if (LIBPLZMA_OPT_NO_C_BINDINGS)
  add_definitions(-DLIBPLZMA_NO_C_BINDINGS=1)
endif()

if (LIBPLZMA_OPT_NO_CRYPTO)
  add_definitions(-DLIBPLZMA_NO_CRYPTO=1)
endif()

if (LIBPLZMA_OPT_HAVE_STD)
  add_definitions(-DLIBPLZMA_HAVE_STD=1)
endif()

if (LIBPLZMA_OPT_NO_TAR)
  add_definitions(-DLIBPLZMA_NO_TAR=1)
endif()

if (LIBPLZMA_OPT_NO_PROGRESS)
  add_definitions(-DLIBPLZMA_NO_PROGRESS=1)
endif()

if (LIBPLZMA_OPT_THREAD_UNSAFE)
  add_definitions(-DLIBPLZMA_THREAD_UNSAFE=1)
endif()

add_definitions(-DCMAKE_BUILD=1)
add_definitions(-DLIBPLZMA_BUILD=1)

# ------ find package ------
#CMake 3.1.0+
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if (LIBPLZMA_OPT_ANDROID)
  find_library(log-lib log)
endif()

# ---- check include ----
CHECK_INCLUDE_FILE("sys/sys_types.h" HAVE_SYS_SYS_TYPES_H)
if (HAVE_SYS_SYS_TYPES_H)
  add_definitions(-DHAVE_SYS_SYS_TYPES_H=1)
endif()

CHECK_INCLUDE_FILE("sys/sysmacros.h" HAVE_SYS_SYSMACROS_H)
if (HAVE_SYS_SYSMACROS_H)
  add_definitions(-DHAVE_SYS_SYSMACROS_H=1)
endif()

CHECK_INCLUDE_FILE("sys/utime.h" HAVE_SYS_UTIME_H)
if (HAVE_SYS_UTIME_H)
  add_definitions(-DHAVE_SYS_UTIME_H=1)
endif()

CHECK_INCLUDE_FILE("utime.h" HAVE_UTIME_H)
if (HAVE_UTIME_H)
  add_definitions(-DHAVE_UTIME_H=1)
endif()

CHECK_INCLUDE_FILE("semaphore.h" HAVE_SEMAPHORE_H)
if (HAVE_SEMAPHORE_H)
  add_definitions(-DHAVE_SEMAPHORE_H=1)
endif()

CHECK_INCLUDE_FILE("unistd.h" HAVE_UNISTD_H)
if (HAVE_UNISTD_H)
  add_definitions(-DHAVE_UNISTD_H=1)
endif()

CHECK_INCLUDE_FILE("pthread.h" HAVE_PTHREAD_H)
if (HAVE_PTHREAD_H)
  add_definitions(-DHAVE_PTHREAD_H=1)
endif()

CHECK_INCLUDE_FILE("android/api-level.h" HAVE_ANDROID_API_LEVEL_H)
if (HAVE_ANDROID_API_LEVEL_H)
  add_definitions(-DHAVE_ANDROID_API_LEVEL_H=1)
endif()


# ---- check functions ----
if (MSVC)
  set(LIBPLZMA_CHECK_SYMBOL_HEADERS
    stdio.h
    stdlib.h
    wchar.h
  )
  check_symbol_exists(_wfopen_s ${LIBPLZMA_CHECK_SYMBOL_HEADERS} HAVE__WFOPEN_S)
  if (HAVE__WFOPEN_S)
    add_definitions(-DHAVE__WFOPEN_S=1)
  endif()
  
  check_symbol_exists(_wdupenv_s ${LIBPLZMA_CHECK_SYMBOL_HEADERS} HAVE__WDUPENV_S)
  if (HAVE__WDUPENV_S)
    add_definitions(-DHAVE__WDUPENV_S=1)
  endif()
  
  check_symbol_exists(_dupenv_s ${LIBPLZMA_CHECK_SYMBOL_HEADERS} HAVE__DUPENV_S)
  if (HAVE__DUPENV_S)
    add_definitions(-DHAVE__DUPENV_S=1)
  endif()
endif()


# ---- include ----
include_directories(${PROJECT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# ---- public headers(exported with the library) ----
set(LIBPLZMA_PUBLIC_HEADERS
  libplzma.h
  libplzma.hpp
)

# ---- internal headers ----
set(LIBPLZMA_INTERNAL_HEADERS
  src/C/7z.h
  src/C/7zCrc.h
  src/C/7zTypes.h
  src/C/7zVersion.h
  src/C/7zWindows.h
  src/C/Aes.h
  src/C/Alloc.h
  src/C/Bcj2.h
  src/C/Bra.h
  src/C/Compiler.h
  src/C/CpuArch.h
  src/C/Delta.h
  src/C/LzFind.h
  src/C/LzFindMt.h
  src/C/LzHash.h
  src/C/Lzma2Dec.h
  src/C/Lzma2DecMt.h
  src/C/Lzma2Enc.h
  src/C/LzmaDec.h
  src/C/LzmaEnc.h
  src/C/LzmaLib.h
  src/C/MtCoder.h
  src/C/MtDec.h
  src/C/Ppmd.h
  src/C/Ppmd7.h
  src/C/Precomp.h
  src/C/RotateDefs.h
  src/C/Sha256.h
  src/C/Sort.h
  src/C/SwapBytes.h
  src/C/Threads.h
  src/C/Xz.h
  src/C/XzCrc64.h
  src/C/XzEnc.h
  src/plzma_base_callback.hpp
  src/plzma_c_bindings_private.hpp
  src/plzma_common.hpp
  src/plzma_convert_utf.hpp
  src/plzma_decoder_impl.hpp
  src/plzma_encoder_impl.hpp
  src/plzma_extract_callback.hpp
  src/plzma_file_utils.hpp
  src/plzma_in_streams.hpp
  src/plzma_mutex.hpp
  src/plzma_open_callback.hpp
  src/plzma_out_streams.hpp
  src/plzma_path_utils.hpp
  src/plzma_private.h
  src/plzma_private.hpp
  src/plzma_progress.hpp
  src/plzma_update_callback.hpp
  src/CPP/7zip/Archive/7z/7zCompressionMode.h
  src/CPP/7zip/Archive/7z/7zDecode.h
  src/CPP/7zip/Archive/7z/7zEncode.h
  src/CPP/7zip/Archive/7z/7zFolderInStream.h
  src/CPP/7zip/Archive/7z/7zHandler.h
  src/CPP/7zip/Archive/7z/7zHeader.h
  src/CPP/7zip/Archive/7z/7zIn.h
  src/CPP/7zip/Archive/7z/7zItem.h
  src/CPP/7zip/Archive/7z/7zOut.h
  src/CPP/7zip/Archive/7z/7zProperties.h
  src/CPP/7zip/Archive/7z/7zSpecStream.h
  src/CPP/7zip/Archive/7z/7zUpdate.h
  src/CPP/7zip/Archive/7z/StdAfx.h
  src/CPP/7zip/Archive/Common/CoderMixer2.h
  src/CPP/7zip/Archive/Common/DummyOutStream.h
  src/CPP/7zip/Archive/Common/HandlerOut.h
  src/CPP/7zip/Archive/Common/InStreamWithCRC.h
  src/CPP/7zip/Archive/Common/ItemNameUtils.h
  src/CPP/7zip/Archive/Common/MultiStream.h
  src/CPP/7zip/Archive/Common/OutStreamWithCRC.h
  src/CPP/7zip/Archive/Common/StdAfx.h
  src/CPP/7zip/Archive/DllExports2.h
  src/CPP/7zip/Archive/IArchive.h
  src/CPP/7zip/Archive/StdAfx.h
  src/CPP/7zip/Archive/XzHandler.h
  src/CPP/7zip/Common/CreateCoder.h
  src/CPP/7zip/Common/CWrappers.h
  src/CPP/7zip/Common/FilePathAutoRename.h
  src/CPP/7zip/Common/FileStreams.h
  src/CPP/7zip/Common/FilterCoder.h
  src/CPP/7zip/Common/InBuffer.h
  src/CPP/7zip/Common/InOutTempBuffer.h
  src/CPP/7zip/Common/LimitedStreams.h
  src/CPP/7zip/Common/MethodId.h
  src/CPP/7zip/Common/MethodProps.h
  src/CPP/7zip/Common/OffsetStream.h
  src/CPP/7zip/Common/OutBuffer.h
  src/CPP/7zip/Common/ProgressUtils.h
  src/CPP/7zip/Common/RegisterArc.h
  src/CPP/7zip/Common/RegisterCodec.h
  src/CPP/7zip/Common/StdAfx.h
  src/CPP/7zip/Common/StreamBinder.h
  src/CPP/7zip/Common/StreamObjects.h
  src/CPP/7zip/Common/StreamUtils.h
  src/CPP/7zip/Common/UniqBlocks.h
  src/CPP/7zip/Common/VirtThread.h
  src/CPP/7zip/Compress/Bcj2Coder.h
  src/CPP/7zip/Compress/BcjCoder.h
  src/CPP/7zip/Compress/BranchMisc.h
  src/CPP/7zip/Compress/CopyCoder.h
  src/CPP/7zip/Compress/Lzma2Decoder.h
  src/CPP/7zip/Compress/Lzma2Encoder.h
  src/CPP/7zip/Compress/LzmaDecoder.h
  src/CPP/7zip/Compress/LzmaEncoder.h
  src/CPP/7zip/Compress/PpmdDecoder.h
  src/CPP/7zip/Compress/PpmdEncoder.h
  src/CPP/7zip/Compress/StdAfx.h
  src/CPP/7zip/Compress/XzDecoder.h
  src/CPP/7zip/Compress/XzEncoder.h
  src/CPP/7zip/Crypto/7zAes.h
  src/CPP/7zip/Crypto/MyAes.h
  src/CPP/7zip/Crypto/RandGen.h
  src/CPP/7zip/Crypto/StdAfx.h
  src/CPP/7zip/ICoder.h
  src/CPP/7zip/IDecl.h
  src/CPP/7zip/IPassword.h
  src/CPP/7zip/IProgress.h
  src/CPP/7zip/IStream.h
  src/CPP/7zip/MyVersion.h
  src/CPP/7zip/PropID.h
  src/CPP/Common/AutoPtr.h
  src/CPP/Common/Common.h
  src/CPP/Common/Common0.h
  src/CPP/Common/ComTry.h
  src/CPP/Common/Defs.h
  src/CPP/Common/DynamicBuffer.h
  src/CPP/Common/IntToString.h
  src/CPP/Common/ListFileUtils.h
  src/CPP/Common/MyBuffer.h
  src/CPP/Common/MyBuffer2.h
  src/CPP/Common/MyCom.h
  src/CPP/Common/MyException.h
  src/CPP/Common/MyGuidDef.h
  src/CPP/Common/MyInitGuid.h
  src/CPP/Common/MyLinux.h
  src/CPP/Common/MyString.h
  src/CPP/Common/MyTypes.h
  src/CPP/Common/MyUnknown.h
  src/CPP/Common/MyVector.h
  src/CPP/Common/MyWindows.h
  src/CPP/Common/NewHandler.h
  src/CPP/Common/StdAfx.h
  src/CPP/Common/StdInStream.h
  src/CPP/Common/StdOutStream.h
  src/CPP/Common/StringConvert.h
  src/CPP/Common/StringToInt.h
  src/CPP/Common/UTFConvert.h
  src/CPP/Common/Wildcard.h
  src/CPP/Windows/Defs.h
  src/CPP/Windows/FileDir.h
  src/CPP/Windows/FileFind.h
  src/CPP/Windows/FileIO.h
  src/CPP/Windows/FileName.h
  src/CPP/Windows/Handle.h
  src/CPP/Windows/NtCheck.h
  src/CPP/Windows/PropVariant.h
  src/CPP/Windows/PropVariantConv.h
  src/CPP/Windows/StdAfx.h
  src/CPP/Windows/Synchronization.h
  src/CPP/Windows/System.h
  src/CPP/Windows/Thread.h
  src/CPP/Windows/TimeUtils.h
)

# ---- all sources(public + internal) ----
set(LIBPLZMA_SOURCES
  src/C/7zCrc.c
  src/C/7zCrcOpt.c
  src/C/7zStream.c
  src/C/Aes.c
  src/C/AesOpt.c
  src/C/Alloc.c
  src/C/Bcj2.c
  src/C/Bcj2Enc.c
  src/C/Bra.c
  src/C/Bra86.c
  src/C/BraIA64.c
  src/C/CpuArch.c
  src/C/Delta.c
  src/C/LzFind.c
  src/C/LzFindMt.c
  src/C/LzFindOpt.c
  src/C/Lzma2Dec.c
  src/C/Lzma2DecMt.c
  src/C/Lzma2Enc.c
  src/C/LzmaDec.c
  src/C/LzmaEnc.c
  src/C/MtCoder.c
  src/C/Ppmd7.c
  src/C/Ppmd7Dec.c
  src/C/Ppmd7Enc.c
  src/C/Sha256.c
  src/C/Sha256Opt.c
  src/C/Sort.c
  src/C/SwapBytes.c
  src/C/Threads.c
  src/C/Xz.c
  src/C/XzCrc64.c
  src/C/XzCrc64Opt.c
  src/C/XzDec.c
  src/C/XzEnc.c
  src/C/XzIn.c
  src/plzma.cpp
  src/plzma_base_callback.cpp
  src/plzma_common.cpp
  src/plzma_decoder_impl.cpp
  src/plzma_encoder_impl.cpp
  src/plzma_exception.cpp
  src/plzma_extract_callback.cpp
  src/plzma_file_utils.cpp
  src/plzma_in_streams.cpp
  src/plzma_item.cpp
  src/plzma_open_callback.cpp
  src/plzma_out_streams.cpp
  src/plzma_path.cpp
  src/plzma_path_utils.cpp
  src/plzma_progress.cpp
  src/plzma_raw_heap_memory.cpp
  src/plzma_string.cpp
  src/plzma_update_callback.cpp
  src/CPP/7zip/Archive/7z/7zDecode.cpp
  src/CPP/7zip/Archive/7z/7zEncode.cpp
  src/CPP/7zip/Archive/7z/7zExtract.cpp
  src/CPP/7zip/Archive/7z/7zFolderInStream.cpp
  src/CPP/7zip/Archive/7z/7zHandler.cpp
  src/CPP/7zip/Archive/7z/7zHandlerOut.cpp
  src/CPP/7zip/Archive/7z/7zHeader.cpp
  src/CPP/7zip/Archive/7z/7zIn.cpp
  src/CPP/7zip/Archive/7z/7zOut.cpp
  src/CPP/7zip/Archive/7z/7zProperties.cpp
  src/CPP/7zip/Archive/7z/7zRegister.cpp
  src/CPP/7zip/Archive/7z/7zUpdate.cpp
  src/CPP/7zip/Archive/ArchiveExports.cpp
  src/CPP/7zip/Archive/Common/CoderMixer2.cpp
  src/CPP/7zip/Archive/Common/DummyOutStream.cpp
  src/CPP/7zip/Archive/Common/HandlerOut.cpp
  src/CPP/7zip/Archive/Common/InStreamWithCRC.cpp
  src/CPP/7zip/Archive/Common/ItemNameUtils.cpp
  src/CPP/7zip/Archive/Common/MultiStream.cpp
  src/CPP/7zip/Archive/Common/OutStreamWithCRC.cpp
  src/CPP/7zip/Archive/DllExports2.cpp
  src/CPP/7zip/Archive/LzmaHandler.cpp
  src/CPP/7zip/Archive/SplitHandler.cpp
  src/CPP/7zip/Archive/XzHandler.cpp
  src/CPP/7zip/Common/CreateCoder.cpp
  src/CPP/7zip/Common/CWrappers.cpp
  src/CPP/7zip/Common/FileStreams.cpp
  src/CPP/7zip/Common/FilterCoder.cpp
  src/CPP/7zip/Common/InBuffer.cpp
  src/CPP/7zip/Common/InOutTempBuffer.cpp
  src/CPP/7zip/Common/LimitedStreams.cpp
  src/CPP/7zip/Common/MethodProps.cpp
  src/CPP/7zip/Common/OffsetStream.cpp
  src/CPP/7zip/Common/OutBuffer.cpp
  src/CPP/7zip/Common/ProgressUtils.cpp
  src/CPP/7zip/Common/PropId.cpp
  src/CPP/7zip/Common/StreamObjects.cpp
  src/CPP/7zip/Common/StreamUtils.cpp
  src/CPP/7zip/Common/UniqBlocks.cpp
  src/CPP/7zip/Common/VirtThread.cpp
  src/CPP/7zip/Compress/Bcj2Coder.cpp
  src/CPP/7zip/Compress/Bcj2Register.cpp
  src/CPP/7zip/Compress/BcjCoder.cpp
  src/CPP/7zip/Compress/BcjRegister.cpp
  src/CPP/7zip/Compress/BranchMisc.cpp
  src/CPP/7zip/Compress/BranchRegister.cpp
  src/CPP/7zip/Compress/ByteSwap.cpp
  src/CPP/7zip/Compress/CodecExports.cpp
  src/CPP/7zip/Compress/CopyCoder.cpp
  src/CPP/7zip/Compress/CopyRegister.cpp
  src/CPP/7zip/Compress/DeltaFilter.cpp
  src/CPP/7zip/Compress/Lzma2Decoder.cpp
  src/CPP/7zip/Compress/Lzma2Encoder.cpp
  src/CPP/7zip/Compress/Lzma2Register.cpp
  src/CPP/7zip/Compress/LzmaDecoder.cpp
  src/CPP/7zip/Compress/LzmaEncoder.cpp
  src/CPP/7zip/Compress/LzmaRegister.cpp
  src/CPP/7zip/Compress/PpmdDecoder.cpp
  src/CPP/7zip/Compress/PpmdEncoder.cpp
  src/CPP/7zip/Compress/PpmdRegister.cpp
  src/CPP/7zip/Compress/XzDecoder.cpp
  src/CPP/7zip/Compress/XzEncoder.cpp
  src/CPP/7zip/Crypto/7zAes.cpp
  src/CPP/7zip/Crypto/7zAesRegister.cpp
  src/CPP/7zip/Crypto/MyAes.cpp
  src/CPP/7zip/Crypto/MyAesReg.cpp
  src/CPP/7zip/Crypto/RandGen.cpp
  src/CPP/Common/CRC.cpp
  src/CPP/Common/CrcReg.cpp
  src/CPP/Common/IntToString.cpp
  src/CPP/Common/MyString.cpp
  src/CPP/Common/MyWindows.cpp
  src/CPP/Common/NewHandler.cpp
  src/CPP/Common/Sha256Reg.cpp
  src/CPP/Common/StringConvert.cpp
  src/CPP/Common/StringToInt.cpp
  src/CPP/Common/UTFConvert.cpp
  src/CPP/Common/Wildcard.cpp
  src/CPP/Common/XzCrc64Reg.cpp
  src/CPP/Windows/FileDir.cpp
  src/CPP/Windows/FileFind.cpp
  src/CPP/Windows/FileIO.cpp
  src/CPP/Windows/FileName.cpp
  src/CPP/Windows/PropVariant.cpp
  src/CPP/Windows/PropVariantConv.cpp
  src/CPP/Windows/System.cpp
  src/CPP/Windows/TimeUtils.cpp
)

if(NOT LIBPLZMA_OPT_NO_TAR)
  list(APPEND LIBPLZMA_INTERNAL_HEADERS
    src/CPP/7zip/Archive/Tar/StdAfx.h
    src/CPP/7zip/Archive/Tar/TarHandler.h
    src/CPP/7zip/Archive/Tar/TarHeader.h
    src/CPP/7zip/Archive/Tar/TarIn.h
    src/CPP/7zip/Archive/Tar/TarItem.h
    src/CPP/7zip/Archive/Tar/TarOut.h
    src/CPP/7zip/Archive/Tar/TarUpdate.h
  )

  list(APPEND LIBPLZMA_SOURCES
    src/CPP/7zip/Archive/Tar/TarHandler.cpp
    src/CPP/7zip/Archive/Tar/TarHandlerOut.cpp
    src/CPP/7zip/Archive/Tar/TarHeader.cpp
    src/CPP/7zip/Archive/Tar/TarIn.cpp
    src/CPP/7zip/Archive/Tar/TarOut.cpp
    src/CPP/7zip/Archive/Tar/TarRegister.cpp
    src/CPP/7zip/Archive/Tar/TarUpdate.cpp
  )
endif()

# ---- groups(headers and sources) ----
# ---- grop: public headers and sources ----
source_group("public"
  FILES
  libplzma.h
  libplzma.hpp
  src/plzma.cpp
)

# ---- grop: internal headers and sources ----
source_group("src"
  FILES
  src/plzma_base_callback.cpp
  src/plzma_base_callback.hpp
  src/plzma_c_bindings_private.hpp
  src/plzma_common.cpp
  src/plzma_common.hpp
  src/plzma_convert_utf.hpp
  src/plzma_decoder_impl.cpp
  src/plzma_decoder_impl.hpp
  src/plzma_encoder_impl.cpp
  src/plzma_encoder_impl.hpp
  src/plzma_exception.cpp
  src/plzma_extract_callback.cpp
  src/plzma_extract_callback.hpp
  src/plzma_file_utils.cpp
  src/plzma_file_utils.hpp
  src/plzma_in_streams.cpp
  src/plzma_in_streams.hpp
  src/plzma_item.cpp
  src/plzma_mutex.hpp
  src/plzma_open_callback.cpp
  src/plzma_open_callback.hpp
  src/plzma_out_streams.cpp
  src/plzma_out_streams.hpp
  src/plzma_path.cpp
  src/plzma_path_utils.cpp
  src/plzma_path_utils.hpp
  src/plzma_private.h
  src/plzma_private.hpp
  src/plzma_progress.cpp
  src/plzma_progress.hpp
  src/plzma_raw_heap_memory.cpp
  src/plzma_string.cpp
  src/plzma_update_callback.cpp
  src/plzma_update_callback.hpp
)

# ---- grop: Lzma C headers and sources ----
source_group("C"
  FILES
  src/C/7z.h
  src/C/7zAlloc.h
  src/C/7zBuf.h
  src/C/7zCrc.c
  src/C/7zCrc.h
  src/C/7zCrcOpt.c
  src/C/7zStream.c
  src/C/7zTypes.h
  src/C/7zVersion.h
  src/C/7zWindows.h
  src/C/Aes.c
  src/C/Aes.h
  src/C/AesOpt.c
  src/C/Alloc.c
  src/C/Alloc.h
  src/C/Bcj2.c
  src/C/Bcj2.h
  src/C/Bcj2Enc.c
  src/C/Bra.c
  src/C/Bra.h
  src/C/Bra86.c
  src/C/BraIA64.c
  src/C/Compiler.h
  src/C/CpuArch.c
  src/C/CpuArch.h
  src/C/Delta.c
  src/C/Delta.h
  src/C/LzFind.c
  src/C/LzFind.h
  src/C/LzFindMt.c
  src/C/LzFindMt.h
  src/C/LzFindOpt.c
  src/C/LzHash.h
  src/C/Lzma2Dec.c
  src/C/Lzma2Dec.h
  src/C/Lzma2DecMt.c
  src/C/Lzma2DecMt.h
  src/C/Lzma2Enc.c
  src/C/Lzma2Enc.h
  src/C/LzmaDec.c
  src/C/LzmaDec.h
  src/C/LzmaEnc.c
  src/C/LzmaEnc.h
  src/C/LzmaLib.h
  src/C/MtCoder.c
  src/C/MtCoder.h
  src/C/MtDec.h
  src/C/Ppmd.h
  src/C/Ppmd7.c
  src/C/Ppmd7.h
  src/C/Ppmd7Dec.c
  src/C/Ppmd7Enc.c
  src/C/Precomp.h
  src/C/RotateDefs.h
  src/C/Sha256.c
  src/C/Sha256.h
  src/C/Sha256Opt.c
  src/C/Sort.c
  src/C/Sort.h
  src/C/SwapBytes.c
  src/C/SwapBytes.h
  src/C/Threads.c
  src/C/Threads.h
  src/C/Xz.c
  src/C/Xz.h
  src/C/XzCrc64.c
  src/C/XzCrc64.h
  src/C/XzCrc64Opt.c
  src/C/XzDec.c
  src/C/XzEnc.c
  src/C/XzEnc.h
  src/C/XzIn.c
)

# ---- grop: Lzma CPP/Windows headers and sources ----
source_group("CPP/Windows"
  FILES
  src/CPP/Windows/Defs.h
  src/CPP/Windows/FileDir.cpp
  src/CPP/Windows/FileDir.h
  src/CPP/Windows/FileFind.cpp
  src/CPP/Windows/FileFind.h
  src/CPP/Windows/FileIO.cpp
  src/CPP/Windows/FileIO.h
  src/CPP/Windows/FileName.cpp
  src/CPP/Windows/FileName.h
  src/CPP/Windows/Handle.h
  src/CPP/Windows/NtCheck.h
  src/CPP/Windows/PropVariant.cpp
  src/CPP/Windows/PropVariant.h
  src/CPP/Windows/PropVariantConv.cpp
  src/CPP/Windows/PropVariantConv.h
  src/CPP/Windows/StdAfx.h
  src/CPP/Windows/Synchronization.h
  src/CPP/Windows/System.cpp
  src/CPP/Windows/System.h
  src/CPP/Windows/Thread.h
  src/CPP/Windows/TimeUtils.cpp
  src/CPP/Windows/TimeUtils.h
)

# ---- grop: Lzma CPP/Common headers and sources ----
source_group("CPP/Common"
  FILES
  src/CPP/Common/AutoPtr.h
  src/CPP/Common/Common.h
  src/CPP/Common/Common0.h
  src/CPP/Common/ComTry.h
  src/CPP/Common/CRC.cpp
  src/CPP/Common/CrcReg.cpp
  src/CPP/Common/Defs.h
  src/CPP/Common/DynamicBuffer.h
  src/CPP/Common/IntToString.cpp
  src/CPP/Common/IntToString.h
  src/CPP/Common/ListFileUtils.h
  src/CPP/Common/MyBuffer.h
  src/CPP/Common/MyBuffer2.h
  src/CPP/Common/MyCom.h
  src/CPP/Common/MyException.h
  src/CPP/Common/MyGuidDef.h
  src/CPP/Common/MyInitGuid.h
  src/CPP/Common/MyLinux.h
  src/CPP/Common/MyString.cpp
  src/CPP/Common/MyString.h
  src/CPP/Common/MyTypes.h
  src/CPP/Common/MyUnknown.h
  src/CPP/Common/MyVector.h
  src/CPP/Common/MyWindows.cpp
  src/CPP/Common/MyWindows.h
  src/CPP/Common/NewHandler.cpp
  src/CPP/Common/NewHandler.h
  src/CPP/Common/Sha256Reg.cpp
  src/CPP/Common/StdAfx.h
  src/CPP/Common/StdInStream.h
  src/CPP/Common/StdOutStream.h
  src/CPP/Common/StringConvert.cpp
  src/CPP/Common/StringConvert.h
  src/CPP/Common/StringToInt.cpp
  src/CPP/Common/StringToInt.h
  src/CPP/Common/UTFConvert.cpp
  src/CPP/Common/UTFConvert.h
  src/CPP/Common/Wildcard.cpp
  src/CPP/Common/Wildcard.h
  src/CPP/Common/XzCrc64Reg.cpp
)

# ---- grop: Lzma CPP/7zip headers and sources ----
source_group("CPP/7zip"
  FILES
  src/CPP/7zip/ICoder.h
  src/CPP/7zip/IDecl.h
  src/CPP/7zip/IPassword.h
  src/CPP/7zip/IProgress.h
  src/CPP/7zip/IStream.h
  src/CPP/7zip/MyVersion.h
  src/CPP/7zip/PropID.h
)

# ---- grop: Lzma CPP/7zip/Archive/7z headers and sources ----
source_group("CPP/7zip/Archive/7z"
  FILES
  src/CPP/7zip/Archive/7z/7zCompressionMode.h
  src/CPP/7zip/Archive/7z/7zDecode.cpp
  src/CPP/7zip/Archive/7z/7zDecode.h
  src/CPP/7zip/Archive/7z/7zEncode.cpp
  src/CPP/7zip/Archive/7z/7zEncode.h
  src/CPP/7zip/Archive/7z/7zExtract.cpp
  src/CPP/7zip/Archive/7z/7zFolderInStream.cpp
  src/CPP/7zip/Archive/7z/7zFolderInStream.h
  src/CPP/7zip/Archive/7z/7zHandler.cpp
  src/CPP/7zip/Archive/7z/7zHandler.h
  src/CPP/7zip/Archive/7z/7zHandlerOut.cpp
  src/CPP/7zip/Archive/7z/7zHeader.cpp
  src/CPP/7zip/Archive/7z/7zHeader.h
  src/CPP/7zip/Archive/7z/7zIn.cpp
  src/CPP/7zip/Archive/7z/7zIn.h
  src/CPP/7zip/Archive/7z/7zItem.h
  src/CPP/7zip/Archive/7z/7zOut.cpp
  src/CPP/7zip/Archive/7z/7zOut.h
  src/CPP/7zip/Archive/7z/7zProperties.cpp
  src/CPP/7zip/Archive/7z/7zProperties.h
  src/CPP/7zip/Archive/7z/7zRegister.cpp
  src/CPP/7zip/Archive/7z/7zSpecStream.h
  src/CPP/7zip/Archive/7z/7zUpdate.cpp
  src/CPP/7zip/Archive/7z/7zUpdate.h
  src/CPP/7zip/Archive/7z/StdAfx.h
)

# ---- grop: Lzma CPP/7zip/Archive/Common headers and sources ----
source_group("CPP/7zip/Archive/Common"
  FILES
  src/CPP/7zip/Archive/Common/CoderMixer2.cpp
  src/CPP/7zip/Archive/Common/CoderMixer2.h
  src/CPP/7zip/Archive/Common/DummyOutStream.cpp
  src/CPP/7zip/Archive/Common/DummyOutStream.h
  src/CPP/7zip/Archive/Common/HandlerOut.cpp
  src/CPP/7zip/Archive/Common/HandlerOut.h
  src/CPP/7zip/Archive/Common/InStreamWithCRC.cpp
  src/CPP/7zip/Archive/Common/InStreamWithCRC.h
  src/CPP/7zip/Archive/Common/ItemNameUtils.cpp
  src/CPP/7zip/Archive/Common/ItemNameUtils.h
  src/CPP/7zip/Archive/Common/MultiStream.cpp
  src/CPP/7zip/Archive/Common/MultiStream.h
  src/CPP/7zip/Archive/Common/OutStreamWithCRC.cpp
  src/CPP/7zip/Archive/Common/OutStreamWithCRC.h
  src/CPP/7zip/Archive/Common/StdAfx.h
)

# ---- grop: Lzma CPP/7zip/Archive/Tar headers and sources ----
source_group("CPP/7zip/Archive/Tar"
  FILES
  src/CPP/7zip/Archive/Tar/StdAfx.h
  src/CPP/7zip/Archive/Tar/TarHandler.cpp
  src/CPP/7zip/Archive/Tar/TarHandler.h
  src/CPP/7zip/Archive/Tar/TarHandlerOut.cpp
  src/CPP/7zip/Archive/Tar/TarHeader.cpp
  src/CPP/7zip/Archive/Tar/TarHeader.h
  src/CPP/7zip/Archive/Tar/TarIn.cpp
  src/CPP/7zip/Archive/Tar/TarIn.h
  src/CPP/7zip/Archive/Tar/TarItem.h
  src/CPP/7zip/Archive/Tar/TarOut.cpp
  src/CPP/7zip/Archive/Tar/TarOut.h
  src/CPP/7zip/Archive/Tar/TarRegister.cpp
  src/CPP/7zip/Archive/Tar/TarUpdate.cpp
  src/CPP/7zip/Archive/Tar/TarUpdate.h
)

# ---- grop: Lzma CPP/7zip/Archive headers and sources ----
source_group("CPP/7zip/Archive"
  FILES
  src/CPP/7zip/Archive/ArchiveExports.cpp
  src/CPP/7zip/Archive/DllExports2.cpp
  src/CPP/7zip/Archive/DllExports2.h
  src/CPP/7zip/Archive/IArchive.h
  src/CPP/7zip/Archive/LzmaHandler.cpp
  src/CPP/7zip/Archive/SplitHandler.cpp
  src/CPP/7zip/Archive/StdAfx.h
  src/CPP/7zip/Archive/XzHandler.cpp
  src/CPP/7zip/Archive/XzHandler.h
)

# ---- grop: Lzma CPP/7zip/Common headers and sources ----
source_group("CPP/7zip/Common"
  FILES
  src/CPP/7zip/Common/CreateCoder.cpp
  src/CPP/7zip/Common/CreateCoder.h
  src/CPP/7zip/Common/CWrappers.cpp
  src/CPP/7zip/Common/CWrappers.h
  src/CPP/7zip/Common/FilePathAutoRename.h
  src/CPP/7zip/Common/FileStreams.cpp
  src/CPP/7zip/Common/FileStreams.h
  src/CPP/7zip/Common/FilterCoder.cpp
  src/CPP/7zip/Common/FilterCoder.h
  src/CPP/7zip/Common/InBuffer.cpp
  src/CPP/7zip/Common/InBuffer.h
  src/CPP/7zip/Common/InOutTempBuffer.cpp
  src/CPP/7zip/Common/InOutTempBuffer.h
  src/CPP/7zip/Common/LimitedStreams.cpp
  src/CPP/7zip/Common/LimitedStreams.h
  src/CPP/7zip/Common/MethodId.h
  src/CPP/7zip/Common/MethodProps.cpp
  src/CPP/7zip/Common/MethodProps.h
  src/CPP/7zip/Common/OffsetStream.cpp
  src/CPP/7zip/Common/OffsetStream.h
  src/CPP/7zip/Common/OutBuffer.cpp
  src/CPP/7zip/Common/OutBuffer.h
  src/CPP/7zip/Common/ProgressUtils.cpp
  src/CPP/7zip/Common/ProgressUtils.h
  src/CPP/7zip/Common/PropId.cpp
  src/CPP/7zip/Common/RegisterArc.h
  src/CPP/7zip/Common/RegisterCodec.h
  src/CPP/7zip/Common/StdAfx.h
  src/CPP/7zip/Common/StreamBinder.h
  src/CPP/7zip/Common/StreamObjects.cpp
  src/CPP/7zip/Common/StreamObjects.h
  src/CPP/7zip/Common/StreamUtils.cpp
  src/CPP/7zip/Common/StreamUtils.h
  src/CPP/7zip/Common/UniqBlocks.cpp
  src/CPP/7zip/Common/UniqBlocks.h
  src/CPP/7zip/Common/VirtThread.cpp
  src/CPP/7zip/Common/VirtThread.h
)

# ---- grop: Lzma CPP/7zip/Compress headers and sources ----
source_group("CPP/7zip/Compress"
  FILES
  src/CPP/7zip/Compress/Bcj2Coder.cpp
  src/CPP/7zip/Compress/Bcj2Coder.h
  src/CPP/7zip/Compress/Bcj2Register.cpp
  src/CPP/7zip/Compress/BcjCoder.cpp
  src/CPP/7zip/Compress/BcjCoder.h
  src/CPP/7zip/Compress/BcjRegister.cpp
  src/CPP/7zip/Compress/BranchMisc.cpp
  src/CPP/7zip/Compress/BranchMisc.h
  src/CPP/7zip/Compress/BranchRegister.cpp
  src/CPP/7zip/Compress/ByteSwap.cpp
  src/CPP/7zip/Compress/CodecExports.cpp
  src/CPP/7zip/Compress/CopyCoder.cpp
  src/CPP/7zip/Compress/CopyCoder.h
  src/CPP/7zip/Compress/CopyRegister.cpp
  src/CPP/7zip/Compress/DeltaFilter.cpp
  src/CPP/7zip/Compress/Lzma2Decoder.cpp
  src/CPP/7zip/Compress/Lzma2Decoder.h
  src/CPP/7zip/Compress/Lzma2Encoder.cpp
  src/CPP/7zip/Compress/Lzma2Encoder.h
  src/CPP/7zip/Compress/Lzma2Register.cpp
  src/CPP/7zip/Compress/LzmaDecoder.cpp
  src/CPP/7zip/Compress/LzmaDecoder.h
  src/CPP/7zip/Compress/LzmaEncoder.cpp
  src/CPP/7zip/Compress/LzmaEncoder.h
  src/CPP/7zip/Compress/LzmaRegister.cpp
  src/CPP/7zip/Compress/PpmdDecoder.cpp
  src/CPP/7zip/Compress/PpmdDecoder.h
  src/CPP/7zip/Compress/PpmdEncoder.cpp
  src/CPP/7zip/Compress/PpmdEncoder.h
  src/CPP/7zip/Compress/PpmdRegister.cpp
  src/CPP/7zip/Compress/StdAfx.h
  src/CPP/7zip/Compress/XzDecoder.cpp
  src/CPP/7zip/Compress/XzDecoder.h
  src/CPP/7zip/Compress/XzEncoder.cpp
  src/CPP/7zip/Compress/XzEncoder.h
)

# ---- grop: Lzma CPP/7zip/Crypto headers and sources ----
source_group("CPP/7zip/Crypto"
  FILES
  src/CPP/7zip/Crypto/7zAes.cpp
  src/CPP/7zip/Crypto/7zAes.h
  src/CPP/7zip/Crypto/7zAesRegister.cpp
  src/CPP/7zip/Crypto/MyAes.cpp
  src/CPP/7zip/Crypto/MyAes.h
  src/CPP/7zip/Crypto/MyAesReg.cpp
  src/CPP/7zip/Crypto/RandGen.cpp
  src/CPP/7zip/Crypto/RandGen.h
  src/CPP/7zip/Crypto/StdAfx.h
)

if (LIBPLZMA_OPT_SHARED)
  add_library(plzma SHARED ${LIBPLZMA_SOURCES} ${LIBPLZMA_PUBLIC_HEADERS} ${LIBPLZMA_INTERNAL_HEADERS})
  set_property(TARGET plzma APPEND PROPERTY COMPILE_FLAGS -DLIBPLZMA_SHARED=1)
  if (MSVC)
    message("Shared lib. MSVC")
    # MSVC does not append 'lib' - do it here to have consistent name
    set_property(TARGET plzma PROPERTY PREFIX "lib")
    set_property(TARGET plzma PROPERTY IMPORT_PREFIX "lib")
  endif()
  target_link_libraries(plzma Threads::Threads)
  if (LIBPLZMA_OPT_ANDROID)
    target_link_libraries(plzma android)
    target_link_libraries(plzma ${log-lib})
  endif()
  install(TARGETS plzma DESTINATION lib)
endif()

if (LIBPLZMA_OPT_STATIC)
  add_library(plzma_static STATIC ${LIBPLZMA_SOURCES} ${LIBPLZMA_PUBLIC_HEADERS} ${LIBPLZMA_INTERNAL_HEADERS})
  set_property(TARGET plzma_static APPEND PROPERTY COMPILE_FLAGS -DLIBPLZMA_STATIC=1)
  if (MSVC)
    message("Static lib. MSVC")
    # MSVC does not append 'lib' - do it here to have consistent name
    set_target_properties(plzma_static PROPERTIES PREFIX "lib")
  endif()
  install(TARGETS plzma_static DESTINATION lib)
endif()

install(FILES ${LIBPLZMA_PUBLIC_HEADERS} DESTINATION include)

if (LIBPLZMA_OPT_TESTS)
  enable_testing()
  add_subdirectory(cmake_tests)

  # This must always be last!
  include(CPack)
endif()
