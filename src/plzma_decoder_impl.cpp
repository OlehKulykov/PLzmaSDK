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

#include "plzma_decoder_impl.hpp"

namespace plzma {
    
    void DecoderImpl::retain() {
#if defined(LIBPLZMA_THREAD_UNSAFE)
        LIBPLZMA_RETAIN_IMPL(__m_RefCount)
#else
        LIBPLZMA_RETAIN_LOCKED_IMPL(__m_RefCount, _mutex)
#endif
    }
    
    void DecoderImpl::release() {
#if defined(LIBPLZMA_THREAD_UNSAFE)
        LIBPLZMA_RELEASE_IMPL(__m_RefCount)
#else
        LIBPLZMA_RELEASE_LOCKED_IMPL(__m_RefCount, _mutex)
#endif
    }
    
    void DecoderImpl::setPassword(const wchar_t * LIBPLZMA_NULLABLE password) {
#if defined(LIBPLZMA_NO_CRYPTO)
        throw Exception(plzma_error_code_invalid_arguments, LIBPLZMA_NO_CRYPTO_EXCEPTION_WHAT, __FILE__, __LINE__);
#else
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        _password.clear(plzma_erase_zero);
        _password.set(password);
#endif
    }
    
    void DecoderImpl::setPassword(const char * LIBPLZMA_NULLABLE password) {
#if defined(LIBPLZMA_NO_CRYPTO)
        throw Exception(plzma_error_code_invalid_arguments, LIBPLZMA_NO_CRYPTO_EXCEPTION_WHAT, __FILE__, __LINE__);
#else
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        _password.clear(plzma_erase_zero);
        _password.set(password);
#endif
    }
    
    void DecoderImpl::setProgressDelegate(ProgressDelegate * LIBPLZMA_NULLABLE delegate) {
#if !defined(LIBPLZMA_NO_PROGRESS)
        _progress->setDelegate(delegate);
#endif
    }
    
    bool DecoderImpl::open() {
        LIBPLZMA_UNIQUE_LOCK(lock, _mutex)
        if (_opened || _opening) {
            return _opened;
        }
        
        CMyComPtr<DecoderImpl> selfPtr(this);
#if defined(LIBPLZMA_NO_CRYPTO)
        _openCallback = CMyComPtr<OpenCallback>(new OpenCallback(_stream, _type));
#else
        _openCallback = CMyComPtr<OpenCallback>(new OpenCallback(_stream, _password, _type));
#endif
        bool opened = false;
        _opening = true;
        
        LIBPLZMA_UNIQUE_LOCK_UNLOCK(lock)
        _stream->open();
        opened = _openCallback->open();
        LIBPLZMA_UNIQUE_LOCK_LOCK(lock)
        
        if (_aborted) {
            _stream->close();
        }
        
        _opening = false;
        return (_opened = opened);
    }
    
    bool DecoderImpl::extract(const Path & path, const bool usingItemsFullPath) {
        return process(NArchive::NExtract::NAskMode::kExtract, path, usingItemsFullPath);
    }
    
    bool DecoderImpl::extract(const SharedPtr<ItemArray> & items, const Path & path, const bool usingItemsFullPath) {
        if (_type == plzma_file_type_xz && items->count() > 1) {
            throw Exception(plzma_error_code_invalid_arguments, "Xz type supports only one item.", __FILE__, __LINE__);
        }
        return process(NArchive::NExtract::NAskMode::kExtract, items, path, usingItemsFullPath);
    }
    
    bool DecoderImpl::extract(const SharedPtr<ItemOutStreamArray> & items) {
        if (_type == plzma_file_type_xz && items->count() > 1) {
            throw Exception(plzma_error_code_invalid_arguments, "Xz type supports only one item.", __FILE__, __LINE__);
        }
        return process(NArchive::NExtract::NAskMode::kExtract, items);
    }
    
    bool DecoderImpl::test(const SharedPtr<ItemArray> & items) {
        if (_type == plzma_file_type_xz && items->count() > 1) {
            throw Exception(plzma_error_code_invalid_arguments, "Xz type supports only one item.", __FILE__, __LINE__);
        }
        return process(NArchive::NExtract::NAskMode::kTest, items);
    }
    
    bool DecoderImpl::test() {
        return process(NArchive::NExtract::NAskMode::kTest);
    }
    
