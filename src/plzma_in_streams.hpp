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


#ifndef __PLZMA_IN_STREAMS_HPP__
#define __PLZMA_IN_STREAMS_HPP__ 1

#include <cstddef>

#include "../libplzma.hpp"
#include "plzma_private.hpp"
#include "plzma_mutex.hpp"

#include "CPP/Common/Common.h"
#include "CPP/Common/MyWindows.h"
#include "CPP/Common/MyString.h"
#include "CPP/Common/MyCom.h"
#include "CPP/7zip/IStream.h"
#include "CPP/7zip/Archive/Common/MultiStream.h"

namespace plzma {
    
    class InStreamBase :
        public InStream,
        public IInStream,
        public CMyUnknownImp {
    private:
        friend struct SharedPtr<InStreamBase>;
        LIBPLZMA_NON_COPYABLE_NON_MOVABLE(InStreamBase)
        
    protected:
        LIBPLZMA_MUTEX(mutable _mutex)
        
        virtual void retain() override final;
        virtual void release() override final;
        virtual void * base() noexcept override final { return this; }
        
    public:
        virtual void open() = 0;
        virtual void close() = 0;
        
        InStreamBase();
        virtual ~InStreamBase() noexcept { }
    };
    
    template<>
    template<>
    inline SharedPtr<InStreamBase> SharedPtr<InStream>::cast() const {
        return SharedPtr<InStreamBase>(static_cast<InStreamBase *>(_ptr ? _ptr->base() : nullptr));
    }
    
    class InFileStream final : public InStreamBase {
    private:
        Path _path;
        FILE * _file = nullptr;
        
        LIBPLZMA_NON_COPYABLE_NON_MOVABLE(InFileStream)
        
    public:
        MY_UNKNOWN_IMP1(IInStream)
        
        STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition);
        STDMETHOD(Read)(void * data, UInt32 size, UInt32 * processedSize);
        
        virtual void open() final;
        virtual void close() final;
        
        virtual bool opened() const final;
        virtual bool erase(const plzma_erase eraseType = plzma_erase_none) final;
        
        const Path & path() const noexcept;
        
        InFileStream(const Path & path);
        InFileStream(Path && path);
        virtual ~InFileStream() noexcept;
    };
    
    class InMemStream final : public InStreamBase {
    private:
        void * _memory = nullptr;
        plzma_free_callback _freeCallback = nullptr;
        UInt64 _size = 0;
        UInt64 _offset = 0;
        bool _opened = false;
        
        LIBPLZMA_NON_COPYABLE_NON_MOVABLE(InMemStream)
        
    public:
        MY_UNKNOWN_IMP1(IInStream)
        
        STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition);
        STDMETHOD(Read)(void * data, UInt32 size, UInt32 * processedSize);
        
        virtual void open() final;
        virtual void close() final;
        
        virtual bool opened() const final;
        virtual bool erase(const plzma_erase eraseType = plzma_erase_none) final;
        
        InMemStream(const void * memory, const size_t size);
        InMemStream(void * memory, const size_t size, plzma_free_callback freeCallback);
        
        virtual ~InMemStream() noexcept;
    };

    class InCallbackStream final : public InStreamBase {
    private:
        plzma_context _context;
        plzma_in_stream_open_callback _openCallback = nullptr;
        plzma_in_stream_close_callback _closeCallback = nullptr;
        plzma_in_stream_seek_callback _seekCallback = nullptr;
        plzma_in_stream_read_callback _readCallback = nullptr;
        bool _opened = false;
        
        LIBPLZMA_NON_COPYABLE_NON_MOVABLE(InCallbackStream)
        
    public:
        MY_UNKNOWN_IMP1(IInStream)
        
        STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition);
        STDMETHOD(Read)(void * data, UInt32 size, UInt32 * processedSize);
        
        virtual void open() final;
        virtual void close() final;
        
        virtual bool opened() const final;
        virtual bool erase(const plzma_erase eraseType = plzma_erase_none) final;
        
        InCallbackStream(plzma_in_stream_open_callback openCallback,
                         plzma_in_stream_close_callback closeCallback,
                         plzma_in_stream_seek_callback seekCallback,
                         plzma_in_stream_read_callback readCallback,
                         const plzma_context context);
        
        virtual ~InCallbackStream() noexcept;
    };

    class InMultiStream final : public InStreamBase {
    private:
        Vector<SharedPtr<InStreamBase> > _streams;
        CMultiStream _stream;
        bool _opened = false;
        
        LIBPLZMA_NON_COPYABLE_NON_MOVABLE(InMultiStream)
        
    public:
        MY_UNKNOWN_IMP1(IInStream)
        
        STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition);
        STDMETHOD(Read)(void * data, UInt32 size, UInt32 * processedSize);
        
        virtual void open() final;
        virtual void close() final;
        
        virtual bool opened() const final;
        virtual bool erase(const plzma_erase eraseType = plzma_erase_none) final;
        
        InMultiStream(InStreamArray && streams);
        virtual ~InMultiStream() noexcept;
    };

} // namespace plzma

#endif // !__PLZMA_IN_STREAMS_HPP__
