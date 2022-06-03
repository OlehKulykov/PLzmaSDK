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


#ifndef __PLZMA_MUTEX_HPP__
#define __PLZMA_MUTEX_HPP__ 1

#include <cstddef>

#include "../libplzma.hpp"
#include "plzma_private.hpp"

#if defined(LIBPLZMA_HAVE_STD)
#include <mutex>
#endif

#include "CPP/Common/Common.h"
#include "CPP/Common/MyWindows.h"

#if !defined(LIBPLZMA_THREAD_UNSAFE)
#if defined(LIBPLZMA_MSC)
#include <windows.h>
#elif defined(LIBPLZMA_POSIX)
#include <pthread.h>
#else
#error "Error once: file contains unimplemented functionality."
#endif

namespace plzma {
    
#if defined(LIBPLZMA_HAVE_STD)
    typedef std::mutex Mutex;
#else
    struct Mutex final {
    private:
#if defined(LIBPLZMA_MSC)
        mutable CRITICAL_SECTION _criticalSection;
#elif defined(LIBPLZMA_POSIX)
        mutable pthread_mutex_t _mutex;
#endif
        LIBPLZMA_NON_COPYABLE_NON_MOVABLE(Mutex)
        
    public:
        void lock() {
#if defined(LIBPLZMA_MSC)
            EnterCriticalSection(static_cast<LPCRITICAL_SECTION>(&_criticalSection));
#elif defined(LIBPLZMA_POSIX)
            if (pthread_mutex_lock(&_mutex) != 0) {
                throw Exception(plzma_error_code_internal, "Can't lock mutex.", __FILE__, __LINE__);
            }
#endif
        }
        
        void unlock() {
#if defined(LIBPLZMA_MSC)
            LeaveCriticalSection(static_cast<LPCRITICAL_SECTION>(&_criticalSection));
#elif defined(LIBPLZMA_POSIX)
            if (pthread_mutex_unlock(&_mutex) != 0) {
                throw Exception(plzma_error_code_internal, "Can't unlock mutex.", __FILE__, __LINE__);
            }
#endif
        }
        
        Mutex() {
#if defined(LIBPLZMA_MSC)
            InitializeCriticalSection(static_cast<LPCRITICAL_SECTION>(&_criticalSection));
#elif defined(LIBPLZMA_POSIX)
            pthread_mutexattr_t attr;
            int maiRes, miRes = -1;
            if ( (maiRes = pthread_mutexattr_init(&attr)) == 0 ) {
                if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_DEFAULT) == 0) {
                    miRes = pthread_mutex_init(&_mutex, &attr);
                }
                if (maiRes == 0) {
                    maiRes = pthread_mutexattr_destroy(&attr);
                }
            }
            if (miRes != 0 || maiRes != 0) {
                throw Exception(plzma_error_code_internal, "Can't initialize mutex.", __FILE__, __LINE__);
            }
#endif
        }
        
        ~Mutex() noexcept {
#if defined(LIBPLZMA_MSC)
            DeleteCriticalSection(static_cast<LPCRITICAL_SECTION>(&_criticalSection));
#elif defined(LIBPLZMA_POSIX)
            pthread_mutex_destroy(&_mutex); // ignore return res.
#endif
        }
    };

    struct LockGuard final {
    private:
        Mutex & _mutex;
        bool _locked = true;
        
        LIBPLZMA_NON_COPYABLE_NON_MOVABLE(LockGuard)
        
    public:
        void lock() {
            _locked = true;
            _mutex.lock();
        }
        
        void unlock() {
            _mutex.unlock();
            _locked = false;
        }
        
        LockGuard(Mutex & mutex) :
            _mutex(mutex) {
                _mutex.lock();
        }
        
        ~LockGuard() {
            if (_locked) {
                _mutex.unlock();
            }
        }
    };
#endif // !LIBPLZMA_HAVE_STD
    
    struct FailableLockGuard final {
    private:
        Mutex & _mutex;
        HRESULT _res = S_OK;
        bool _locked = true;
        
        LIBPLZMA_NON_COPYABLE_NON_MOVABLE(FailableLockGuard)
        
    public:
        HRESULT res() const noexcept {
            return _res;
        }
        
        void lock() noexcept {
            LIBPLZMA_DEBUG_ASSERT(_res == S_OK)
            _locked = true;
            try {
                _mutex.lock();
            } catch (...) {
                _res = E_FAIL;
            }
        }
        
        void unlock() noexcept {
            LIBPLZMA_DEBUG_ASSERT(_res == S_OK)
            try {
                _mutex.unlock();
                _locked = false;
            } catch (...) {
                _res = E_FAIL;
            }
        }
        
        FailableLockGuard(Mutex & mutex) noexcept :
            _mutex(mutex) {
                try {
                    _mutex.lock();
                } catch (...) {
                    _res = E_FAIL;
                }
        }
        
        ~FailableLockGuard() noexcept {
            if (_locked && (_res == S_OK)) {
                try {
                    _mutex.unlock();
                } catch (...) {
                    // do nothing
                }
            }
        }
    };

#if defined(LIBPLZMA_HAVE_STD)
#define LIBPLZMA_MUTEX(NAME) std::mutex NAME;

#define LIBPLZMA_LOCKGUARD(NAME,MUTEX) const std::lock_guard<std::mutex> NAME(MUTEX);

#define LIBPLZMA_UNIQUE_LOCK(NAME,MUTEX) std::unique_lock<std::mutex> NAME(MUTEX);

#define LIBPLZMA_UNIQUE_LOCK_LOCK(NAME) NAME.lock();

#define LIBPLZMA_UNIQUE_LOCK_UNLOCK(NAME) NAME.unlock();

#else
#define LIBPLZMA_MUTEX(NAME) Mutex NAME;

#define LIBPLZMA_LOCKGUARD(NAME,MUTEX) const LockGuard NAME(MUTEX);

#define LIBPLZMA_UNIQUE_LOCK(NAME,MUTEX) LockGuard NAME(MUTEX);

#define LIBPLZMA_UNIQUE_LOCK_LOCK(NAME) NAME.lock();

#define LIBPLZMA_UNIQUE_LOCK_UNLOCK(NAME) NAME.unlock();

#endif // !LIBPLZMA_HAVE_STD

} // namespace plzma

#endif // !LIBPLZMA_THREAD_UNSAFE

#if defined(LIBPLZMA_THREAD_UNSAFE)
#define LIBPLZMA_MUTEX(NAME)

#define LIBPLZMA_LOCKGUARD(NAME,MUTEX)

#define LIBPLZMA_UNIQUE_LOCK(NAME,MUTEX)

#define LIBPLZMA_UNIQUE_LOCK_LOCK(NAME)

#define LIBPLZMA_UNIQUE_LOCK_UNLOCK(NAME)

#endif // LIBPLZMA_THREAD_UNSAFE

#endif // !__PLZMA_MUTEX_HPP__

