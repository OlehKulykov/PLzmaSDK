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


#ifndef __PLZMA_PROGRESS_HPP__
#define __PLZMA_PROGRESS_HPP__ 1

#include <cstddef>

#include "plzma_private.hpp"

#if !defined(LIBPLZMA_NO_PROGRESS)
#include "plzma_mutex.hpp"
#include "plzma_c_bindings_private.hpp"

namespace plzma {
    
    class Progress final {
    private:
        friend struct SharedPtr<Progress>;
        
        struct ReportData final {
        private:
            String _path;
            void * _context = nullptr;
            ProgressDelegate * _delegate = nullptr;
#if !defined(LIBPLZMA_NO_C_BINDINGS)
            plzma_progress_delegate_utf8_callback _utf8Callback = nullptr;
            plzma_progress_delegate_wide_callback _wideCallback = nullptr;
#endif
            double _progress = 0.0;
            
        public:
            void report() const {
                if (_delegate) {
                    _delegate->onProgress(_context, _path, _progress);
                }
#if !defined(LIBPLZMA_NO_C_BINDINGS)
                if (_utf8Callback) {
                    _utf8Callback(_context, _path.utf8(), _progress);
                }
                if (_wideCallback) {
                    _wideCallback(_context, _path.wide(), _progress);
                }
#endif
            }
            
            ReportData & operator = (ReportData &&) = delete;
            ReportData & operator = (const ReportData &) = delete;
            ReportData(const ReportData &) = delete;
            ReportData() = delete;
            
            ReportData(ReportData && data) noexcept :
                _path(static_cast<String &&>(data._path)),
                _context(data._context),
                _delegate(data._delegate),
#if !defined(LIBPLZMA_NO_C_BINDINGS)
                _utf8Callback(data._utf8Callback),
                _wideCallback(data._wideCallback),
#endif
                _progress(data._progress) { }
            
            ReportData(const Path & path,
                       void * context,
                       ProgressDelegate * delegate,
#if !defined(LIBPLZMA_NO_C_BINDINGS)
                       plzma_progress_delegate_utf8_callback utf8Callback,
                       plzma_progress_delegate_wide_callback wideCallback,
#endif
                       const double progress) :
                _path(path),
                _context(context),
                _delegate(delegate),
#if !defined(LIBPLZMA_NO_C_BINDINGS)
                _utf8Callback(utf8Callback),
                _wideCallback(wideCallback),
#endif
                _progress(progress) { }
            
            ~ReportData() noexcept {
                _path.clear(plzma_erase_zero);
            }
        };
        
        LIBPLZMA_MUTEX(_mutex)
        Path _path;
        plzma_context _context = plzma_context{nullptr, nullptr}; // C2059 = { .context = nullptr, .deinitializer = nullptr }
        ProgressDelegate * _delegate = nullptr;
#if !defined(LIBPLZMA_NO_C_BINDINGS)
        plzma_progress_delegate_utf8_callback _utf8Callback = nullptr;
        plzma_progress_delegate_wide_callback _wideCallback = nullptr;
#endif
        uint64_t _partCompleted = 0;
        uint64_t _partTotal = 0;
        double _progress = 0.0;
        double _perPart = 0.0;
        uint32_t _partsCount = 1;
        uint32_t _partNumber = 0;
        uint16_t _referenceCounter = 0;
        bool _reportable = false;
        
#if defined(LIBPLZMA_THREAD_UNSAFE)
        void retain() noexcept;
        void release() noexcept;
#else
        void retain();
        void release();
#endif
        bool calculateReportable() const noexcept;
        void updateProgress() noexcept;
        
        ReportData reportData() const {
            return ReportData(_path, _context.context, _delegate,
#if !defined(LIBPLZMA_NO_C_BINDINGS)
                              _utf8Callback,
                              _wideCallback,
#endif
                              _progress);
        }
        
        LIBPLZMA_NON_COPYABLE_NON_MOVABLE(Progress)
        
    public:
        void reset();
        void setDelegate(ProgressDelegate * delegate);
#if !defined(LIBPLZMA_NO_C_BINDINGS)
        void setUtf8Callback(plzma_progress_delegate_utf8_callback callback);
        void setWideCallback(plzma_progress_delegate_wide_callback callback);
#endif
        void setPartsCount(const uint32_t partsCount);
        void startPart();
        void setTotal(const uint64_t total);
        void setCompleted(const uint64_t completed);
        void finish(); // 1.0
        void setPath(Path && path);
        void setPath(const Path & path);
        Progress(const plzma_context context) : _context(context) { }
        ~Progress() noexcept;
    };
    
} // namespace plzma

#endif // !LIBPLZMA_NO_PROGRESS
#endif // !__PLZMA_PROGRESS_HPP__
