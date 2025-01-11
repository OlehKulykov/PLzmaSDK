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


#ifndef __PLZMA_ENCODER_IMPL_HPP__
#define __PLZMA_ENCODER_IMPL_HPP__ 1

#include <cstddef>

#include "../libplzma.hpp"
#include "plzma_private.hpp"
#include "plzma_in_streams.hpp"
#include "plzma_out_streams.hpp"
#include "plzma_open_callback.hpp"
#include "plzma_extract_callback.hpp"
#include "plzma_common.hpp"

#include "CPP/Common/Common.h"
#include "CPP/Common/MyWindows.h"
#include "CPP/Common/MyString.h"
#include "CPP/Common/MyCom.h"
#include "CPP/7zip/Common/FileStreams.h"
#include "CPP/7zip/Archive/IArchive.h"
#include "CPP/7zip/IPassword.h"
#include "CPP/7zip/ICoder.h"
#include "CPP/Windows/PropVariant.h"

namespace plzma {
    
    class EncoderImpl final :
        public IArchiveUpdateCallback2,
        public ICryptoGetTextPassword,
        public ICryptoGetTextPassword2,
        public Encoder,
        public BaseCallback,
        public CMyUnknownImp {
    private:
        friend struct SharedPtr<EncoderImpl>;
        enum Option : uint16_t {
            OptionSolid                 = 1 << 0,
            OptionCompressHeader        = 1 << 1,
            OptionCompressHeaderFull    = 1 << 2,
            OptionEncryptContent        = 1 << 3,
            OptionEncryptHeader         = 1 << 4,
            OptionStoreCTime            = 1 << 5,
            OptionStoreMTime            = 1 << 6,
            OptionStoreATime            = 1 << 7,
            
            OptionRequirePassword       = OptionEncryptContent | OptionEncryptHeader
        };
        struct AddedPath final {
            Path path;
            Path archivePath;
            plzma_open_dir_mode_t openDirMode = 0;
            bool isDir = false;
        };
        struct AddedFile final {
            Path path;
            Path archivePath;
            plzma_path_stat stat;
        };
        struct AddedSubDir final {
            Path path;
            Vector<AddedFile> files;
        };
        struct AddedStream final {
            SharedPtr<InStreamBase> stream;
            Path archivePath;
            plzma_path_stat stat;
        };
        CMyComPtr<OutStreamBase> _stream;
        CMyComPtr<IOutArchive> _archive;
        Vector<AddedPath> _paths;
        Vector<AddedSubDir> _subDirs;
        Vector<AddedFile> _files;
        Vector<AddedStream> _streams;
        struct Source final {
            Path path;
            Path archivePath;
            SharedPtr<InStreamBase> stream;
            plzma_path_stat stat;
            UInt32 itemIndex;
            void close() {
                if (stream) {
                    stream->close();
                    stream.clear();
                }
                path.clear(plzma_erase_zero);
                archivePath.clear(plzma_erase_zero);
            }
        } _source;
        plzma_file_type _type = plzma_file_type_7z;
        plzma_method _method = plzma_method_LZMA;
        UInt32 _itemsCount = 0;
        uint16_t _options = 0;
        uint8_t _compressionLevel = 7;
        bool _opening = false;
        bool _compressing = false;
        
        virtual void retain() override final;
        virtual void release() override final;
        
        uint64_t processAddedPaths();
        HRESULT setupSource(UInt32 index);
        void applySettings7z(ISetProperties * properties);
        void applySettingsXz(ISetProperties * properties);
        void applySettingsTar(ISetProperties * properties);
        void applySettings();
        bool hasOption(const Option option) const;
        void setOption(const Option option, const bool set);
        
    protected:
        LIBPLZMA_NON_COPYABLE_NON_MOVABLE(EncoderImpl)
        
    public:
        Z7_COM_UNKNOWN_IMP_3(IArchiveUpdateCallback2, ICryptoGetTextPassword, ICryptoGetTextPassword2)
        
    public:
        // IProgress
        STDMETHOD(SetTotal)(UInt64 size) throw() override final;
        STDMETHOD(SetCompleted)(const UInt64 * completeValue) throw() override final;
        
        // IUpdateCallback2
        STDMETHOD(GetUpdateItemInfo)(UInt32 index, Int32 * newData, Int32 * newProperties, UInt32 * indexInArchive) throw() override final;
        STDMETHOD(GetProperty)(UInt32 index, PROPID propID, PROPVARIANT * value) throw() override final;
        STDMETHOD(GetStream)(UInt32 index, ISequentialInStream ** inStream) throw() override final;
        STDMETHOD(SetOperationResult)(Int32 operationResult) throw() override final;
        STDMETHOD(GetVolumeSize)(UInt32 index, UInt64 * size) throw() override final;
        STDMETHOD(GetVolumeStream)(UInt32 index, ISequentialOutStream ** volumeStream) throw() override final;
        
        // ICryptoGetTextPassword
        STDMETHOD(CryptoGetTextPassword)(BSTR * password) throw() override final;
        
        // ICryptoGetTextPassword2
        STDMETHOD(CryptoGetTextPassword2)(Int32 * passwordIsDefined, BSTR * password) throw() override final;
        
        virtual void setPassword(const wchar_t * password) override final;
        virtual void setPassword(const char * password) override final;
        virtual void setProgressDelegate(ProgressDelegate * delegate) override final;
        virtual void add(const Path & path, const plzma_open_dir_mode_t openDirMode = 0, const Path & archivePath = Path()) override final;
        virtual void add(const SharedPtr<InStream> & stream, const Path & archivePath) override final;
        virtual bool open() override final;
        virtual void abort() override final;
        virtual bool compress() override final;
        virtual bool shouldCreateSolidArchive() const override final;
        virtual void setShouldCreateSolidArchive(const bool solid) override final;
        virtual uint8_t compressionLevel() const override final;
        virtual void setCompressionLevel(const uint8_t level) override final;
        virtual bool shouldCompressHeader() const override final;
        virtual void setShouldCompressHeader(const bool compress) override final;
        virtual bool shouldCompressHeaderFull() const override final;
        virtual void setShouldCompressHeaderFull(const bool compress) override final;
        virtual bool shouldEncryptContent() const override final;
        virtual void setShouldEncryptContent(const bool encrypt) override final;
        virtual bool shouldEncryptHeader() const override final;
        virtual void setShouldEncryptHeader(const bool encrypt) override final;
        virtual bool shouldStoreCreationTime() const override final;
        virtual void setShouldStoreCreationTime(const bool store) override final;
        virtual bool shouldStoreAccessTime() const override final;
        virtual void setShouldStoreAccessTime(const bool store) override final;
        virtual bool shouldStoreModificationTime() const override final;
        virtual void setShouldStoreModificationTime(const bool store) override final;
        
#if !defined(LIBPLZMA_NO_C_BINDINGS)
        void setUtf8Callback(plzma_progress_delegate_utf8_callback callback);
        void setWideCallback(plzma_progress_delegate_wide_callback callback);
#endif
        
        EncoderImpl(const CMyComPtr<OutStreamBase> & stream,
                    const plzma_file_type type,
                    const plzma_method method,
                    const plzma_context context);
        virtual ~EncoderImpl();
    };
    
} // namespace plzma

#endif // !__PLZMA_ENCODER_IMPL_HPP__