    void DecoderImpl::abort() {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        _aborted = true;
        if (_openCallback) {
            _openCallback->abort();
        }
        CMyComPtr<ExtractCallback> extractCallback(static_cast<CMyComPtr<ExtractCallback> &&>(_extractCallback));
        if (extractCallback) {
            extractCallback->abort();
        }
        if ( !(_opening || extractCallback) ) {
            _stream->close();
        }
    }
    
    plzma_size_t DecoderImpl::count() const {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        return _opened ? _openCallback->itemsCount() : 0;
    }
    
    SharedPtr<ItemArray> DecoderImpl::items() const {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        return _opened ? _openCallback->allItems() : SharedPtr<ItemArray>();
    }
    
    SharedPtr<Item> DecoderImpl::itemAt(const plzma_size_t index) const {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        return _opened ? _openCallback->itemAt(index) : SharedPtr<Item>();
    }
    
#if !defined(LIBPLZMA_NO_C_BINDINGS)
    void DecoderImpl::setUtf8Callback(plzma_progress_delegate_utf8_callback LIBPLZMA_NULLABLE callback) {
#if !defined(LIBPLZMA_NO_PROGRESS)
        _progress->setUtf8Callback(callback);
#endif
    }
    
    void DecoderImpl::setWideCallback(plzma_progress_delegate_wide_callback LIBPLZMA_NULLABLE callback) {
#if !defined(LIBPLZMA_NO_PROGRESS)
        _progress->setWideCallback(callback);
#endif
    }
#endif
    
    DecoderImpl::DecoderImpl(const CMyComPtr<InStreamBase> & stream,
                             const plzma_file_type type,
                             const plzma_context context) : CMyUnknownImp(),
        _stream(stream),
#if !defined(LIBPLZMA_NO_PROGRESS)
        _progress(makeShared<Progress>(context)),
#endif
        _type(type) {
            plzma::initialize();
    }
    
    DecoderImpl::~DecoderImpl() {
#if !defined(LIBPLZMA_NO_CRYPTO)
        _password.clear(plzma_erase_zero);
#endif
        _stream->close();
    }
    
    SharedPtr<Decoder> makeSharedDecoder(const SharedPtr<InStream> & stream,
                                         const plzma_file_type type,
                                         const plzma_context context) {
        auto baseStream = stream.cast<InStreamBase>();
        if (baseStream) {
            return SharedPtr<Decoder>(new DecoderImpl(CMyComPtr<InStreamBase>(baseStream.get()), type, context));
        }
        throw Exception(plzma_error_code_invalid_arguments, "No input stream.", __FILE__, __LINE__);
    }
    
} // namespace plzma


#include "plzma_c_bindings_private.hpp"

#if !defined(LIBPLZMA_NO_C_BINDINGS)

using namespace plzma;

plzma_decoder plzma_decoder_create(plzma_in_stream * LIBPLZMA_NONNULL stream,
                                   const plzma_file_type type,
                                   const plzma_context context) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_decoder, stream)
    SharedPtr<InStream> inStream(static_cast<InStream *>(stream->object));
    auto baseStream = inStream.cast<InStreamBase>();
    if (!baseStream) {
        throw Exception(plzma_error_code_invalid_arguments, "No input stream.", __FILE__, __LINE__);
    }
    SharedPtr<DecoderImpl> decoderImpl(new DecoderImpl(CMyComPtr<InStreamBase>(baseStream.get()), type, context));
    createdCObject.object = static_cast<void *>(decoderImpl.take());
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

void plzma_decoder_set_progress_delegate_utf8_callback(plzma_decoder * LIBPLZMA_NONNULL decoder,
                                                       plzma_progress_delegate_utf8_callback LIBPLZMA_NULLABLE callback) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(decoder)
    static_cast<DecoderImpl *>(decoder->object)->setUtf8Callback(callback);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(decoder)
}

void plzma_decoder_set_progress_delegate_wide_callback(plzma_decoder * LIBPLZMA_NONNULL decoder,
                                                       plzma_progress_delegate_wide_callback LIBPLZMA_NULLABLE callback) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(decoder)
    static_cast<DecoderImpl *>(decoder->object)->setWideCallback(callback);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(decoder)
}

