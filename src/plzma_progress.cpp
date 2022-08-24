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


#include <cstddef>

#include "../libplzma.hpp"
#include "plzma_private.hpp"
#include "plzma_progress.hpp"

#if !defined(LIBPLZMA_NO_PROGRESS)

#include "CPP/Common/Defs.h"

namespace plzma {

#if defined(LIBPLZMA_THREAD_UNSAFE)
    void Progress::retain() noexcept {
        LIBPLZMA_RETAIN_IMPL(_referenceCounter)
    }
    
    void Progress::release() noexcept {
        LIBPLZMA_RELEASE_IMPL(_referenceCounter)
    }
#else
    void Progress::retain() {
        LIBPLZMA_RETAIN_LOCKED_IMPL(_referenceCounter, _mutex)
    }
    
    void Progress::release() {
        LIBPLZMA_RELEASE_LOCKED_IMPL(_referenceCounter, _mutex)
    }
#endif
    
    bool Progress::calculateReportable() const noexcept {
#if defined(LIBPLZMA_NO_C_BINDINGS)
        return _delegate != nullptr;
#else
        return (_delegate || _utf8Callback || _wideCallback) ? true : false;
#endif
    }
    
    void Progress::updateProgress() noexcept {
        _progress = (_partNumber > 0) ? (_perPart * (_partNumber - 1)) : 0.0;
        if (_partTotal > 0) {
            const double inc = static_cast<double>(_partCompleted) / static_cast<double>(_partTotal);
            _progress += (_perPart * MyMin<double>(inc, 1.0));
        }
    }
    
    void Progress::reset() {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        _path.clear(plzma_erase_zero);
        _partsCount = 1;
        _partNumber = 0;
        _partCompleted = _partTotal = 0;
        _progress = _perPart = 0.0;
    }
    
    void Progress::setDelegate(ProgressDelegate * delegate) {
        LIBPLZMA_UNIQUE_LOCK(lock, _mutex)
        _delegate = delegate;
        if ( (_reportable = calculateReportable()) && _progress > 0.0) {
            const auto report = reportData();
            LIBPLZMA_UNIQUE_LOCK_UNLOCK(lock)
            report.report();
        }
    }
    
#if !defined(LIBPLZMA_NO_C_BINDINGS)
    void Progress::setUtf8Callback(plzma_progress_delegate_utf8_callback callback) {
        LIBPLZMA_UNIQUE_LOCK(lock, _mutex)
        _utf8Callback = callback;
        if ( (_reportable = calculateReportable()) && _progress > 0.0) {
            const auto report = reportData();
            LIBPLZMA_UNIQUE_LOCK_UNLOCK(lock)
            report.report();
        }
    }
    
    void Progress::setWideCallback(plzma_progress_delegate_wide_callback callback) {
        LIBPLZMA_UNIQUE_LOCK(lock, _mutex)
        _wideCallback = callback;
        _reportable = calculateReportable();
        if ( (_reportable = calculateReportable()) && _progress > 0.0) {
            const auto report = reportData();
            LIBPLZMA_UNIQUE_LOCK_UNLOCK(lock)
            report.report();
        }
    }
#endif // !LIBPLZMA_NO_C_BINDINGS
    
    void Progress::setPartsCount(const uint32_t partsCount) {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        if ( (_partsCount = partsCount) > 0 ) {
            _perPart = 1.0 / static_cast<double>(partsCount);
        } else {
            _perPart = 0.0;
        }
    }
    
    void Progress::startPart() {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        _partNumber++;
        _partCompleted = _partTotal = 0;
    }
    
    void Progress::setCompleted(const uint64_t completed) {
        LIBPLZMA_UNIQUE_LOCK(lock, _mutex)
        _partCompleted = completed;
        const auto prevProgress = _progress;
        updateProgress();
        if (_reportable && prevProgress != _progress) {
            const auto report = reportData();
            LIBPLZMA_UNIQUE_LOCK_UNLOCK(lock)
            report.report();
        }
    }
    
    void Progress::setTotal(const uint64_t total) {
        LIBPLZMA_UNIQUE_LOCK(lock, _mutex)
        _partTotal = total;
        const auto prevProgress = _progress;
        updateProgress();
        if (_reportable && prevProgress != _progress) {
            const auto report = reportData();
            LIBPLZMA_UNIQUE_LOCK_UNLOCK(lock)
            report.report();
        }
    }
    
    void Progress::finish() {
        LIBPLZMA_UNIQUE_LOCK(lock, _mutex)
        _partNumber = _partsCount;
        _partCompleted = _partTotal;
        const auto prevProgress = _progress;
        updateProgress();
        if (_reportable && prevProgress != _progress) {
            const auto report = reportData();
            LIBPLZMA_UNIQUE_LOCK_UNLOCK(lock)
            report.report();
        }
    }
    
    void Progress::setPath(Path && path) {
        LIBPLZMA_UNIQUE_LOCK(lock, _mutex)
        _path = static_cast<Path &&>(path);
        if (_reportable) {
            const auto report = reportData();
            LIBPLZMA_UNIQUE_LOCK_UNLOCK(lock)
            report.report();
        }
    }
    
    void Progress::setPath(const Path & path) {
        LIBPLZMA_UNIQUE_LOCK(lock, _mutex)
        _path = path;
        if (_reportable) {
            const auto report = reportData();
            LIBPLZMA_UNIQUE_LOCK_UNLOCK(lock)
            report.report();
        }
    }
    
    Progress::~Progress() noexcept {
        if (_context.context && _context.deinitializer) {
            _context.deinitializer(_context.context);
        }
    }
    
} // namespace plzma

#endif // !LIBPLZMA_NO_PROGRESS
