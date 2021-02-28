//
// By using this Software, you are accepting original [LZMA SDK] and MIT license below:
//
// The MIT License (MIT)
//
// Copyright (c) 2015 - 2021 Oleh Kulykov <olehkulykov@gmail.com>
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


#ifndef __PLZMA_OUT_STREAMS_HPP__
#define __PLZMA_OUT_STREAMS_HPP__ 1

#include <cstddef>

#include "../libplzma.hpp"
#include "plzma_private.hpp"
#include "plzma_in_streams.hpp" // all headers + delegate
#include "plzma_mutex.hpp"

#if !defined(LIBPLZMA_NO_PROGRESS)
#include "plzma_progress.hpp"
#endif

namespace plzma {

    class OutStreamBase :
        public OutStream,
        public IOutStream,
        public CMyUnknownImp {
    private:
        friend struct SharedPtr<OutStreamBase>;
        LIBPLZMA_NON_COPYABLE_NON_MOVABLE(OutStreamBase)
        
    protected:
        LIBPLZMA_MUTEX(_mutex)
        
        virtual void retain() override final;
        virtual void release() override final;
        virtual void * base() noexcept override final { return this; }
        
    public:
        virtual void open() = 0;
        virtual void close() = 0;
        
        OutStreamBase();
        virtual ~OutStreamBase() noexcept { }
    };
    
    template<>
    template<>
    inline SharedPtr<OutStreamBase> SharedPtr<OutStream>::cast() const {
        return SharedPtr<OutStreamBase>(static_cast<OutStreamBase *>(_ptr ? _ptr->base() : nullptr));
    }
    
    class OutFileStream final : public OutStreamBase {
    private:
        Path _path;
        FILE * _file = nullptr;
        
        LIBPLZMA_NON_COPYABLE_NON_MOVABLE(OutFileStream)
        
    public:
        MY_UNKNOWN_IMP1(IOutStream)
        
        STDMETHOD(Write)(const void * data, UInt32 size, UInt32 * processedSize);
        STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition);
        STDMETHOD(SetSize)(UInt64 newSize);
        
        virtual void open();
        virtual void close();
        
        virtual bool opened() const;
        virtual bool erase(const plzma_erase eraseType = plzma_erase_none);
        virtual RawHeapMemorySize copyContent() const;
        
        OutFileStream(const Path & path);
        OutFileStream(Path && path);
        virtual ~OutFileStream() noexcept;
    };
    
    class OutMemStream final : public OutStreamBase {
    private:
        RawHeapMemory _memory;
        uint64_t _size = 0;
        uint64_t _offset = 0;
        bool _opened = false;
        
        LIBPLZMA_NON_COPYABLE_NON_MOVABLE(OutMemStream)
        
    public:
        MY_UNKNOWN_IMP1(IOutStream)
        
        STDMETHOD(Write)(const void * data, UInt32 size, UInt32 * processedSize);
        STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition);
        STDMETHOD(SetSize)(UInt64 newSize);
        
        virtual void open();
        virtual void close();
        
        virtual bool opened() const;
        virtual bool erase(const plzma_erase eraseType = plzma_erase_none);
        RawHeapMemorySize copyContent() const;
        
        OutMemStream() = default;
        virtual ~OutMemStream() noexcept { }
    };
    
    class OutTestStream final : public OutStreamBase {
    private:
        bool _opened;
        
        LIBPLZMA_NON_COPYABLE_NON_MOVABLE(OutTestStream)
    
    public:
        MY_UNKNOWN_IMP1(IOutStream)
        
        STDMETHOD(Write)(const void * data, UInt32 size, UInt32 * processedSize);
        STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition);
        STDMETHOD(SetSize)(UInt64 newSize);
        
        virtual void open();
        virtual void close();
        
        virtual bool opened() const;
        virtual bool erase(const plzma_erase eraseType = plzma_erase_none);
        virtual RawHeapMemorySize copyContent() const;
        
        OutTestStream() = default;
        virtual ~OutTestStream() noexcept { }
    };

} // namespace plzma

#endif // !__PLZMA_OUT_STREAMS_HPP__
