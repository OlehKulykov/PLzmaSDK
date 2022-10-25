//
// By using this Software, you are accepting original [LZMA SDK] and MIT license below:
//
// The MIT License (MIT)
//
// Copyright (c) 2015 - 2022 Oleh Kulykov <olehkulykov@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//


#ifndef __PLZMA_C_BINDINGS_PRIVATE_HPP__
#define __PLZMA_C_BINDINGS_PRIVATE_HPP__ 1

#if !defined(LIBPLZMA_NO_C_BINDINGS)

#include <cstddef>

#include "../libplzma.h"


// plzma_private.h
#if !defined(LIBPLZMA_HAVE_STD) && (defined(SWIFT_PACKAGE) || defined(COCOAPODS))
#define LIBPLZMA_HAVE_STD 1
#endif

#if defined(LIBPLZMA_HAVE_STD)
#include <exception>
#endif

// Creating

#define LIBPLZMA_C_BINDINGS_CREATE_OBJECT_TRY(OBJ_TYPE) \
OBJ_TYPE createdCObject; \
createdCObject.object = createdCObject.exception = nullptr; \
try { \



#define LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(OBJ_TYPE, FROM_PTR) \
OBJ_TYPE createdCObject; \
createdCObject.object = createdCObject.exception = nullptr; \
if (FROM_PTR->exception) return createdCObject; \
try { \



#if defined(LIBPLZMA_HAVE_STD)
#define LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH \
} catch (const Exception & exception) { \
    createdCObject.exception = static_cast<void *>(exception.moveToHeapCopy()); \
} catch (const std::exception & exception) { \
    createdCObject.exception = static_cast<void *>(Exception::create(plzma_error_code_internal, exception.what(), __FILE__, __LINE__)); \
} catch (...) { \
    createdCObject.exception = static_cast<void *>(Exception::create(plzma_error_code_unknown, nullptr, __FILE__, __LINE__)); \
} \
return createdCObject; \


#else
#define LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH \
} catch (const Exception & exception) { \
    createdCObject.exception = static_cast<void *>(exception.moveToHeapCopy()); \
} catch (...) { \
    createdCObject.exception = static_cast<void *>(Exception::create(plzma_error_code_unknown, nullptr, __FILE__, __LINE__)); \
} \
return createdCObject; \


#endif // LIBPLZMA_HAVE_STD


// Executing

#define LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(OBJ_PTR, FAIL_RES) \
if (OBJ_PTR->exception) return FAIL_RES; \
try { \



#define LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(OBJ_PTR) \
if (OBJ_PTR->exception) return; \
try { \



#define LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN_WITH_ARG1(OBJ_PTR, ARG1_PTR, FAIL_RES) \
if (OBJ_PTR->exception || ARG1_PTR->exception) return FAIL_RES; \
try { \



#define LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN_WITH_ARG2(OBJ_PTR, ARG1_PTR, ARG2_PTR, FAIL_RES) \
if (OBJ_PTR->exception || ARG1_PTR->exception || ARG2_PTR->exception) return FAIL_RES; \
try { \



#if defined(LIBPLZMA_HAVE_STD)
#define LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(OBJ_PTR, FAIL_RES) \
} catch (const Exception & exception) { \
    OBJ_PTR->exception = static_cast<void *>(exception.moveToHeapCopy()); \
} catch (const std::exception & exception) { \
    OBJ_PTR->exception = static_cast<void *>(Exception::create(plzma_error_code_internal, exception.what(), __FILE__, __LINE__)); \
} catch (...) { \
    OBJ_PTR->exception = static_cast<void *>(Exception::create(plzma_error_code_unknown, nullptr, __FILE__, __LINE__)); \
} \
return FAIL_RES; \


#else
#define LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(OBJ_PTR, FAIL_RES) \
} catch (const Exception & exception) { \
    OBJ_PTR->exception = static_cast<void *>(exception.moveToHeapCopy()); \
} catch (...) { \
    OBJ_PTR->exception = static_cast<void *>(Exception::create(plzma_error_code_unknown, nullptr, __FILE__, __LINE__)); \
} \
return FAIL_RES; \


#endif // LIBPLZMA_HAVE_STD


#if defined(LIBPLZMA_HAVE_STD)
#define LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(OBJ_PTR) \
} catch (const Exception & exception) { \
    OBJ_PTR->exception = static_cast<void *>(exception.moveToHeapCopy()); \
} catch (const std::exception & exception) { \
    OBJ_PTR->exception = static_cast<void *>(Exception::create(plzma_error_code_internal, exception.what(), __FILE__, __LINE__)); \
}  catch (...) { \
    OBJ_PTR->exception = static_cast<void *>(Exception::create(plzma_error_code_unknown, nullptr, __FILE__, __LINE__)); \
} \


#else
#define LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(OBJ_PTR) \
} catch (const Exception & exception) { \
    OBJ_PTR->exception = static_cast<void *>(exception.moveToHeapCopy()); \
} catch (...) { \
    OBJ_PTR->exception = static_cast<void *>(Exception::create(plzma_error_code_unknown, nullptr, __FILE__, __LINE__)); \
} \


#endif // LIBPLZMA_HAVE_STD


#endif // !LIBPLZMA_NO_C_BINDINGS
#endif // !__PLZMA_C_BINDINGS_PRIVATE_HPP__
