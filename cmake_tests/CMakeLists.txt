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


include(CheckLibraryExists)

include_directories(${PROJECT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

link_directories(${PROJECT_BINARY_DIR})

remove_definitions(-DCMAKE_BUILD=1)
remove_definitions(-DLIBPLZMA_BUILD=1)

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
  if (CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MTd")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MTd")
  else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MT")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MT")
  endif()
  
  # Always use UTF8 for sources and execution
  # https://docs.microsoft.com/en-us/cpp/build/reference/utf-8-set-source-and-executable-character-sets-to-utf-8
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
  
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
  
  # CXX with -fno-rtti
  check_cxx_compiler_flag("-fno-rtti" WITH_FNO_RTTI_CXX)
  if (WITH_FNO_RTTI_CXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
    add_definitions(-DLIBPLZMA_NO_CPP_RTTI=1)
  endif()
  
endif()

set(LIBPLZMA_TESTS
  "test_plzma_compress"
  "test_plzma_containers"
  "test_plzma_extract"
  "test_plzma_multivolume"
  "test_plzma_open"
  "test_plzma_path"
  "test_plzma_streams"
  "test_plzma_string"
)

foreach(LIBPLZMA_TEST ${LIBPLZMA_TESTS})
  # No dynamic tests for now
  add_executable(${LIBPLZMA_TEST} ${LIBPLZMA_TEST}.cpp plzma_public_tests.hpp)
  target_link_libraries(${LIBPLZMA_TEST} plzma)
  set_property(TARGET ${LIBPLZMA_TEST} APPEND PROPERTY COMPILE_FLAGS -DLIBPLZMA_SHARED=1)
  add_test(${LIBPLZMA_TEST} ${LIBPLZMA_TEST})
  
  # Copy DLL(s) to test executable.
  add_custom_command(TARGET ${LIBPLZMA_TEST} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:${LIBPLZMA_TEST}> $<TARGET_RUNTIME_DLLS:${LIBPLZMA_TEST}>
    COMMAND_EXPAND_LISTS
  )

  add_executable("${LIBPLZMA_TEST}_static" ${LIBPLZMA_TEST}.cpp plzma_public_tests.hpp)
  target_link_libraries("${LIBPLZMA_TEST}_static" plzma_static)
  target_link_libraries("${LIBPLZMA_TEST}_static" Threads::Threads)
  set_property(TARGET "${LIBPLZMA_TEST}_static" APPEND PROPERTY COMPILE_FLAGS -DLIBPLZMA_STATIC=1)
  add_test("${LIBPLZMA_TEST}_static" "${LIBPLZMA_TEST}_static")

  if (WIN32)
    target_link_libraries(${LIBPLZMA_TEST} ws2_32)
    target_link_libraries("${LIBPLZMA_TEST}_static" ws2_32)
  endif()
  
  # Do not install test binaries, no one needs them
  # install(TARGETS ${LIBPLZMA_TEST} DESTINATION bin)
  # install(TARGETS "${LIBPLZMA_TEST}_static" DESTINATION bin)
endforeach()
