//
// By using this Software, you are accepting original [LZMA SDK] and MIT license below:
//
// The MIT License (MIT)
//
// Copyright (c) 2015 - 2025 Oleh Kulykov <olehkulykov@gmail.com>
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

    class LIBPLZMA_CPP_CLASS_API OutStreamBase :
        public OutStream,
        public IOutStream,
        public CMyUnknownImp {
    private:
        friend struct SharedPtr<OutStreamBase>;
        
    protected:
        LIBPLZMA_MUTEX(mutable _mutex)
        
        virtual void retain() override;
        virtual void release() override;
        virtual void * base() noexcept override { return this; }
            
        LIBPLZMA_NON_COPYABLE_NON_MOVABLE(OutStreamBase)
        
    public:
        virtual void setTimestamp(const plzma_path_timestamp & timestamp) = 0;
        virtual void open() = 0;
        virtual void close() = 0;
        virtual Exception * takeException() noexcept { return nullptr; }
        
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
        plzma_path_timestamp _timestamp{0, 0, 0};
        
    protected:
        LIBPLZMA_NON_COPYABLE_NON_MOVABLE(OutFileStream)
        
    public:
        Z7_COM_UNKNOWN_IMP_1(IOutStream)
    
    public:
        STDMETHOD(Write)(const void * data, UInt32 size, UInt32 * processedSize) throw() override final;
        STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition) throw() override final;
        STDMETHOD(SetSize)(UInt64 newSize) throw() override final;
        
        virtual void setTimestamp(const plzma_path_timestamp & timestamp) override final;
        virtual void open() override final;
        virtual void close() override final;
        
        virtual bool opened() const final;
        virtual bool erase(const plzma_erase eraseType = plzma_erase_none) override final;
        virtual RawHeapMemorySize copyContent() const override final;
        
        OutFileStream(const Path & path);
        OutFileStream(Path && path);
        virtual ~OutFileStream() noexcept;
    };
    
    class OutMemStream final : public OutStreamBase {
    private:
        RawHeapMemory _memory;
        Exception * _exception = nullptr;
        uint64_t _size = 0;
        uint64_t _offset = 0;
        bool _opened = false;
        
    protected:
        LIBPLZMA_NON_COPYABLE_NON_MOVABLE(OutMemStream)
        
    public:
        Z7_COM_UNKNOWN_IMP_1(IOutStream)
        
    public:
        STDMETHOD(Write)(const void * data, UInt32 size, UInt32 * processedSize) throw() override final;
        STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition) throw() override final;
        STDMETHOD(SetSize)(UInt64 newSize) throw() override final;
        
        virtual void setTimestamp(const plzma_path_timestamp & timestamp) override final { }
        virtual void open() override final;
        virtual void close() override final;
        virtual Exception * takeException() noexcept override final;
        
        virtual bool opened() const override final;
        virtual bool erase(const plzma_erase eraseType = plzma_erase_none) override final;
        RawHeapMemorySize copyContent() const override final;
        
        OutMemStream() = default;
        virtual ~OutMemStream() noexcept;
    };
    
    class OutTestStream final : public OutStreamBase {
    private:
        bool _opened;
        
    protected:
        LIBPLZMA_NON_COPYABLE_NON_MOVABLE(OutTestStream)
    
    public:
        Z7_COM_UNKNOWN_IMP_1(IOutStream)
        
    public:
        STDMETHOD(Write)(const void * data, UInt32 size, UInt32 * processedSize) throw() override final;
        STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition) throw() override final;
        STDMETHOD(SetSize)(UInt64 newSize) throw() override final;
        
        virtual void setTimestamp(const plzma_path_timestamp & timestamp) override final { }
        virtual void open() override final;
        virtual void close() override final;
        
        virtual bool opened() const override final;
        virtual bool erase(const plzma_erase eraseType = plzma_erase_none) override final;
        virtual RawHeapMemorySize copyContent() const override final;
        
        OutTestStream() = default;
        virtual ~OutTestStream() noexcept { }
    };

    class OutMultiStreamBase : public OutStreamBase, public OutMultiStream {
    private:
        friend struct SharedPtr<OutMultiStreamBase>;
        
    protected:
        Vector<SharedPtr<OutStreamBase> > _parts;
        Exception * _exception = nullptr;
        uint64_t _size = 0;
        uint64_t _offset = 0;
        plzma_size_t _partSize = 0;
        bool _opened = false;
        
        void clear();
        
        virtual void retain() override final { OutStreamBase::retain(); }
        virtual void release() override final { OutStreamBase::release(); }
        virtual void * base() noexcept override final { return this; }
        
        virtual SharedPtr<OutStreamBase> addPart() = 0;
        virtual void checkPartsCount(const uint64_t partsCount) const;
        void checkPartSize(const plzma_size_t partSize);
        
        LIBPLZMA_NON_COPYABLE_NON_MOVABLE(OutMultiStreamBase)
        
    public:
        Z7_COM_UNKNOWN_IMP_1(IOutStream)
        
    public:
        STDMETHOD(Write)(const void * data, UInt32 size, UInt32 * processedSize) throw() override final;
        STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition) throw() override final;
        STDMETHOD(SetSize)(UInt64 newSize) throw() override final;
        
        virtual void setTimestamp(const plzma_path_timestamp & timestamp) override final { }
        virtual void open() override final;
        virtual void close() override final;
        virtual Exception * takeException() noexcept override final;
        
        virtual bool opened() const override final;
        virtual bool erase(const plzma_erase eraseType = plzma_erase_none) override;
        virtual RawHeapMemorySize copyContent() const override final;
        
        virtual OutStreamArray streams() const override final;
        
        OutMultiStreamBase(const plzma_size_t partSize);
        OutMultiStreamBase() = delete;
        virtual ~OutMultiStreamBase() noexcept;
    };

    class OutMultiFileStream final : public OutMultiStreamBase {
    private:
        friend struct SharedPtr<OutMultiFileStream>;
        Path _dirPath;
        String _partName;
        String _partExtension;
        plzma_plzma_multi_stream_part_name_format _format = plzma_plzma_multi_stream_part_name_format_name_ext_00x;
        
    protected:
        virtual SharedPtr<OutStreamBase> addPart() override final;
        virtual void checkPartsCount(const uint64_t partsCount) const override final;
        void preparePath(const Path & path);
        
        LIBPLZMA_NON_COPYABLE_NON_MOVABLE(OutMultiFileStream)
        
    public:
        virtual bool erase(const plzma_erase eraseType = plzma_erase_none) override final;
        
        OutMultiFileStream(const Path & dirPath,
                           const String & partName,
                           const String & partExtension,
                           const plzma_plzma_multi_stream_part_name_format format,
                           const plzma_size_t partSize);
        
        OutMultiFileStream(Path && dirPath,
                           String && partName,
                           String && partExtension,
                           const plzma_plzma_multi_stream_part_name_format format,
                           const plzma_size_t partSize);
        
        OutMultiFileStream() = delete;
        virtual ~OutMultiFileStream() noexcept { }
    };

    class OutMultiMemStream final : public OutMultiStreamBase {
    private:
        friend struct SharedPtr<OutMultiMemStream>;
        
    protected:
        virtual SharedPtr<OutStreamBase> addPart() override final {
            const SharedPtr<OutStreamBase> stream(new OutMemStream());
            _parts.push(stream);
            return stream;
        }
        
        LIBPLZMA_NON_COPYABLE_NON_MOVABLE(OutMultiMemStream)
        
    public:
        OutMultiMemStream(const plzma_size_t partSize) : OutMultiStreamBase(partSize) {  }
        OutMultiMemStream() = delete;
        virtual ~OutMultiMemStream() noexcept { }
    };

    template<>
    template<>
    inline SharedPtr<OutStreamBase> SharedPtr<OutMultiStream>::cast() const {
        return SharedPtr<OutStreamBase>(static_cast<OutStreamBase *>(_ptr ? _ptr->base() : nullptr));
    }

} // namespace plzma

#endif // !__PLZMA_OUT_STREAMS_HPP__