void plzma_decoder_set_password_wide_string(plzma_decoder * LIBPLZMA_NONNULL decoder, const wchar_t * LIBPLZMA_NULLABLE password) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(decoder)
    static_cast<DecoderImpl *>(decoder->object)->setPassword(password);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(decoder)
}

void plzma_decoder_set_password_utf8_string(plzma_decoder * LIBPLZMA_NONNULL decoder, const char * LIBPLZMA_NULLABLE password) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(decoder)
    static_cast<DecoderImpl *>(decoder->object)->setPassword(password);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(decoder)
}

bool plzma_decoder_open(plzma_decoder * LIBPLZMA_NONNULL decoder) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(decoder, false)
    return static_cast<DecoderImpl *>(decoder->object)->open();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(decoder, false)
}

void plzma_decoder_abort(plzma_decoder * LIBPLZMA_NONNULL decoder) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(decoder)
    static_cast<DecoderImpl *>(decoder->object)->abort();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(decoder)
}

plzma_size_t plzma_decoder_count(plzma_decoder * LIBPLZMA_NONNULL decoder) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(decoder, 0)
    return static_cast<DecoderImpl *>(decoder->object)->count();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(decoder, 0)
}

plzma_item_array plzma_decoder_items(plzma_decoder * LIBPLZMA_NONNULL decoder) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_item_array, decoder)
    auto items = static_cast<DecoderImpl *>(decoder->object)->items();
    createdCObject.object = static_cast<void *>(items.take());
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

plzma_item plzma_decoder_item_at(plzma_decoder * LIBPLZMA_NONNULL decoder, const plzma_size_t index) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_item, decoder)
    auto item = static_cast<DecoderImpl *>(decoder->object)->itemAt(index);
    createdCObject.object = static_cast<void *>(item.take());
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

bool plzma_decoder_extract_all_items_to_path(plzma_decoder * LIBPLZMA_NONNULL decoder,
                                             const plzma_path * LIBPLZMA_NONNULL path,
                                             const bool items_full_path) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN_WITH_ARG1(decoder, path, false)
    return static_cast<DecoderImpl *>(decoder->object)->extract(*static_cast<const Path *>(path->object), items_full_path);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(decoder, false)
}

bool plzma_decoder_extract_items_to_path(plzma_decoder * LIBPLZMA_NONNULL decoder,
                                         plzma_item_array * LIBPLZMA_NONNULL items,
                                         const plzma_path * LIBPLZMA_NONNULL path,
                                         const bool items_full_path) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN_WITH_ARG2(decoder, items, path, false)
    SharedPtr<ItemArray> itemsSPtr(static_cast<ItemArray *>(items->object));
    return static_cast<DecoderImpl *>(decoder->object)->extract(itemsSPtr,
                                                                *static_cast<const Path *>(path->object),
                                                                items_full_path);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(decoder, false)
}

bool plzma_decoder_extract_item_out_stream_array(plzma_decoder * LIBPLZMA_NONNULL decoder,
                                                 plzma_item_out_stream_array * LIBPLZMA_NONNULL items) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN_WITH_ARG1(decoder, items, false)
    SharedPtr<ItemOutStreamArray> itemsSPtr(static_cast<ItemOutStreamArray *>(items->object));
    return static_cast<DecoderImpl *>(decoder->object)->extract(itemsSPtr);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(decoder, false)
}

bool plzma_decoder_test_items(plzma_decoder * LIBPLZMA_NONNULL decoder,
                              plzma_item_array * LIBPLZMA_NONNULL items) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN_WITH_ARG1(decoder, items, false)
    SharedPtr<ItemArray> itemsSPtr(static_cast<ItemArray *>(items->object));
    return static_cast<DecoderImpl *>(decoder->object)->test(itemsSPtr);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(decoder, false)
}

bool plzma_decoder_test(plzma_decoder * LIBPLZMA_NONNULL decoder) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(decoder, false)
    return static_cast<DecoderImpl *>(decoder->object)->test();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(decoder, false)
}

void plzma_decoder_release(plzma_decoder * LIBPLZMA_NONNULL decoder) {
    plzma_object_exception_release(decoder);
    SharedPtr<DecoderImpl> decoderSPtr;
    decoderSPtr.assign(static_cast<DecoderImpl *>(decoder->object));
    decoder->object = nullptr;
}

#endif // !LIBPLZMA_NO_C_BINDINGS
