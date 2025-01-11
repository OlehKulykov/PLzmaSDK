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


#include <limits>
#include <memory>
#include <sstream>
#include <iostream> // std::cout
#include <type_traits>
#include <node.h>
#include <node_object_wrap.h>
#include <uv.h>
#include "../libplzma.hpp"
#include "../src/plzma_private.hpp"

using namespace v8;

namespace nplzma {
    std::string exceptionToString(const plzma::Exception & e) {
        std::ostringstream oss;
        oss << "code: " << e.code();
        const char * str;
        if ( (str = e.what()) ) { oss << "\nwhat: " << str; }
        if ( (str = e.reason()) ) { oss << "\nreason: " << str; }
        if ( (str = e.file()) ) { oss << "\nfile: " << str; }
        oss << "\nline: " << e.line();
        oss << "\nversion: " << plzma_version();
        return oss.str();
    }
    
    std::string exceptionToString(const std::exception & e) {
        std::ostringstream oss;
        oss << "code: 0";
        const char * str;
        if ( (str = e.what()) ) { oss << "\nwhat: " << str; }
        oss << "\nversion: " << plzma_version();
        return oss.str();
    }
}

#define NPLZMA_TRY \
try { \


#define NPLZMA_CATCH_RET(ISOLATE) \
} catch (const plzma::Exception & e) { \
    const auto estr = nplzma::exceptionToString(e); \
    ISOLATE->ThrowException(Exception::Error(String::NewFromUtf8(ISOLATE, estr.c_str()).ToLocalChecked())); return; \
} catch (const std::exception & e) { \
    const auto estr = nplzma::exceptionToString(e); \
    ISOLATE->ThrowException(Exception::Error(String::NewFromUtf8(ISOLATE, estr.c_str()).ToLocalChecked())); return; \
} catch (...) { \
    ISOLATE->ThrowException(Exception::Error(String::NewFromUtf8(ISOLATE, "Unknown exception.").ToLocalChecked())); return; \
} \


#define NPLZMA_CATCH_TO_STR(STR) \
} catch (const plzma::Exception & e) { \
    STR = nplzma::exceptionToString(e); \
} catch (const std::exception & e) { \
    STR = nplzma::exceptionToString(e); \
} catch (...) { \
    STR = "Unknown exception."; \
} \


#define NPLZMA_THROW_ARG_TYPE_ERROR_RET(ISOLATE, PROP) \
static const char * ecsutf8 = "Unsupported argument type or value for '" PROP "' property/method."; \
ISOLATE->ThrowException(Exception::TypeError(String::NewFromUtf8(ISOLATE, ecsutf8).ToLocalChecked())); \
return; \


#define NPLZMA_THROW_ARG1_TYPE_ERROR_RET(ISOLATE, FORMT, ARG1) \
static const char * ecsutf8Format = "Unsupported argument type or value for '" FORMT "' property/method."; \
char ecsutf8[128] = { 0 }; \
snprintf(ecsutf8, 128, ecsutf8Format, ARG1); \
ISOLATE->ThrowException(Exception::TypeError(String::NewFromUtf8(ISOLATE, ecsutf8).ToLocalChecked())); \
return; \


#define NPLZMA_GET_UINT64_FROM_VALUE(CONTEXT, VALUE, OUT_VALUE, OUT_VALUE_DEFINED) \
if (VALUE->IsBigInt()) { \
    OUT_VALUE = VALUE->ToBigInt(CONTEXT).ToLocalChecked()->Uint64Value(); \
    OUT_VALUE_DEFINED = true; \
} else if (VALUE->IsUint32()) { \
    OUT_VALUE = VALUE->ToUint32(CONTEXT).ToLocalChecked()->Value(); \
    OUT_VALUE_DEFINED = true; \
} else if (VALUE->IsNumber()) { \
    const auto valueFromNumber = VALUE->ToNumber(CONTEXT).ToLocalChecked()->Value(); \
    if (valueFromNumber >= 0 && valueFromNumber <= UINT64_MAX) { \
        OUT_VALUE = static_cast<uint64_t>(valueFromNumber); \
        OUT_VALUE_DEFINED = true; \
    } \
} \


#define NPLZMA_GET_UINT32_FROM_VALUE(CONTEXT, VALUE, OUT_VALUE, OUT_VALUE_DEFINED) \
if (VALUE->IsUint32()) { \
    OUT_VALUE = VALUE->ToUint32(CONTEXT).ToLocalChecked()->Value(); \
    OUT_VALUE_DEFINED = true; \
} else if (VALUE->IsBigInt()) { \
    const auto valueFromBigInt = VALUE->ToBigInt(CONTEXT).ToLocalChecked()->Uint64Value(); \
    if (valueFromBigInt <= UINT32_MAX) { \
        OUT_VALUE = static_cast<uint32_t>(valueFromBigInt); \
        OUT_VALUE_DEFINED = true; \
    } \
} else if (VALUE->IsNumber()) { \
    const auto valueFromNumber = VALUE->ToNumber(CONTEXT).ToLocalChecked()->Value(); \
    if (valueFromNumber >= 0 && valueFromNumber <= UINT32_MAX) { \
        OUT_VALUE = static_cast<uint32_t>(valueFromNumber); \
        OUT_VALUE_DEFINED = true; \
    } \
} \


#define NPLZMA_GET_PATH_FROM_VALUE(ISOLATE, CONTEXT, VALUE, OUT_VALUE, OUT_VALUE_DEFINED) \
if (VALUE->IsString()) { \
    String::Utf8Value tmpPathUtf8Str(ISOLATE, VALUE); \
    NPLZMA_TRY \
    OUT_VALUE.set(*tmpPathUtf8Str); \
    OUT_VALUE_DEFINED = true; \
    NPLZMA_CATCH_RET(ISOLATE) \
} else if (VALUE->IsObject()) { \
    Local<Object> tmpPathObject = VALUE->ToObject(CONTEXT).ToLocalChecked(); \
    Path * tmpPathPtr = Path::TypedUnwrap(tmpPathObject); \
    if (tmpPathPtr) { \
        NPLZMA_TRY \
        OUT_VALUE.set(tmpPathPtr->_path); \
        OUT_VALUE_DEFINED = true; \
        NPLZMA_CATCH_RET(ISOLATE) \
    } \
} \


namespace nplzma {
    
    class PathIterator;
    class Item;
    template<class T>
    class OutStream;
    class InStream;
    class Decoder;
    class Encoder;
    struct AsyncData;
    
    template <class T>
    class TypedObjectWrap : public node::ObjectWrap {
    protected:
        V8_INLINE void TypedWrap(Local<Object> object) {
            node::ObjectWrap::Wrap(object);
            assert(object->InternalFieldCount() > 1);
            object->SetAlignedPointerInInternalField(1, T::TypeKey());
        }
        
        static Persistent<FunctionTemplate> _constructor;
    public:
        TypedObjectWrap() : node::ObjectWrap() { }
        virtual ~TypedObjectWrap() { }
        
        static void * TypeKey() {
            Persistent<FunctionTemplate> * constructorPtr = static_cast<Persistent<FunctionTemplate> *>(&_constructor);
            return static_cast<void *>(constructorPtr);
        }
        
        V8_INLINE static T * TypedUnwrap(Local<Object> object) {
            if (object->InternalFieldCount() > 1) {
                void * typeKeyPtr = object->GetAlignedPointerFromInternalField(1);
                if (typeKeyPtr == T::TypeKey()) {
                    return node::ObjectWrap::Unwrap<T>(object);
                }
            }
            return nullptr;
        }
    };
    
    template <class T>
    Persistent<FunctionTemplate> TypedObjectWrap<T>::_constructor;
    
    class Path final : public TypedObjectWrap<Path> {
    private:
        friend class PathIterator;
        friend class Item;
        friend class OutStream<plzma::OutStream>;
        friend class OutStream<plzma::OutMultiStream>;
        friend class InStream;
        friend class Decoder;
        friend class Encoder;
        plzma::Path _path;
        
        static void Count(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void Exists(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void Readable(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void Writable(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void ReadableAndWritable(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void Stat(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void OpenDir(const FunctionCallbackInfo<Value> & args);
        static void ToString(const FunctionCallbackInfo<Value> & args);
        static void Clear(const FunctionCallbackInfo<Value> & args);
        static void Set(const FunctionCallbackInfo<Value> & args);
        static void Append(const FunctionCallbackInfo<Value> & args);
        static void Appending(const FunctionCallbackInfo<Value> & args);
        static void AppendRandomComponent(const FunctionCallbackInfo<Value> & args);
        static void AppendingRandomComponent(const FunctionCallbackInfo<Value> & args);
        static void LastComponent(const FunctionCallbackInfo<Value> & args);
        static void RemoveLastComponent(const FunctionCallbackInfo<Value> & args);
        static void RemovingLastComponent(const FunctionCallbackInfo<Value> & args);
        static void Remove(const FunctionCallbackInfo<Value> & args);
        static void CreateDir(const FunctionCallbackInfo<Value> & args);
        static void TmpPath(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void New(const FunctionCallbackInfo<Value> & args);
    public:
        Path(plzma::Path && path) : TypedObjectWrap<Path>(),
            _path(std::move(path)) { }
        virtual ~Path() { }
        
        static void Init(Local<Object> exports);
    };
    
    class PathIterator final : public node::ObjectWrap {
    private:
        friend class Path;
        plzma::SharedPtr<plzma::Path::Iterator> _iterator;
    public:
        PathIterator(plzma::SharedPtr<plzma::Path::Iterator> && iterator) : node::ObjectWrap(),
            _iterator(std::move(iterator)) { }
        virtual ~PathIterator() { }
        
        static void Next(const FunctionCallbackInfo<Value> & args);
        static void Close(const FunctionCallbackInfo<Value> & args);
        static void GetPath(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void Component(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void FullPath(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void IsDir(Local<String> property, const PropertyCallbackInfo<Value> & info);
    };
    
    class Item final : public TypedObjectWrap<Item> {
    private:
        friend class Decoder;
        plzma::SharedPtr<plzma::Item> _item;
        
        static void ToString(const FunctionCallbackInfo<Value> & args);
        static void GetPath(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void GetIndex(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void GetSize(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void SetSize(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info);
        static void GetPackSize(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void SetPackSize(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info);
        static void GetCrc32(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void SetCrc32(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info);
        static void GetCreationDate(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void SetCreationDate(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info);
        static void GetAccessDate(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void SetAccessDate(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info);
        static void GetModificationDate(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void SetModificationDate(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info);
        static void GetEncrypted(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void SetEncrypted(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info);
        static void GetIsDir(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void SetIsDir(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info);
        static void New(const FunctionCallbackInfo<Value> & args);
    public:
        Item(plzma::SharedPtr<plzma::Item> && item) : TypedObjectWrap<Item>(),
            _item(std::move(item)) { }
        virtual ~Item() { }
        
        static void Init(Local<Object> exports);
    };
    
    template<class T>
    class OutStream final : public TypedObjectWrap<OutStream<T> > {
    private:
        friend class Decoder;
        friend class Encoder;
        friend class OutStream<plzma::OutStream>;
        friend class OutStream<plzma::OutMultiStream>;
        plzma::SharedPtr<T> _stream;
        
        static Persistent<Symbol> _instantiateWithoutNativeStream;
        
        static void Erase(const FunctionCallbackInfo<Value> & args);
        static void CopyContent(const FunctionCallbackInfo<Value> & args);
        static void Opened(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void Streams(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void New(const FunctionCallbackInfo<Value> & args);
    public:
        OutStream(plzma::SharedPtr<T> && stream) : TypedObjectWrap<OutStream<T> >(),
            _stream(std::move(stream)) { }
        OutStream() : TypedObjectWrap<OutStream<T> >() { }
        virtual ~OutStream() { }
        
        static void Init(Local<Object> exports);
    };
    
    template <class T>
    Persistent<Symbol> OutStream<T>::_instantiateWithoutNativeStream;

    class InStream final : public TypedObjectWrap<InStream> {
    private:
        friend class Decoder;
        friend class Encoder;
        plzma::SharedPtr<plzma::InStream> _stream;
        std::shared_ptr<BackingStore> _backingStore;
        
        static void Erase(const FunctionCallbackInfo<Value> & args);
        static void Opened(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void New(const FunctionCallbackInfo<Value> & args);
    public:
        InStream(plzma::SharedPtr<plzma::InStream> && stream, std::shared_ptr<BackingStore> && backingStore) : TypedObjectWrap<InStream>(),
            _stream(std::move(stream)),
            _backingStore(std::move(backingStore)) { }
        virtual ~InStream() { }
        
        static void Init(Local<Object> exports);
    };
    
    class Encoder final : public node::ObjectWrap, public plzma::ProgressDelegate {
    private:
        friend struct AsyncData;
        Persistent<Function> _progressCallback;
        plzma::SharedPtr<plzma::Encoder> _encoder;
        AsyncData * _asyncData = nullptr;
        
        static void SetPassword(const FunctionCallbackInfo<Value> & args);
        static void SetProgressDelegate(const FunctionCallbackInfo<Value> & args);
        static void Add(const FunctionCallbackInfo<Value> & args);
        static void Open(const FunctionCallbackInfo<Value> & args);
        static void Abort(const FunctionCallbackInfo<Value> & args);
        static void Compress(const FunctionCallbackInfo<Value> & args);
        static void ShouldCreateSolidArchive(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void SetShouldCreateSolidArchive(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info);
        static void CompressionLevel(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void SetCompressionLevel(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info);
        static void ShouldCompressHeader(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void SetShouldCompressHeader(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info);
        static void ShouldCompressHeaderFull(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void SetShouldCompressHeaderFull(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info);
        static void ShouldEncryptContent(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void SetShouldEncryptContent(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info);
        static void ShouldEncryptHeader(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void SetShouldEncryptHeader(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info);
        static void ShouldStoreCreationDate(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void SetShouldStoreCreationDate(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info);
        static void ShouldStoreAccessDate(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void SetShouldStoreAccessDate(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info);
        static void ShouldStoreModificationDate(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void SetShouldStoreModificationDate(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info);
        static void New(const FunctionCallbackInfo<Value> & args);
    public:
        Encoder(plzma::SharedPtr<plzma::Encoder> && encoder) : node::ObjectWrap(),
            _encoder(std::move(encoder)) { }
        virtual ~Encoder() { }
        
        virtual void onProgress(void * LIBPLZMA_NULLABLE ctx, const plzma::String & path, const double progress);
        static void Init(Local<Object> exports);
    };
    
    class Decoder final : public node::ObjectWrap, public plzma::ProgressDelegate {
    private:
        friend struct AsyncData;
        Persistent<Function> _progressCallback;
        plzma::SharedPtr<plzma::Decoder> _decoder;
        AsyncData * _asyncData = nullptr;
        
        static void SetProgressDelegate(const FunctionCallbackInfo<Value> & args);
        static void SetPassword(const FunctionCallbackInfo<Value> & args);
        static void Open(const FunctionCallbackInfo<Value> & args);
        static void Abort(const FunctionCallbackInfo<Value> & args);
        static void ItemAt(const FunctionCallbackInfo<Value> & args);
        static void Extract(const FunctionCallbackInfo<Value> & args);
        static void Test(const FunctionCallbackInfo<Value> & args);
        static void Count(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void Items(Local<String> property, const PropertyCallbackInfo<Value> & info);
        static void New(const FunctionCallbackInfo<Value> & args);
    public:
        Decoder(plzma::SharedPtr<plzma::Decoder> && decoder) : node::ObjectWrap(),
            _decoder(std::move(decoder)) { }
        virtual ~Decoder() { }
        
        virtual void onProgress(void * LIBPLZMA_NULLABLE ctx, const plzma::String & path, const double progress);
        static void Init(Local<Object> exports);
    };
    
    struct AsyncData final {
    private:
        uv_work_t _request;
        uv_async_t _async;
        
        static void Close(uv_handle_t * handle);
        static void Work(uv_work_t * req);
        static void Progress(uv_async_t * handle);
        static void Complete(uv_work_t * req, int status);
    public:
        Persistent<Object> coderObject;
        Persistent<Promise::Resolver> resolver;
        Persistent<Function> progressCallback;
        plzma::SharedPtr<plzma::Decoder> decoder; // extracted from local coderObject
        plzma::SharedPtr<plzma::Encoder> encoder; // extracted from local coderObject
        plzma::SharedPtr<plzma::ItemArray> items;
        plzma::SharedPtr<plzma::ItemOutStreamArray> itemsOutStreams;
        plzma::Path path;
        std::string exceptionString;
        double progress = 0.0;
        uint32_t uint32Value = 0;
        bool boolValue = false;
        bool result = false;
        bool opening = false;
        bool testing = false;
        bool extracting = false;
        bool compressing = false;
        
        void close();
        void init();
        
        V8_INLINE void asyncSend() {
            if (uv_is_closing((uv_handle_t *)&_async) == 0) {
                uv_async_send(&_async);
            }
        }
    };
    
    void AsyncData::close() {
        if (uv_is_closing((uv_handle_t *)&_async) == 0) {
            uv_close((uv_handle_t *)&_async, AsyncData::Close);
        }
        coderObject.Reset();
        resolver.Reset();
        progressCallback.Reset();
        decoder.clear();
        encoder.clear();
        items.clear();
        itemsOutStreams.clear();
        path.clear(plzma_erase_zero);
        exceptionString.clear();
    }
    
    void AsyncData::init() {
        memset(&_request, 0, sizeof(uv_work_t));
        memset(&_async, 0, sizeof(uv_async_t));
        _request.data = _async.data = this;
        uv_async_init(uv_default_loop(), &_async, AsyncData::Progress);
        uv_queue_work(uv_default_loop(), &_request, AsyncData::Work, AsyncData::Complete);
    }
    
    void AsyncData::Close(uv_handle_t * handle) {
        AsyncData * data = static_cast<AsyncData *>(handle->data);
        delete data;
    }
    
    void AsyncData::Work(uv_work_t * req) {
        AsyncData * data = static_cast<AsyncData *>(req->data);
        NPLZMA_TRY
        if (data->opening) {
            data->result = data->decoder ? data->decoder->open() : data->encoder->open();
        } else if (data->extracting) {
            switch (data->uint32Value) {
                case 1:
                    data->result = data->decoder->extract(data->itemsOutStreams);
                    break;
                case 2:
                    data->result = data->decoder->extract(data->path, data->boolValue);
                    break;
                case 3:
                    data->result = data->decoder->extract(data->items, data->path, data->boolValue);
                    break;
                default:
                    data->result = false;
                    break;
            }
        } else if (data->compressing) {
            data->result = data->encoder->compress();
        } else if (data->testing) {
            data->result = data->items ? data->decoder->test(data->items) : data->decoder->test();
        }
        NPLZMA_CATCH_TO_STR(data->exceptionString)
    }
    
    void AsyncData::Progress(uv_async_t * handle) {
        AsyncData * data = static_cast<AsyncData *>(handle->data);
        if (!data->progressCallback.IsEmpty()) {
            Isolate * isolate = Isolate::GetCurrent();
            v8::HandleScope handleScope(isolate);
            Local<Context> context = isolate->GetCurrentContext();
            Local<Value> argv[2] = {
                String::NewFromUtf8(isolate, data->path.utf8()).ToLocalChecked(),
                Number::New(isolate, data->progress)
            };
            Local<Function> func = data->progressCallback.Get(isolate);
            auto unused = func->Call(context, context->Global(), 2, argv);
            unused.IsEmpty();
        }
    }
    
    void AsyncData::Complete(uv_work_t * req, int status) {
        Isolate * isolate = Isolate::GetCurrent();
        v8::HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        AsyncData * data = static_cast<AsyncData *>(req->data);
        Local<Object> coderObject = data->coderObject.Get(isolate);
        if (data->decoder) {
            Decoder * decoder = node::ObjectWrap::Unwrap<Decoder>(coderObject);
            if (data == decoder->_asyncData) {
                decoder->_asyncData = nullptr;
            }
        } else {
            Encoder * encoder = node::ObjectWrap::Unwrap<Encoder>(coderObject);
            if (data == encoder->_asyncData) {
                encoder->_asyncData = nullptr;
            }
        }
        if (!data->resolver.IsEmpty()) {
            Local<Promise::Resolver> resolver = data->resolver.Get(isolate);
            if (data->exceptionString.empty()) {
                resolver->Resolve(context, Boolean::New(isolate, data->result)).Check();
            } else {
                resolver->Reject(context, Exception::Error(String::NewFromUtf8(isolate, data->exceptionString.c_str()).ToLocalChecked())).Check();
            }
        }
        data->close();
    }
    
    void Encoder::onProgress(void * LIBPLZMA_NULLABLE ctx, const plzma::String & path, const double progress) {
        if (_asyncData) {
            if (!_asyncData->progressCallback.IsEmpty()) {
                _asyncData->path.set(path);
                _asyncData->progress = progress;
                _asyncData->asyncSend();
            }
        } else if (!_progressCallback.IsEmpty()) {
            Isolate * isolate = Isolate::GetCurrent();
            v8::HandleScope handleScope(isolate);
            Local<Context> context = isolate->GetCurrentContext();
            Local<Value> argv[2] = {
                String::NewFromUtf8(isolate, path.utf8()).ToLocalChecked(),
                Number::New(isolate, progress)
            };
            Local<Function> func = _progressCallback.Get(isolate);
            auto unused = func->Call(context, context->Global(), 2, argv);
            unused.IsEmpty();
        }
    }
    
    void Encoder::SetPassword(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Encoder * encoder = ObjectWrap::Unwrap<Encoder>(args.Holder());
        if (args.Length() > 0 && args[0]->IsString()) {
            String::Utf8Value str(isolate, args[0]);
            NPLZMA_TRY
            encoder->_encoder->setPassword(*str);
            NPLZMA_CATCH_RET(isolate)
        } else {
            NPLZMA_TRY
            encoder->_encoder->setPassword(static_cast<const char *>(nullptr));
            NPLZMA_CATCH_RET(isolate)
        }
    }
    
    void Encoder::SetProgressDelegate(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Encoder * encoder = ObjectWrap::Unwrap<Encoder>(args.Holder());
        if (args.Length() > 0 && args[0]->IsFunction()) {
            Local<Function> func = Local<Function>::Cast(args[0]);
            if (encoder->_asyncData) {
                encoder->_asyncData->progressCallback.Reset(isolate, func);
            }
            encoder->_progressCallback.Reset(isolate, func);
            NPLZMA_TRY
            encoder->_encoder->setProgressDelegate(encoder);
            NPLZMA_CATCH_RET(isolate)
        } else {
            if (encoder->_asyncData) {
                encoder->_asyncData->progressCallback.Reset();
            }
            encoder->_progressCallback.Reset();
            NPLZMA_TRY
            encoder->_encoder->setProgressDelegate(nullptr);
            NPLZMA_CATCH_RET(isolate)
        }
    }
    
    void Encoder::Add(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        Local<Object> encoderObject = args.Holder();
        Encoder * encoder = ObjectWrap::Unwrap<Encoder>(encoderObject);
        plzma::Path path, archivePath;
        plzma::SharedPtr<plzma::InStream> stream;
        plzma_open_dir_mode_t openDirMode = static_cast<plzma_open_dir_mode_t>(0);
        size_t method = 0;
        bool unsupportedArg1 = true, unsupportedArg2 = false, unsupportedArg3 = false;
        if (args.Length() > 0) {
            if (args[0]->IsString()) {
                String::Utf8Value pathStr(isolate, args[0]);
                NPLZMA_TRY
                path.set(*pathStr);
                unsupportedArg1 = false;
                method = 1;
                NPLZMA_CATCH_RET(isolate)
            } else if (args[0]->IsObject()) {
                Local<Object> obj = args[0]->ToObject(context).ToLocalChecked();
                Path * pathPtr = Path::TypedUnwrap(obj);
                if (pathPtr) {
                    NPLZMA_TRY
                    path.set(pathPtr->_path);
                    unsupportedArg1 = false;
                    method = 1;
                    NPLZMA_CATCH_RET(isolate)
                } else {
                    InStream * inStream = InStream::TypedUnwrap(obj);
                    if (inStream) {
                        NPLZMA_TRY
                        stream = inStream->_stream;
                        unsupportedArg1 = false;
                        method = 2;
                        NPLZMA_CATCH_RET(isolate)
                    }
                }
            }
        }
        switch (method) {
            case 1: {
                if (args.Length() > 1) {
                    uint32_t mode = 0;
                    unsupportedArg2 = true;
                    if (args[1]->IsArray()) {
                        Local<Array> arr = Local<Array>::Cast(args[1]);
                        for (uint32_t i = 0, n = arr->Length(); i < n; i++) {
                            Local<Value> obj = arr->Get(context, i).ToLocalChecked();
                            uint32_t modeValue = 0;
                            bool modeValueDefined = false;
                            NPLZMA_GET_UINT32_FROM_VALUE(context, obj, modeValue, modeValueDefined)
                            if (modeValueDefined) {
                                mode |= modeValue;
                            } else {
                                NPLZMA_THROW_ARG1_TYPE_ERROR_RET(isolate, "add(?,mode[%llu?],...)", static_cast<unsigned long long>(i))
                            }
                        }
                        unsupportedArg2 = false;
                    } else {
                        uint32_t modeValue = 0;
                        bool modeValueDefined = false;
                        NPLZMA_GET_UINT32_FROM_VALUE(context, args[1], modeValue, modeValueDefined)
                        if (modeValueDefined) {
                            mode |= modeValue;
                            unsupportedArg2 = false;
                        }
                    }
                    if (!unsupportedArg2) {
                        openDirMode = static_cast<plzma_open_dir_mode_t>(mode);
                    }
                }
                if (args.Length() > 2) {
                    unsupportedArg3 = true;
                    bool valueDefined = false;
                    NPLZMA_GET_PATH_FROM_VALUE(isolate, context, args[2], archivePath, valueDefined)
                    unsupportedArg3 = !valueDefined;
                }
            }
                break;
            case 2: {
                if (args.Length() > 1) {
                    unsupportedArg2 = true;
                    bool valueDefined = false;
                    NPLZMA_GET_PATH_FROM_VALUE(isolate, context, args[1], archivePath, valueDefined)
                    unsupportedArg2 = !valueDefined;
                }
            }
                break;
            default:
                break;
        }
        if (unsupportedArg1) {
            NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "add(?,...,...)")
        }
        if (unsupportedArg2) {
            NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "add(...,?,...)")
        }
        if (unsupportedArg3) {
            NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "add(...,...,?)")
        }
        switch (method) {
            case 1:
                NPLZMA_TRY
                encoder->_encoder->add(path, openDirMode, archivePath);
                NPLZMA_CATCH_RET(isolate)
                break;
            case 2:
                NPLZMA_TRY
                encoder->_encoder->add(stream, archivePath);
                NPLZMA_CATCH_RET(isolate)
                break;
            default:
                NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "add(?)")
                break;
        }
    }
    
    void Encoder::Open(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        Local<Object> encoderObject = args.Holder();
        Encoder * encoder = ObjectWrap::Unwrap<Encoder>(encoderObject);
        if (encoder->_asyncData) {
            isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Previous asynchronous operation is not completed.").ToLocalChecked()));
            return;
        }
        if (args.Data()->BooleanValue(isolate)) { // async
            Local<Promise::Resolver> resolver = Promise::Resolver::New(context).ToLocalChecked();
            AsyncData * data = nullptr;
            NPLZMA_TRY
            data = new AsyncData();
            data->encoder = encoder->_encoder;
            NPLZMA_CATCH_RET(isolate)
            data->coderObject.Reset(isolate, encoderObject);
            data->resolver.Reset(isolate, resolver);
            data->opening = true;
            if (!encoder->_progressCallback.IsEmpty()) {
                data->progressCallback.Reset(isolate, encoder->_progressCallback);
            }
            encoder->_asyncData = data;
            data->init();
            args.GetReturnValue().Set(resolver->GetPromise());
        } else {
            bool opened = true;
            NPLZMA_TRY
            opened = encoder->_encoder->open();
            NPLZMA_CATCH_RET(isolate)
            args.GetReturnValue().Set(Boolean::New(isolate, opened));
        }
    }
    
    void Encoder::Abort(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Encoder * encoder = ObjectWrap::Unwrap<Encoder>(args.Holder());
        NPLZMA_TRY
        encoder->_encoder->abort();
        NPLZMA_CATCH_RET(isolate)
    }
    
    void Encoder::Compress(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        Local<Object> encoderObject = args.Holder();
        Encoder * encoder = ObjectWrap::Unwrap<Encoder>(encoderObject);
        if (encoder->_asyncData) {
            isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Previous asynchronous operation is not completed.").ToLocalChecked()));
            return;
        }
        if (args.Data()->BooleanValue(isolate)) { // async
            Local<Promise::Resolver> resolver = Promise::Resolver::New(context).ToLocalChecked();
            AsyncData * data = nullptr;
            NPLZMA_TRY
            data = new AsyncData();
            data->encoder = encoder->_encoder;
            NPLZMA_CATCH_RET(isolate)
            data->coderObject.Reset(isolate, encoderObject);
            data->resolver.Reset(isolate, resolver);
            data->compressing = true;
            if (!encoder->_progressCallback.IsEmpty()) {
                data->progressCallback.Reset(isolate, encoder->_progressCallback);
            }
            encoder->_asyncData = data;
            data->init();
            args.GetReturnValue().Set(resolver->GetPromise());
        } else {
            bool opened = true;
            NPLZMA_TRY
            opened = encoder->_encoder->compress();
            NPLZMA_CATCH_RET(isolate)
            args.GetReturnValue().Set(Boolean::New(isolate, opened));
        }
    }
    
    void Encoder::ShouldCreateSolidArchive(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Encoder * encoder = ObjectWrap::Unwrap<Encoder>(info.Holder());
        info.GetReturnValue().Set(Boolean::New(isolate, encoder->_encoder->shouldCreateSolidArchive()));
    }
    
    void Encoder::SetShouldCreateSolidArchive(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Encoder * encoder = ObjectWrap::Unwrap<Encoder>(info.Holder());
        encoder->_encoder->setShouldCreateSolidArchive(value->BooleanValue(isolate));
    }
    
    void Encoder::CompressionLevel(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Encoder * encoder = ObjectWrap::Unwrap<Encoder>(info.Holder());
        info.GetReturnValue().Set(Uint32::New(isolate, encoder->_encoder->compressionLevel()));
    }
    
    void Encoder::SetCompressionLevel(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Encoder * encoder = ObjectWrap::Unwrap<Encoder>(info.Holder());
        Local<Context> context = isolate->GetCurrentContext();
        uint32_t compressionLevelValue = 0;
        bool compressionLevelValueDefined = false;
        NPLZMA_GET_UINT32_FROM_VALUE(context, value, compressionLevelValue, compressionLevelValueDefined)
        if (compressionLevelValueDefined) {
            encoder->_encoder->setCompressionLevel(static_cast<uint8_t>(compressionLevelValue));
        } else {
            NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "compressionLevel")
        }
    }
    
    void Encoder::ShouldCompressHeader(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Encoder * encoder = ObjectWrap::Unwrap<Encoder>(info.Holder());
        info.GetReturnValue().Set(Boolean::New(isolate, encoder->_encoder->shouldCompressHeader()));
    }
    
    void Encoder::SetShouldCompressHeader(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Encoder * encoder = ObjectWrap::Unwrap<Encoder>(info.Holder());
        encoder->_encoder->setShouldCompressHeader(value->BooleanValue(isolate));
    }
    
    void Encoder::ShouldCompressHeaderFull(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Encoder * encoder = ObjectWrap::Unwrap<Encoder>(info.Holder());
        info.GetReturnValue().Set(Boolean::New(isolate, encoder->_encoder->shouldCompressHeaderFull()));
    }
    
    void Encoder::SetShouldCompressHeaderFull(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Encoder * encoder = ObjectWrap::Unwrap<Encoder>(info.Holder());
        encoder->_encoder->setShouldCompressHeaderFull(value->BooleanValue(isolate));
    }
    
    void Encoder::ShouldEncryptContent(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Encoder * encoder = ObjectWrap::Unwrap<Encoder>(info.Holder());
        info.GetReturnValue().Set(Boolean::New(isolate, encoder->_encoder->shouldEncryptContent()));
    }
    
    void Encoder::SetShouldEncryptContent(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Encoder * encoder = ObjectWrap::Unwrap<Encoder>(info.Holder());
        encoder->_encoder->setShouldEncryptContent(value->BooleanValue(isolate));
    }
    
    void Encoder::ShouldEncryptHeader(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Encoder * encoder = ObjectWrap::Unwrap<Encoder>(info.Holder());
        info.GetReturnValue().Set(Boolean::New(isolate, encoder->_encoder->shouldEncryptHeader()));
    }
    
    void Encoder::SetShouldEncryptHeader(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Encoder * encoder = ObjectWrap::Unwrap<Encoder>(info.Holder());
        encoder->_encoder->setShouldEncryptHeader(value->BooleanValue(isolate));
    }
    
    void Encoder::ShouldStoreCreationDate(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Encoder * encoder = ObjectWrap::Unwrap<Encoder>(info.Holder());
        info.GetReturnValue().Set(Boolean::New(isolate, encoder->_encoder->shouldStoreCreationTime()));
    }
    
    void Encoder::SetShouldStoreCreationDate(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Encoder * encoder = ObjectWrap::Unwrap<Encoder>(info.Holder());
        encoder->_encoder->setShouldStoreCreationTime(value->BooleanValue(isolate));
    }
    
    void Encoder::ShouldStoreAccessDate(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Encoder * encoder = ObjectWrap::Unwrap<Encoder>(info.Holder());
        info.GetReturnValue().Set(Boolean::New(isolate, encoder->_encoder->shouldStoreAccessTime()));
    }
    
    void Encoder::SetShouldStoreAccessDate(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Encoder * encoder = ObjectWrap::Unwrap<Encoder>(info.Holder());
        encoder->_encoder->setShouldStoreAccessTime(value->BooleanValue(isolate));
    }
    
    void Encoder::ShouldStoreModificationDate(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Encoder * encoder = ObjectWrap::Unwrap<Encoder>(info.Holder());
        info.GetReturnValue().Set(Boolean::New(isolate, encoder->_encoder->shouldStoreModificationTime()));
    }
    
    void Encoder::SetShouldStoreModificationDate(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Encoder * encoder = ObjectWrap::Unwrap<Encoder>(info.Holder());
        encoder->_encoder->setShouldStoreModificationTime(value->BooleanValue(isolate));
    }
    
    void Encoder::Init(Local<Object> exports) {
        Isolate * isolate = exports->GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        Local<ObjectTemplate> dataTpl = ObjectTemplate::New(isolate);
        dataTpl->SetInternalFieldCount(1);
        Local<Object> dataObject = dataTpl->NewInstance(context).ToLocalChecked();
        
        // Constructor template: 'Encoder' func tpl
        Local<FunctionTemplate> ctorTpl = FunctionTemplate::New(isolate, Encoder::New, dataObject);
        ctorTpl->SetClassName(String::NewFromUtf8(isolate, "Encoder").ToLocalChecked());
        
        Local<ObjectTemplate> ctorInstTpl = ctorTpl->InstanceTemplate();
        ctorInstTpl->SetInternalFieldCount(1);
        
        // (new Encoder(...)).<func>(...)
        Local<ObjectTemplate> ctorProtoTpl = ctorTpl->PrototypeTemplate();
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "setProgressDelegate").ToLocalChecked(), FunctionTemplate::New(isolate, Encoder::SetProgressDelegate), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "setPassword").ToLocalChecked(), FunctionTemplate::New(isolate, Encoder::SetPassword), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "add").ToLocalChecked(), FunctionTemplate::New(isolate, Encoder::Add), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "open").ToLocalChecked(), FunctionTemplate::New(isolate, Encoder::Open, Boolean::New(isolate, false)), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "openAsync").ToLocalChecked(), FunctionTemplate::New(isolate, Encoder::Open, Boolean::New(isolate, true)), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "abort").ToLocalChecked(), FunctionTemplate::New(isolate, Encoder::Abort), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "compress").ToLocalChecked(), FunctionTemplate::New(isolate, Encoder::Compress, Boolean::New(isolate, false)), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "compressAsync").ToLocalChecked(), FunctionTemplate::New(isolate, Encoder::Compress, Boolean::New(isolate, true)), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        
        // (new Encoder(...)).<prop>
        ctorInstTpl->SetAccessor(String::NewFromUtf8(isolate, "shouldCreateSolidArchive").ToLocalChecked(), Encoder::ShouldCreateSolidArchive, Encoder::SetShouldCreateSolidArchive, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(DontDelete | DontEnum));
        ctorInstTpl->SetAccessor(String::NewFromUtf8(isolate, "compressionLevel").ToLocalChecked(), Encoder::CompressionLevel, Encoder::SetCompressionLevel, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(DontDelete | DontEnum));
        ctorInstTpl->SetAccessor(String::NewFromUtf8(isolate, "shouldCompressHeader").ToLocalChecked(), Encoder::ShouldCompressHeader, Encoder::SetShouldCompressHeader, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(DontDelete | DontEnum));
        ctorInstTpl->SetAccessor(String::NewFromUtf8(isolate, "shouldCompressHeaderFull").ToLocalChecked(), Encoder::ShouldCompressHeaderFull, Encoder::SetShouldCompressHeaderFull, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(DontDelete | DontEnum));
        ctorInstTpl->SetAccessor(String::NewFromUtf8(isolate, "shouldEncryptContent").ToLocalChecked(), Encoder::ShouldEncryptContent, Encoder::SetShouldEncryptContent, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(DontDelete | DontEnum));
        ctorInstTpl->SetAccessor(String::NewFromUtf8(isolate, "shouldEncryptHeader").ToLocalChecked(), Encoder::ShouldEncryptHeader, Encoder::SetShouldEncryptHeader, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(DontDelete | DontEnum));
        ctorInstTpl->SetAccessor(String::NewFromUtf8(isolate, "shouldStoreCreationDate").ToLocalChecked(), Encoder::ShouldStoreCreationDate, Encoder::SetShouldStoreCreationDate, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(DontDelete | DontEnum));
        ctorInstTpl->SetAccessor(String::NewFromUtf8(isolate, "shouldStoreAccessDate").ToLocalChecked(), Encoder::ShouldStoreAccessDate, Encoder::SetShouldStoreAccessDate, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(DontDelete | DontEnum));
        ctorInstTpl->SetAccessor(String::NewFromUtf8(isolate, "shouldStoreModificationDate").ToLocalChecked(), Encoder::ShouldStoreModificationDate, Encoder::SetShouldStoreModificationDate, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(DontDelete | DontEnum));
        
        Local<Function> constructor = ctorTpl->GetFunction(context).ToLocalChecked();
        dataObject->SetInternalField(0, constructor);
        exports->Set(context, String::NewFromUtf8(isolate, "Encoder").ToLocalChecked(), constructor).FromJust();
    }
    
    void Encoder::New(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        if (args.IsConstructCall()) {
            plzma::SharedPtr<plzma::OutStream> stream;
            plzma::SharedPtr<plzma::OutMultiStream> multiStream;
            plzma_file_type fileType = static_cast<plzma_file_type>(0);
            plzma_method method = static_cast<plzma_method>(0);
            bool unsupportedArg1 = true, unsupportedArg2 = true, unsupportedArg3 = true;
            if (args.Length() > 2) {
                if (args[0]->IsObject()) {
                    Local<Object> inObj = args[0]->ToObject(context).ToLocalChecked();
                    OutStream<plzma::OutStream> * outStream = OutStream<plzma::OutStream>::TypedUnwrap(inObj);
                    if (outStream) {
                        NPLZMA_TRY
                        stream = outStream->_stream;
                        unsupportedArg1 = false;
                        NPLZMA_CATCH_RET(isolate)
                    } else {
                        OutStream<plzma::OutMultiStream> * outMultiStream = OutStream<plzma::OutMultiStream>::TypedUnwrap(inObj);
                        if (outMultiStream) {
                            NPLZMA_TRY
                            multiStream = outMultiStream->_stream;
                            unsupportedArg1 = false;
                            NPLZMA_CATCH_RET(isolate)
                        }
                    }
                }
                uint32_t fileTypeValue = 0, methodValue = 0;
                bool fileTypeValueDefined = false, methodValueDefined = false;
                NPLZMA_GET_UINT32_FROM_VALUE(context, args[1], fileTypeValue, fileTypeValueDefined)
                NPLZMA_GET_UINT32_FROM_VALUE(context, args[2], methodValue, methodValueDefined)
                if (fileTypeValueDefined) {
                    fileType = static_cast<plzma_file_type>(fileTypeValue);
                    unsupportedArg2 = false;
                }
                if (methodValueDefined) {
                    method = static_cast<plzma_method>(methodValue);
                    unsupportedArg3 = false;
                }
            }
            if (unsupportedArg1) {
                NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "Encoder(?,...,...)")
            }
            if (unsupportedArg2) {
                NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "Encoder(...,?,...)")
            }
            if (unsupportedArg3) {
                NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "Encoder(...,...,?)")
            }
            Encoder * obj = nullptr;
            NPLZMA_TRY
            auto encoder = stream ? plzma::makeSharedEncoder(stream, fileType, method) : plzma::makeSharedEncoder(multiStream, fileType, method);
            obj = new Encoder(std::move(encoder));
            NPLZMA_CATCH_RET(isolate)
            obj->Wrap(args.This());
            args.GetReturnValue().Set(args.This());
        } else {
            Local<Function> constructor = args.Data().As<Object>()->GetInternalField(0).As<Function>();
            int argc = 0;
            Local<Value> argv[3];
            for (int i = 0, n = args.Length(); i < n && i < 3; i++) {
                argv[argc++] = args[i];
            }
            TryCatch trycatch(isolate);
            MaybeLocal<Object> maybeResult = constructor->NewInstance(context, argc, argv);
            if (maybeResult.IsEmpty()) {
                if (trycatch.HasCaught()) {
                    trycatch.ReThrow();
                } else {
                    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Can't instantiate Encoder.").ToLocalChecked()));
                }
                return;
            }
            args.GetReturnValue().Set(maybeResult.ToLocalChecked());
        }
    }
    
    void Decoder::onProgress(void * LIBPLZMA_NULLABLE ctx, const plzma::String & path, const double progress) {
        if (_asyncData) {
            if (!_asyncData->progressCallback.IsEmpty()) {
                _asyncData->path.set(path);
                _asyncData->progress = progress;
                _asyncData->asyncSend();
            }
        } else if (!_progressCallback.IsEmpty()) {
            Isolate * isolate = Isolate::GetCurrent();
            v8::HandleScope handleScope(isolate);
            Local<Context> context = isolate->GetCurrentContext();
            Local<Value> argv[2] = {
                String::NewFromUtf8(isolate, path.utf8()).ToLocalChecked(),
                Number::New(isolate, progress)
            };
            Local<Function> func = _progressCallback.Get(isolate);
            auto unused = func->Call(context, context->Global(), 2, argv);
            unused.IsEmpty();
        }
    }
        
    void Decoder::SetProgressDelegate(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Decoder * decoder = ObjectWrap::Unwrap<Decoder>(args.Holder());
        if (args.Length() > 0 && args[0]->IsFunction()) {
            Local<Function> func = Local<Function>::Cast(args[0]);
            if (decoder->_asyncData) {
                decoder->_asyncData->progressCallback.Reset(isolate, func);
            }
            decoder->_progressCallback.Reset(isolate, func);
            NPLZMA_TRY
            decoder->_decoder->setProgressDelegate(decoder);
            NPLZMA_CATCH_RET(isolate)
        } else {
            if (decoder->_asyncData) {
                decoder->_asyncData->progressCallback.Reset();
            }
            decoder->_progressCallback.Reset();
            NPLZMA_TRY
            decoder->_decoder->setProgressDelegate(nullptr);
            NPLZMA_CATCH_RET(isolate)
        }
    }
    
    void Decoder::SetPassword(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Decoder * decoder = ObjectWrap::Unwrap<Decoder>(args.Holder());
        if (args.Length() > 0 && args[0]->IsString()) {
            String::Utf8Value str(isolate, args[0]);
            NPLZMA_TRY
            decoder->_decoder->setPassword(*str);
            NPLZMA_CATCH_RET(isolate)
        } else {
            NPLZMA_TRY
            decoder->_decoder->setPassword(static_cast<const char *>(nullptr));
            NPLZMA_CATCH_RET(isolate)
        }
    }
    
    void Decoder::Open(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        Local<Object> decoderObject = args.Holder();
        Decoder * decoder = ObjectWrap::Unwrap<Decoder>(decoderObject);
        if (decoder->_asyncData) {
            isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Previous asynchronous operation is not completed.").ToLocalChecked()));
            return;
        }
        if (args.Data()->BooleanValue(isolate)) { // async
            Local<Promise::Resolver> resolver = Promise::Resolver::New(context).ToLocalChecked();
            AsyncData * data = nullptr;
            NPLZMA_TRY
            data = new AsyncData();
            data->decoder = decoder->_decoder;
            NPLZMA_CATCH_RET(isolate)
            data->coderObject.Reset(isolate, decoderObject);
            data->resolver.Reset(isolate, resolver);
            data->opening = true;
            if (!decoder->_progressCallback.IsEmpty()) {
                data->progressCallback.Reset(isolate, decoder->_progressCallback);
            }
            decoder->_asyncData = data;
            data->init();
            args.GetReturnValue().Set(resolver->GetPromise());
        } else {
            bool opened = true;
            NPLZMA_TRY
            opened = decoder->_decoder->open();
            NPLZMA_CATCH_RET(isolate)
            args.GetReturnValue().Set(Boolean::New(isolate, opened));
        }
    }
    
    void Decoder::Abort(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Decoder * decoder = ObjectWrap::Unwrap<Decoder>(args.Holder());
        NPLZMA_TRY
        decoder->_decoder->abort();
        NPLZMA_CATCH_RET(isolate)
    }
    
    void Decoder::ItemAt(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        Decoder * decoder = ObjectWrap::Unwrap<Decoder>(args.Holder());
        plzma_size_t index = 0;
        bool unsupportedArg1 = true;
        if (args.Length() > 0) {
            uint32_t indexValue = 0;
            bool indexValueDefined = false;
            NPLZMA_GET_UINT32_FROM_VALUE(context, args[0], indexValue, indexValueDefined)
            if (indexValueDefined) {
                index = indexValue;
                unsupportedArg1 = false;
            }
        }
        if (unsupportedArg1) {
            NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "itemAt(?)")
        }
        plzma::SharedPtr<plzma::Item> itemSPtr;
        NPLZMA_TRY
        if (index < decoder->_decoder->count()) {
            itemSPtr = decoder->_decoder->itemAt(index);
        }
        NPLZMA_CATCH_RET(isolate)
        if (itemSPtr) {
            Local<Function> constructor = Item::_constructor.Get(isolate)->GetFunction(context).ToLocalChecked();
            Local<Value> argv[1];
            TryCatch trycatch(isolate);
            MaybeLocal<Object> maybeItemObject = constructor->NewInstance(context, 0, argv);
            if (maybeItemObject.IsEmpty()) {
                if (trycatch.HasCaught()) {
                    trycatch.ReThrow();
                } else {
                    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Can't instantiate Item.").ToLocalChecked()));
                }
                return;
            }
            Local<Object> itemObject = maybeItemObject.ToLocalChecked();
            Item * item = Item::TypedUnwrap(itemObject);
            item->_item = std::move(itemSPtr);
            args.GetReturnValue().Set(itemObject);
        } else {
            isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Can't retrieve item at index. Decoder not opened or index out of bounds or internal error.").ToLocalChecked()));
        }
    }
    
    void Decoder::Extract(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        Local<Object> decoderObject = args.Holder();
        Decoder * decoder = ObjectWrap::Unwrap<Decoder>(decoderObject);
        if (decoder->_asyncData) {
            isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Previous asynchronous operation is not completed.").ToLocalChecked()));
            return;
        }
        plzma::Path path;
        plzma::SharedPtr<plzma::ItemArray> itemsArray;
        plzma::SharedPtr<plzma::ItemOutStreamArray> itemsMap;
        bool usingItemsFullPath = true;
        uint32_t method = 0;
        if (args.Length() > 0) {
            if (args[0]->IsMap()) {
                // 1. extract(const SharedPtr<ItemOutStreamArray> & items)
                Local<Map> mapObject = Local<Map>::Cast(args[0]);
                if (mapObject->Size() > (UINT32_MAX / 2)) {
                    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "The number of map items is more than supported(uint32.max / 2)").ToLocalChecked()));
                    return;
                }
                Local<Array> mapArray = mapObject->AsArray();
                NPLZMA_TRY
                itemsMap = plzma::makeShared<plzma::ItemOutStreamArray>(static_cast<plzma_size_t>(mapObject->Size()));
                NPLZMA_CATCH_RET(isolate)
                for (uint32_t i = 0, n = mapArray->Length(); i < n; i += 2) {
                    bool invalidArg1 = true, invalidArg2 = true;
                    Local<Value> keyValue = mapArray->Get(context, i).ToLocalChecked();
                    if (keyValue->IsObject()) {
                        Local<Value> valueValue = mapArray->Get(context, i + 1).ToLocalChecked();
                        if (valueValue->IsObject()) {
                            Local<Object> keyObject = keyValue->ToObject(context).ToLocalChecked();
                            Item * item = Item::TypedUnwrap(keyObject);
                            if (item) {
                                invalidArg1 = false;
                                Local<Object> valueObject = valueValue->ToObject(context).ToLocalChecked();
                                OutStream<plzma::OutStream> * stream = OutStream<plzma::OutStream>::TypedUnwrap(valueObject);
                                if (stream) {
                                    invalidArg2 = false;
                                    NPLZMA_TRY
                                    itemsMap->push(plzma::ItemOutStreamArray::ElementType(item->_item, stream->_stream));
                                    NPLZMA_CATCH_RET(isolate)
                                } else {
                                    OutStream<plzma::OutMultiStream> * multiStream = OutStream<plzma::OutMultiStream>::TypedUnwrap(valueObject);
                                    if (multiStream) {
                                        invalidArg2 = false;
                                        NPLZMA_TRY
                                        itemsMap->push(plzma::ItemOutStreamArray::ElementType(item->_item, multiStream->_stream.cast<plzma::OutStream>()));
                                        NPLZMA_CATCH_RET(isolate)
                                    }
                                }
                            }
                        }
                    }
                    if (invalidArg1) {
                        NPLZMA_THROW_ARG1_TYPE_ERROR_RET(isolate, "extract(map<Item,OutStream>[%llu?,...])", static_cast<unsigned long long>(i / 2))
                    }
                    if (invalidArg2) {
                        NPLZMA_THROW_ARG1_TYPE_ERROR_RET(isolate, "extract(map<Item,OutStream>[...,%llu?])", static_cast<unsigned long long>(i / 2))
                    }
                }
                method = 1;
            } else if (args[0]->IsString()) {
                // 2. extract(const Path & path, const bool usingItemsFullPath = true)
                Local<String> pathObj = args[0]->ToString(context).ToLocalChecked();
                String::Utf8Value pathStr(isolate, pathObj);
                NPLZMA_TRY
                path.set(*pathStr);
                NPLZMA_CATCH_RET(isolate)
                if (args.Length() > 1) {
                    usingItemsFullPath = args[1]->BooleanValue(isolate);
                }
                method = 2;
            } else if (args[0]->IsArray()) {
                // 3. extract(const SharedPtr<ItemArray> & items, const Path & path, const bool usingItemsFullPath = true)
                if (args.Length() < 2) {
                    NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "extract(items<Item>,?,?)")
                }
                Local<Array> arr = Local<Array>::Cast(args[1]);
                NPLZMA_TRY
                itemsArray = plzma::makeShared<plzma::ItemArray>(static_cast<plzma_size_t>(arr->Length()));
                NPLZMA_CATCH_RET(isolate)
                for (uint32_t i = 0, n = arr->Length(); i < n; i++) {
                    Local<Value> itemValue = arr->Get(context, i).ToLocalChecked();
                    bool invalidArg1 = true;
                    if (itemValue->IsObject()) {
                        Local<Object> itemObj = itemValue->ToObject(context).ToLocalChecked();
                        Item * item = Item::TypedUnwrap(itemObj);
                        if (item) {
                            NPLZMA_TRY
                            itemsArray->push(item->_item);
                            invalidArg1 = false;
                            NPLZMA_CATCH_RET(isolate)
                        }
                    }
                    if (invalidArg1) {
                        NPLZMA_THROW_ARG1_TYPE_ERROR_RET(isolate, "extract(items<Item>[%llu?],...,...)", static_cast<unsigned long long>(i / 2))
                    }
                }
                bool valueDefined = false;
                NPLZMA_GET_PATH_FROM_VALUE(isolate, context, args[1], path, valueDefined)
                if (!valueDefined) {
                    NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "extract(items<Item>,?,...)")
                }
                if (args.Length() > 2) {
                    usingItemsFullPath = args[2]->BooleanValue(isolate);
                }
                method = 3;
            } else if (args[0]->IsObject()) {
                // 2. extract(const Path & path, const bool usingItemsFullPath = true)
                Local<Object> pathObj = args[0]->ToObject(context).ToLocalChecked();
                Path * inPath = Path::TypedUnwrap(pathObj);
                bool invalidArg1 = true;
                if (inPath) {
                    NPLZMA_TRY
                    path.set(inPath->_path);
                    invalidArg1 = false;
                    NPLZMA_CATCH_RET(isolate)
                }
                if (invalidArg1) {
                    NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "extract(?,...)")
                }
                if (args.Length() > 1) {
                    usingItemsFullPath = args[1]->BooleanValue(isolate);
                }
                method = 2;
            }
        }
        if (method == 0) {
            NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "extract(?)")
        }
        if (args.Data()->BooleanValue(isolate)) { // async
            Local<Promise::Resolver> resolver = Promise::Resolver::New(context).ToLocalChecked();
            AsyncData * data = nullptr;
            NPLZMA_TRY
            data = new AsyncData();
            data->items = std::move(itemsArray);
            data->itemsOutStreams = std::move(itemsMap);
            data->path = std::move(path);
            data->decoder = decoder->_decoder;
            data->uint32Value = method;
            data->boolValue = usingItemsFullPath;
            NPLZMA_CATCH_RET(isolate)
            data->coderObject.Reset(isolate, decoderObject);
            data->resolver.Reset(isolate, resolver);
            data->extracting = true;
            if (!decoder->_progressCallback.IsEmpty()) {
                data->progressCallback.Reset(isolate, decoder->_progressCallback);
            }
            decoder->_asyncData = data;
            data->init();
            args.GetReturnValue().Set(resolver->GetPromise());
        } else {
            bool extracted = false;
            NPLZMA_TRY
            switch (method) {
                case 1:
                    extracted = decoder->_decoder->extract(itemsMap);
                    break;
                case 2:
                    extracted = decoder->_decoder->extract(path, usingItemsFullPath);
                    break;
                case 3:
                    extracted = decoder->_decoder->extract(itemsArray, path, usingItemsFullPath);
                    break;
                default:
                    break;
            }
            NPLZMA_CATCH_RET(isolate)
            args.GetReturnValue().Set(Boolean::New(isolate, extracted));
        }
    }
        
    void Decoder::Test(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        Local<Object> decoderObject = args.Holder();
        Decoder * decoder = ObjectWrap::Unwrap<Decoder>(decoderObject);
        if (decoder->_asyncData) {
            isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Previous asynchronous operation is not completed.").ToLocalChecked()));
            return;
        }
        plzma::SharedPtr<plzma::ItemArray> items;
        if (args.Length() > 0 && args[0]->IsArray()) {
            Local<Array> arr = Local<Array>::Cast(args[0]);
            const uint32_t arrLen = arr->Length();
            NPLZMA_TRY
            items = plzma::makeShared<plzma::ItemArray>(static_cast<plzma_size_t>(arrLen));
            NPLZMA_CATCH_RET(isolate)
            for (uint32_t i = 0; i < arrLen; i++) {
                bool invalidItem = true;
                Local<Value> val = arr->Get(context, i).ToLocalChecked();
                if (val->IsObject()) {
                    Local<Object> obj = val->ToObject(context).ToLocalChecked();
                    Item * item = Item::TypedUnwrap(obj);
                    if (item) {
                        NPLZMA_TRY
                        items->push(item->_item);
                        invalidItem = false;
                        NPLZMA_CATCH_RET(isolate)
                    }
                }
                if (invalidItem) {
                    NPLZMA_THROW_ARG1_TYPE_ERROR_RET(isolate, "test(items<Item>[%llu?])", static_cast<unsigned long long>(i))
                }
            }
        }
        if (args.Data()->BooleanValue(isolate)) { // async
            Local<Promise::Resolver> resolver = Promise::Resolver::New(context).ToLocalChecked();
            AsyncData * data = nullptr;
            NPLZMA_TRY
            data = new AsyncData();
            data->items = std::move(items);
            data->decoder = decoder->_decoder;
            NPLZMA_CATCH_RET(isolate)
            data->coderObject.Reset(isolate, decoderObject);
            data->resolver.Reset(isolate, resolver);
            data->testing = true;
            if (!decoder->_progressCallback.IsEmpty()) {
                data->progressCallback.Reset(isolate, decoder->_progressCallback);
            }
            decoder->_asyncData = data;
            data->init();
            args.GetReturnValue().Set(resolver->GetPromise());
        } else {
            bool tested = true;
            NPLZMA_TRY
            if (items) {
                tested = decoder->_decoder->test(items);
            } else {
                tested = decoder->_decoder->test();
            }
            NPLZMA_CATCH_RET(isolate)
            args.GetReturnValue().Set(Boolean::New(isolate, tested));
        }
    }
    
    void Decoder::Count(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Decoder * decoder = ObjectWrap::Unwrap<Decoder>(info.Holder());
        plzma_size_t count = 0;
        NPLZMA_TRY
        count = decoder->_decoder->count();
        NPLZMA_CATCH_RET(isolate)
        info.GetReturnValue().Set(Uint32::NewFromUnsigned(isolate, count));
    }
    
    void Decoder::Items(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        Decoder * decoder = ObjectWrap::Unwrap<Decoder>(info.Holder());
        plzma::SharedPtr<plzma::ItemArray> items;
        NPLZMA_TRY
        items = decoder->_decoder->items();
        NPLZMA_CATCH_RET(isolate)
        Local<Array> arr;
        const plzma_size_t count = items->count();
        if (count > 0) {
            arr = Array::New(isolate, count);
            Local<Function> constructor = Item::_constructor.Get(isolate)->GetFunction(context).ToLocalChecked();
            Local<Value> argv[1];
            TryCatch trycatch(isolate);
            for (plzma_size_t i = 0; i < count; i++) {
                MaybeLocal<Object> maybeItemObject = constructor->NewInstance(context, 0, argv);
                if (maybeItemObject.IsEmpty()) {
                    if (trycatch.HasCaught()) {
                        trycatch.ReThrow();
                    } else {
                        isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Can't instantiate Item.").ToLocalChecked()));
                    }
                    return;
                }
                Local<Object> itemObject = maybeItemObject.ToLocalChecked();
                Item * item = Item::TypedUnwrap(itemObject);
                item->_item = std::move(items->at(i));
                arr->Set(context, i, itemObject).Check();
            }
        } else {
            arr = Array::New(isolate);
        }
        info.GetReturnValue().Set(arr);
    }
    
    void Decoder::Init(Local<Object> exports) {
        Isolate * isolate = exports->GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        Local<ObjectTemplate> dataTpl = ObjectTemplate::New(isolate);
        dataTpl->SetInternalFieldCount(1);
        Local<Object> dataObject = dataTpl->NewInstance(context).ToLocalChecked();
        
        // Constructor template: 'Decoder' func tpl
        Local<FunctionTemplate> ctorTpl = FunctionTemplate::New(isolate, Decoder::New, dataObject);
        ctorTpl->SetClassName(String::NewFromUtf8(isolate, "Decoder").ToLocalChecked());
        
        Local<ObjectTemplate> ctorInstTpl = ctorTpl->InstanceTemplate();
        ctorInstTpl->SetInternalFieldCount(1);
        
        // (new Decoder(...)).<func>(...)
        Local<ObjectTemplate> ctorProtoTpl = ctorTpl->PrototypeTemplate();
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "setProgressDelegate").ToLocalChecked(), FunctionTemplate::New(isolate, Decoder::SetProgressDelegate), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "setPassword").ToLocalChecked(), FunctionTemplate::New(isolate, Decoder::SetPassword), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "open").ToLocalChecked(), FunctionTemplate::New(isolate, Decoder::Open, Boolean::New(isolate, false)), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "openAsync").ToLocalChecked(), FunctionTemplate::New(isolate, Decoder::Open, Boolean::New(isolate, true)), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "abort").ToLocalChecked(), FunctionTemplate::New(isolate, Decoder::Abort), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "itemAt").ToLocalChecked(), FunctionTemplate::New(isolate, Decoder::ItemAt), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "extract").ToLocalChecked(), FunctionTemplate::New(isolate, Decoder::Extract, Boolean::New(isolate, false)), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "extractAsync").ToLocalChecked(), FunctionTemplate::New(isolate, Decoder::Extract, Boolean::New(isolate, true)), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "test").ToLocalChecked(), FunctionTemplate::New(isolate, Decoder::Test, Boolean::New(isolate, false)), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "testAsync").ToLocalChecked(), FunctionTemplate::New(isolate, Decoder::Test, Boolean::New(isolate, true)), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        
        // (new Decoder(...)).<prop>
        ctorInstTpl->SetAccessor(String::NewFromUtf8(isolate, "count").ToLocalChecked(), Decoder::Count, nullptr, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(ReadOnly | DontDelete | DontEnum));
        ctorInstTpl->SetAccessor(String::NewFromUtf8(isolate, "items").ToLocalChecked(), Decoder::Items, nullptr, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(ReadOnly | DontDelete | DontEnum));
        
        Local<Function> constructor = ctorTpl->GetFunction(context).ToLocalChecked();
        dataObject->SetInternalField(0, constructor);
        exports->Set(context, String::NewFromUtf8(isolate, "Decoder").ToLocalChecked(), constructor).FromJust();
    }
    
    void Decoder::New(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        if (args.IsConstructCall()) {
            plzma::SharedPtr<plzma::InStream> stream;
            plzma_file_type fileType = static_cast<plzma_file_type>(0);
            bool unsupportedArg1 = true, unsupportedArg2 = true;
            if (args.Length() > 1) {
                if (args[0]->IsObject()) {
                    Local<Object> inObj = args[0]->ToObject(context).ToLocalChecked();
                    InStream * inStream = InStream::TypedUnwrap(inObj);
                    if (inStream) {
                        NPLZMA_TRY
                        stream = inStream->_stream;
                        unsupportedArg1 = false;
                        NPLZMA_CATCH_RET(isolate)
                    }
                }
                uint32_t fileTypeValue = 0;
                bool fileTypeValueDefined = false;
                NPLZMA_GET_UINT32_FROM_VALUE(context, args[1], fileTypeValue, fileTypeValueDefined)
                if (fileTypeValueDefined) {
                    fileType = static_cast<plzma_file_type>(fileTypeValue);
                    unsupportedArg2 = false;
                }
            }
            if (unsupportedArg1) {
                NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "Decoder(?, fileType)")
            }
            if (unsupportedArg2) {
                NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "Decoder(inStream, ?)")
            }
            Decoder * obj = nullptr;
            NPLZMA_TRY
            auto decoder = plzma::makeSharedDecoder(stream, fileType);
            obj = new Decoder(std::move(decoder));
            NPLZMA_CATCH_RET(isolate)
            obj->Wrap(args.This());
            args.GetReturnValue().Set(args.This());
        } else {
            Local<Function> constructor = args.Data().As<Object>()->GetInternalField(0).As<Function>();
            int argc = 0;
            Local<Value> argv[2];
            if (args.Length() > 0) { argv[argc++] = args[0]; }
            if (args.Length() > 1) { argv[argc++] = args[1]; }
            TryCatch trycatch(isolate);
            MaybeLocal<Object> maybeResult = constructor->NewInstance(context, argc, argv);
            if (maybeResult.IsEmpty()) {
                if (trycatch.HasCaught()) {
                    trycatch.ReThrow();
                } else {
                    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Can't instantiate Decoder.").ToLocalChecked()));
                }
                return;
            }
            args.GetReturnValue().Set(maybeResult.ToLocalChecked());
        }
    }
    
    void InStream::Erase(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        InStream * stream = ObjectWrap::Unwrap<InStream>(args.Holder());
        plzma_erase erase = plzma_erase_none;
        if (args.Length() > 0) {
            uint32_t eraseValue = 0;
            bool eraseValueDefined = false;
            NPLZMA_GET_UINT32_FROM_VALUE(context, args[0], eraseValue, eraseValueDefined)
            if (eraseValueDefined) {
                erase = static_cast<plzma_erase>(eraseValue);
            } else {
                NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "erase(?)")
            }
        }
        bool erased = false;
        NPLZMA_TRY
        erased = stream->_stream->erase(erase);
        NPLZMA_CATCH_RET(isolate)
        args.GetReturnValue().Set(Boolean::New(isolate, erased));
    }
    
    void InStream::Opened(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        InStream * stream = ObjectWrap::Unwrap<InStream>(info.Holder());
        bool opened = false;
        NPLZMA_TRY
        opened = stream->_stream->opened();
        NPLZMA_CATCH_RET(isolate)
        info.GetReturnValue().Set(Boolean::New(isolate, opened));
    }
    
    void InStream::Init(Local<Object> exports) {
        Isolate * isolate = exports->GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        Local<ObjectTemplate> dataTpl = ObjectTemplate::New(isolate);
        dataTpl->SetInternalFieldCount(1);
        Local<Object> dataObject = dataTpl->NewInstance(context).ToLocalChecked();
        
        // Constructor template: 'InStream' func tpl
        Local<FunctionTemplate> ctorTpl = FunctionTemplate::New(isolate, InStream::New, dataObject);
        ctorTpl->SetClassName(String::NewFromUtf8(isolate, "InStream").ToLocalChecked());
        
        _constructor.Reset(isolate, ctorTpl);
        
        Local<ObjectTemplate> ctorInstTpl = ctorTpl->InstanceTemplate();
        ctorInstTpl->SetInternalFieldCount(2);
        
        // (new InStream(...)).<func>(...)
        Local<ObjectTemplate> ctorProtoTpl = ctorTpl->PrototypeTemplate();
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "erase").ToLocalChecked(), FunctionTemplate::New(isolate, InStream::Erase), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        
        // (new InStream(...)).<prop>
        ctorInstTpl->SetAccessor(String::NewFromUtf8(isolate, "opened").ToLocalChecked(), InStream::Opened, nullptr, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(ReadOnly | DontDelete | DontEnum));
        
        Local<Function> constructor = ctorTpl->GetFunction(context).ToLocalChecked();
        dataObject->SetInternalField(0, constructor);
        exports->Set(context, String::NewFromUtf8(isolate, "InStream").ToLocalChecked(), constructor).FromJust();
    }
    
    static void InStreamDummyFreeCallback(void * LIBPLZMA_NULLABLE memory) { }

    void InStream::New(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        if (args.IsConstructCall()) {
            std::shared_ptr<BackingStore> backingStore;
            plzma::Path path;
            plzma::InStreamArray multiStreams;
            int method = 0; // 0 - data, 1 - path, 2 - multi volume streams
            bool unsupportedArg = true;
            if (args.Length() > 0) {
                if (args[0]->IsString()) {
                    String::Utf8Value str(isolate, args[0]);
                    NPLZMA_TRY
                    path.set(*str);
                    unsupportedArg = false;
                    method = 1;
                    NPLZMA_CATCH_RET(isolate)
                } else if (args[0]->IsArrayBuffer()) {
                    Local<ArrayBuffer> arrayBuffer = Local<ArrayBuffer>::Cast(args[0]);
                    backingStore = arrayBuffer->GetBackingStore();
                    unsupportedArg = false;
                } else if (args[0]->IsArray()) {
                    Local<Array> arr = Local<Array>::Cast(args[0]);
                    const uint32_t arrLen = arr->Length();
                    NPLZMA_TRY
                    multiStreams = plzma::InStreamArray(static_cast<plzma_size_t>(arrLen));
                    NPLZMA_CATCH_RET(isolate)
                    for (uint32_t i = 0; i < arrLen; i++) {
                        bool invalidInSubStream = true;
                        Local<Value> val = arr->Get(context, i).ToLocalChecked();
                        if (val->IsObject()) {
                            Local<Object> obj = val->ToObject(context).ToLocalChecked();
                            InStream * inSubStream = InStream::TypedUnwrap(obj);
                            if (inSubStream) {
                                NPLZMA_TRY
                                multiStreams.push(inSubStream->_stream);
                                invalidInSubStream = false;
                                NPLZMA_CATCH_RET(isolate)
                            }
                        }
                        if (invalidInSubStream) {
                            NPLZMA_THROW_ARG1_TYPE_ERROR_RET(isolate, "InStream(stream<InStream>[%llu?])", static_cast<unsigned long long>(i))
                        }
                    }
                    unsupportedArg = false;
                    method = 2;
                } else if (args[0]->IsObject()) {
                    Local<Object> inObj = args[0]->ToObject(context).ToLocalChecked();
                    Path * inPath = Path::TypedUnwrap(inObj);
                    if (inPath) {
                        NPLZMA_TRY
                        path.set(inPath->_path);
                        unsupportedArg = false;
                        method = 1;
                        NPLZMA_CATCH_RET(isolate)
                    }
                }
            }
            if (unsupportedArg) {
                NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "InStream(?)")
            }
            InStream * obj = nullptr;
            NPLZMA_TRY
            plzma::SharedPtr<plzma::InStream> stream;
            switch (method) {
                case 0: // data
                    stream = plzma::makeSharedInStream(backingStore->Data(), backingStore->ByteLength(), InStreamDummyFreeCallback);
                    break;
                case 1: // path
                    stream = plzma::makeSharedInStream(std::move(path));
                    break;
                case 2: // multi volume streams
                    stream = plzma::makeSharedInStream(std::move(multiStreams));
                    break;
                default:
                    break;
            }
            obj = new InStream(std::move(stream), std::move(backingStore));
            NPLZMA_CATCH_RET(isolate)
            obj->TypedWrap(args.This());
            args.GetReturnValue().Set(args.This());
        } else {
            Local<Function> constructor = args.Data().As<Object>()->GetInternalField(0).As<Function>();
            int argc = 0;
            Local<Value> argv[2];
            if (args.Length() > 0) { argv[argc++] = args[0]; }
            if (args.Length() > 1) { argv[argc++] = args[1]; }
            TryCatch trycatch(isolate);
            MaybeLocal<Object> maybeResult = constructor->NewInstance(context, argc, argv);
            if (maybeResult.IsEmpty()) {
                if (trycatch.HasCaught()) {
                    trycatch.ReThrow();
                } else {
                    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Can't instantiate InStream.").ToLocalChecked()));
                }
                return;
            }
            args.GetReturnValue().Set(maybeResult.ToLocalChecked());
        }
    }
    
    template<class T>
    void OutStream<T>::Erase(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        OutStream<T> * stream = OutStream<T>::TypedUnwrap(args.Holder());
        plzma_erase erase = plzma_erase_none;
        if (args.Length() > 0) {
            uint32_t eraseValue = 0;
            bool eraseValueDefined = false;
            NPLZMA_GET_UINT32_FROM_VALUE(context, args[0], eraseValue, eraseValueDefined)
            if (eraseValueDefined) {
                erase = static_cast<plzma_erase>(eraseValue);
            } else {
                NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "erase(?)")
            }
        }
        bool erased = false;
        NPLZMA_TRY
        erased = stream->_stream->erase(erase);
        NPLZMA_CATCH_RET(isolate)
        args.GetReturnValue().Set(Boolean::New(isolate, erased));
    }
    
    static void RawHeapMemoryDeleterCallback(void* data, size_t length, void* deleter_data) {
        plzma_free(data);
    }
    
    template<class T>
    void OutStream<T>::CopyContent(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        OutStream<T> * stream = OutStream<T>::TypedUnwrap(args.Holder());
        plzma::RawHeapMemorySize content;
        NPLZMA_TRY
        content = stream->_stream->copyContent();
        NPLZMA_CATCH_RET(isolate)
        auto backingStore = ArrayBuffer::NewBackingStore(content.first.take(), content.second, RawHeapMemoryDeleterCallback, nullptr);
        args.GetReturnValue().Set(ArrayBuffer::New(isolate, std::move(backingStore)));
    }
    
    template<class T>
    void OutStream<T>::Opened(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        OutStream<T> * stream = OutStream<T>::TypedUnwrap(info.Holder());
        bool opened = false;
        NPLZMA_TRY
        opened = stream->_stream->opened();
        NPLZMA_CATCH_RET(isolate)
        info.GetReturnValue().Set(Boolean::New(isolate, opened));
    }

    template<>
    void OutStream<plzma::OutMultiStream>::Streams(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        OutStream<plzma::OutMultiStream> * stream = OutStream<plzma::OutMultiStream>::TypedUnwrap(info.Holder());
        plzma::OutStreamArray streams;
        NPLZMA_TRY
        streams = stream->_stream->streams();
        NPLZMA_CATCH_RET(isolate)
        const auto count = streams.count();
        if (count == 0) {
            info.GetReturnValue().Set(Array::New(isolate));
            return;
        }
        auto array = Array::New(isolate, count);
        Local<Value> argv[1];
        argv[0] = OutStream<plzma::OutStream>::_instantiateWithoutNativeStream.Get(isolate);
        Local<Function> constructor = OutStream<plzma::OutStream>::_constructor.Get(isolate)->GetFunction(context).ToLocalChecked();
        TryCatch trycatch(isolate);
        for (plzma_size_t i = 0; i < count; i++) {
            auto stream = streams.at(i);
            if (stream) {
                MaybeLocal<Object> maybeStreamObject = constructor->NewInstance(context, 1, argv);
                if (maybeStreamObject.IsEmpty()) {
                    if (trycatch.HasCaught()) {
                        trycatch.ReThrow();
                    } else {
                        isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Can't instantiate OutStream.").ToLocalChecked()));
                    }
                    return;
                }
                auto streamObject = maybeStreamObject.ToLocalChecked();
                OutStream<plzma::OutStream> * streamPtr = OutStream<plzma::OutStream>::TypedUnwrap(streamObject);
                streamPtr->_stream = std::move(stream);
                auto unused = array->Set(context, i, streamObject);
                unused.Check();
            } else {
                NPLZMA_THROW_ARG1_TYPE_ERROR_RET(isolate, "streams[%llu] is empty.", static_cast<unsigned long long>(i))
            }
        }
        info.GetReturnValue().Set(array);
    }

    template<class T>
    void OutStream<T>::Init(Local<Object> exports) {
        const char * className = nullptr;
        if (std::is_same<T, plzma::OutStream>::value) {
            className = "OutStream";
        } else if (std::is_same<T, plzma::OutMultiStream>::value) {
            className = "OutMultiStream";
        }
        Isolate * isolate = exports->GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        Local<ObjectTemplate> dataTpl = ObjectTemplate::New(isolate);
        dataTpl->SetInternalFieldCount(1);
        Local<Object> dataObject = dataTpl->NewInstance(context).ToLocalChecked();
        
        // Constructor template: 'OutStream' func tpl
        Local<FunctionTemplate> ctorTpl = FunctionTemplate::New(isolate, OutStream<T>::New, dataObject);
        ctorTpl->SetClassName(String::NewFromUtf8(isolate, className).ToLocalChecked());
        
        OutStream<T>::_constructor.Reset(isolate, ctorTpl);
        OutStream<T>::_instantiateWithoutNativeStream.Reset(isolate, Symbol::New(isolate));
        
        Local<ObjectTemplate> ctorInstTpl = ctorTpl->InstanceTemplate();
        ctorInstTpl->SetInternalFieldCount(2);
        
        // (new OutStream(...)).<func>(...)
        Local<ObjectTemplate> ctorProtoTpl = ctorTpl->PrototypeTemplate();
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "erase").ToLocalChecked(), FunctionTemplate::New(isolate, OutStream<T>::Erase), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "copyContent").ToLocalChecked(), FunctionTemplate::New(isolate, OutStream<T>::CopyContent), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        
        // (new OutStream(...)).<prop>
        ctorInstTpl->SetAccessor(String::NewFromUtf8(isolate, "opened").ToLocalChecked(), OutStream<T>::Opened, nullptr, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(ReadOnly | DontDelete | DontEnum));
        if (std::is_same<T, plzma::OutMultiStream>::value) {
            ctorInstTpl->SetAccessor(String::NewFromUtf8(isolate, "streams").ToLocalChecked(), OutStream<T>::Streams, nullptr, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(ReadOnly | DontDelete | DontEnum));
        }
        
        Local<Function> constructor = ctorTpl->GetFunction(context).ToLocalChecked();
        dataObject->SetInternalField(0, constructor);
        exports->Set(context, String::NewFromUtf8(isolate, className).ToLocalChecked(), constructor).FromJust();
    }
    
    template<>
    void OutStream<plzma::OutStream>::New(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        if (args.IsConstructCall()) {
            plzma::Path path;
            bool instantiateWithoutNativeStream = false;
            if (args.Length() > 0) {
                bool valueDefined = false;
                NPLZMA_GET_PATH_FROM_VALUE(isolate, context, args[0], path, valueDefined)
                if (!valueDefined) {
                    if (args[0]->StrictEquals(_instantiateWithoutNativeStream.Get(isolate))) {
                        instantiateWithoutNativeStream = true;
                    } else {
                        NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "OutStream(?)")
                    }
                }
            }
            OutStream<plzma::OutStream> * obj = nullptr;
            NPLZMA_TRY
            if (instantiateWithoutNativeStream) {
                obj = new OutStream<plzma::OutStream>();
            } else {
                auto stream = (args.Length() > 0) ? plzma::makeSharedOutStream(std::move(path)) : plzma::makeSharedOutStream();
                obj = new OutStream<plzma::OutStream>(std::move(stream));
            }
            NPLZMA_CATCH_RET(isolate)
            obj->TypedWrap(args.This());
            args.GetReturnValue().Set(args.This());
        } else {
            Local<Function> constructor = args.Data().As<Object>()->GetInternalField(0).As<Function>();
            int argc = 0;
            Local<Value> argv[1];
            if (args.Length() > 0) { argv[argc++] = args[0]; }
            TryCatch trycatch(isolate);
            MaybeLocal<Object> maybeResult = constructor->NewInstance(context, argc, argv);
            if (maybeResult.IsEmpty()) {
                if (trycatch.HasCaught()) {
                    trycatch.ReThrow();
                } else {
                    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Can't instantiate OutStream.").ToLocalChecked()));
                }
                return;
            }
            args.GetReturnValue().Set(maybeResult.ToLocalChecked());
        }
    }

    template<>
    void OutStream<plzma::OutMultiStream>::New(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        if (args.IsConstructCall()) {
            plzma::Path dirPath;
            plzma::String partName;
            plzma::String partExtension;
            uint32_t formatValue = 0;
            plzma_size_t partSize = 0;
            int method = 0;
            if (args.Length() > 0) {
                bool valueDefined = false;
                NPLZMA_GET_UINT32_FROM_VALUE(context, args[0], partSize, valueDefined)
                if (valueDefined) {
                    method = 1;
                } else if (args.Length() == 5) {
                    for (int i = 0; i < 5; i++) {
                        valueDefined = false;
                        switch (i) {
                            case 0:
                                NPLZMA_GET_PATH_FROM_VALUE(isolate, context, args[i], dirPath, valueDefined)
                                break;
                            case 1:
                                NPLZMA_GET_PATH_FROM_VALUE(isolate, context, args[i], partName, valueDefined)
                                break;
                            case 2:
                                NPLZMA_GET_PATH_FROM_VALUE(isolate, context, args[i], partExtension, valueDefined)
                                break;
                            case 3:
                                NPLZMA_GET_UINT32_FROM_VALUE(context, args[i], formatValue, valueDefined)
                                break;
                            case 4:
                                NPLZMA_GET_UINT32_FROM_VALUE(context, args[i], partSize, valueDefined)
                                break;
                            default:
                                break;
                        }
                        if (!valueDefined) {
                            NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "OutMultiStream(?)")
                        }
                        method = 2;
                    }
                }
            }
            if (method == 0) {
                NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "OutMultiStream(?)")
            }
            OutStream<plzma::OutMultiStream> * obj = nullptr;
            NPLZMA_TRY
            plzma::SharedPtr<plzma::OutMultiStream> stream;
            switch (method) {
                case 1:
                    stream = plzma::makeSharedOutMultiStream(partSize);
                    break;
                case 2:
                    stream = plzma::makeSharedOutMultiStream(std::move(dirPath), std::move(partName), std::move(partExtension), static_cast<plzma_plzma_multi_stream_part_name_format>(formatValue), partSize);
                    break;
                default:
                    break;
            }
            obj = new OutStream<plzma::OutMultiStream>(std::move(stream));
            NPLZMA_CATCH_RET(isolate)
            obj->TypedWrap(args.This());
            args.GetReturnValue().Set(args.This());
        } else {
            Local<Function> constructor = args.Data().As<Object>()->GetInternalField(0).As<Function>();
            int argc = 0;
            Local<Value> argv[5];
            for (int i = 0; i < args.Length() && i < 5; i++) {
                argv[argc++] = args[i];
            }
            TryCatch trycatch(isolate);
            MaybeLocal<Object> maybeResult = constructor->NewInstance(context, argc, argv);
            if (maybeResult.IsEmpty()) {
                if (trycatch.HasCaught()) {
                    trycatch.ReThrow();
                } else {
                    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Can't instantiate OutMultiStream.").ToLocalChecked()));
                }
                return;
            }
            args.GetReturnValue().Set(maybeResult.ToLocalChecked());
        }
    }
    
    void Item::ToString(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        std::ostringstream oss;
        const auto item = ObjectWrap::Unwrap<Item>(args.Holder())->_item;
        oss << "[path: " << item->path().utf8()
        << ", idx: " << item->index()
        << ", size: " << item->size()
        << ", psize: " << item->packSize()
        << ", crc32: " << item->crc32()
        << ", cts: " << item->creationTime()
        << ", ats: " << item->accessTime()
        << ", mts: " << item->modificationTime()
        << ", enc: " << item->encrypted()
        << ", dir: " << item->isDir() << "]" << std::endl;
        const auto str = oss.str();
        const char * utf8 = str.c_str();
        args.GetReturnValue().Set(String::NewFromUtf8(isolate, utf8 ? utf8 : "").ToLocalChecked());
    }
    
    void Item::GetIndex(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        const auto item = ObjectWrap::Unwrap<Item>(info.Holder())->_item;
        info.GetReturnValue().Set(Uint32::NewFromUnsigned(isolate, item->index()));
    }
    
    void Item::GetPath(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        const auto item = ObjectWrap::Unwrap<Item>(info.Holder())->_item;
        Local<Function> constructor = Path::_constructor.Get(isolate)->GetFunction(context).ToLocalChecked();
        Local<Value> argv[1];
        TryCatch trycatch(isolate);
        MaybeLocal<Object> maybePathObject = constructor->NewInstance(context, 0, argv);
        if (maybePathObject.IsEmpty()) {
            if (trycatch.HasCaught()) {
                trycatch.ReThrow();
            } else {
                isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Can't instantiate Path.").ToLocalChecked()));
            }
            return;
        }
        Local<Object> pathObject = maybePathObject.ToLocalChecked();
        Path * path = Path::TypedUnwrap(pathObject);
        NPLZMA_TRY
        path->_path = item->path();
        NPLZMA_CATCH_RET(isolate)
        info.GetReturnValue().Set(pathObject);
    }
    
    void Item::GetSize(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        const auto item = ObjectWrap::Unwrap<Item>(info.Holder())->_item;
        info.GetReturnValue().Set(BigInt::NewFromUnsigned(isolate, item->size()));
    }
    
    void Item::SetSize(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        auto item = ObjectWrap::Unwrap<Item>(info.Holder())->_item;
        uint64_t sizeValue = 0;
        bool sizeValueDefined = false;
        NPLZMA_GET_UINT64_FROM_VALUE(context, value, sizeValue, sizeValueDefined)
        if (sizeValueDefined) {
            item->setSize(sizeValue);
        } else {
            NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "size")
        }
    }
    
    void Item::GetPackSize(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        const auto item = ObjectWrap::Unwrap<Item>(info.Holder())->_item;
        info.GetReturnValue().Set(BigInt::NewFromUnsigned(isolate, item->packSize()));
    }
    
    void Item::SetPackSize(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        auto item = ObjectWrap::Unwrap<Item>(info.Holder())->_item;
        uint64_t sizeValue = 0;
        bool sizeValueDefined = false;
        NPLZMA_GET_UINT64_FROM_VALUE(context, value, sizeValue, sizeValueDefined)
        if (sizeValueDefined) {
            item->setPackSize(sizeValue);
        } else {
            NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "packSize")
        }
    }
    
    void Item::GetCrc32(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        const auto item = ObjectWrap::Unwrap<Item>(info.Holder())->_item;
        info.GetReturnValue().Set(Uint32::NewFromUnsigned(isolate, item->crc32()));
    }
    
    void Item::SetCrc32(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        auto item = ObjectWrap::Unwrap<Item>(info.Holder())->_item;
        uint32_t crc32Value = 0;
        bool crc32ValueDefined = false;
        NPLZMA_GET_UINT32_FROM_VALUE(context, value, crc32Value, crc32ValueDefined)
        if (crc32ValueDefined) {
            item->setCrc32(crc32Value);
        } else {
            NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "crc32")
        }
    }
    
    void Item::GetCreationDate(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        const auto item = ObjectWrap::Unwrap<Item>(info.Holder())->_item;
        info.GetReturnValue().Set(Date::New(context, item->creationTime() * 1000).ToLocalChecked());
    }
    
    void Item::SetCreationDate(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        auto item = ObjectWrap::Unwrap<Item>(info.Holder())->_item;
        if (value->IsNumber()) {
            Local<Number> num = Local<Number>::Cast(value);
            item->setCreationTime(num->Value() / 1000);
        } else if (value->IsDate()) {
            Local<Date> date = Local<Date>::Cast(value);
            item->setCreationTime(date->ValueOf() / 1000);
        } else {
            NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "creationDate")
        }
    }
    
    void Item::GetAccessDate(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        const auto item = ObjectWrap::Unwrap<Item>(info.Holder())->_item;
        info.GetReturnValue().Set(Date::New(context, item->accessTime() * 1000).ToLocalChecked());
    }
    
    void Item::SetAccessDate(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        auto item = ObjectWrap::Unwrap<Item>(info.Holder())->_item;
        if (value->IsNumber()) {
            Local<Number> num = Local<Number>::Cast(value);
            item->setAccessTime(num->Value() / 1000);
        } else if (value->IsDate()) {
            Local<Date> date = Local<Date>::Cast(value);
            item->setAccessTime(date->ValueOf() / 1000);
        } else {
            NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "accessDate")
        }
    }
    
    void Item::GetModificationDate(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        const auto item = ObjectWrap::Unwrap<Item>(info.Holder())->_item;
        info.GetReturnValue().Set(Date::New(context, item->modificationTime() * 1000).ToLocalChecked());
    }
    
    void Item::SetModificationDate(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        auto item = ObjectWrap::Unwrap<Item>(info.Holder())->_item;
        if (value->IsNumber()) {
            Local<Number> num = Local<Number>::Cast(value);
            item->setModificationTime(num->Value() / 1000);
        } else if (value->IsDate()) {
            Local<Date> date = Local<Date>::Cast(value);
            item->setModificationTime(date->ValueOf() / 1000);
        } else {
            NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "modificationDate")
        }
    }
    
    void Item::GetEncrypted(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        const auto item = ObjectWrap::Unwrap<Item>(info.Holder())->_item;
        info.GetReturnValue().Set(Boolean::New(isolate, item->encrypted()));
    }
    
    void Item::SetEncrypted(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        auto item = ObjectWrap::Unwrap<Item>(info.Holder())->_item;
        item->setEncrypted(value->BooleanValue(isolate));
    }
    
    void Item::GetIsDir(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        const auto item = ObjectWrap::Unwrap<Item>(info.Holder())->_item;
        info.GetReturnValue().Set(Boolean::New(isolate, item->isDir()));
    }
    
    void Item::SetIsDir(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        auto item = ObjectWrap::Unwrap<Item>(info.Holder())->_item;
        item->setIsDir(value->BooleanValue(isolate));
    }
    
    void Item::Init(Local<Object> exports) {
        Isolate * isolate = exports->GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        Local<ObjectTemplate> dataTpl = ObjectTemplate::New(isolate);
        dataTpl->SetInternalFieldCount(1);
        Local<Object> dataObject = dataTpl->NewInstance(context).ToLocalChecked();
        
        // Constructor template: 'Item' func tpl
        Local<FunctionTemplate> ctorTpl = FunctionTemplate::New(isolate, Item::New, dataObject);
        ctorTpl->SetClassName(String::NewFromUtf8(isolate, "Item").ToLocalChecked());
        
        _constructor.Reset(isolate, ctorTpl);
        
        Local<ObjectTemplate> ctorInstTpl = ctorTpl->InstanceTemplate();
        ctorInstTpl->SetInternalFieldCount(2);
        
        // (new Item(...)).<func>(...)
        Local<ObjectTemplate> ctorProtoTpl = ctorTpl->PrototypeTemplate();
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "toString").ToLocalChecked(), FunctionTemplate::New(isolate, Item::ToString), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        
        // (new Item(...)).<prop>
        ctorInstTpl->SetAccessor(String::NewFromUtf8(isolate, "path").ToLocalChecked(), Item::GetPath, nullptr, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(ReadOnly | DontDelete | DontEnum));
        ctorInstTpl->SetAccessor(String::NewFromUtf8(isolate, "index").ToLocalChecked(), Item::GetIndex, nullptr, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(ReadOnly | DontDelete | DontEnum));
        ctorInstTpl->SetAccessor(String::NewFromUtf8(isolate, "size").ToLocalChecked(), Item::GetSize, Item::SetSize, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(DontDelete | DontEnum));
        ctorInstTpl->SetAccessor(String::NewFromUtf8(isolate, "packSize").ToLocalChecked(), Item::GetPackSize, Item::SetPackSize, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(DontDelete | DontEnum));
        ctorInstTpl->SetAccessor(String::NewFromUtf8(isolate, "crc32").ToLocalChecked(), Item::GetCrc32, Item::SetCrc32, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(DontDelete | DontEnum));
        ctorInstTpl->SetAccessor(String::NewFromUtf8(isolate, "creationDate").ToLocalChecked(), Item::GetCreationDate, Item::SetCreationDate, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(DontDelete | DontEnum));
        ctorInstTpl->SetAccessor(String::NewFromUtf8(isolate, "accessDate").ToLocalChecked(), Item::GetAccessDate, Item::SetAccessDate, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(DontDelete | DontEnum));
        ctorInstTpl->SetAccessor(String::NewFromUtf8(isolate, "modificationDate").ToLocalChecked(), Item::GetModificationDate, Item::SetModificationDate, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(DontDelete | DontEnum));
        ctorInstTpl->SetAccessor(String::NewFromUtf8(isolate, "encrypted").ToLocalChecked(), Item::GetEncrypted, Item::SetEncrypted, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(DontDelete | DontEnum));
        ctorInstTpl->SetAccessor(String::NewFromUtf8(isolate, "isDir").ToLocalChecked(), Item::GetIsDir, Item::SetIsDir, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(DontDelete | DontEnum));
        
        Local<Function> constructor = ctorTpl->GetFunction(context).ToLocalChecked();
        dataObject->SetInternalField(0, constructor);
        exports->Set(context, String::NewFromUtf8(isolate, "Item").ToLocalChecked(), constructor).FromJust();
    }
    
    void Item::New(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        if (args.IsConstructCall()) {
            plzma::Path path;
            plzma_size_t index = 0;
            bool unsupportedArg1 = false, unsupportedArg2 = false;
            if (args.Length() > 0) {
                bool valueDefined = false;
                NPLZMA_GET_PATH_FROM_VALUE(isolate, context, args[0], path, valueDefined)
                unsupportedArg1 = !valueDefined;
            }
            if (args.Length() > 1) {
                uint32_t indexValue = 0;
                bool indexValueDefined = false;
                NPLZMA_GET_UINT32_FROM_VALUE(context, args[1], indexValue, indexValueDefined)
                if (indexValueDefined) {
                    index = indexValue;
                } else {
                    unsupportedArg2 = true;
                }
            }
            if (unsupportedArg1) {
                NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "Item(?, index)")
            }
            if (unsupportedArg2) {
                NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "Item(path, ?)")
            }
            Item * obj = nullptr;
            NPLZMA_TRY
            auto item = plzma::makeShared<plzma::Item>(std::move(path), index);
            obj = new Item(std::move(item));
            NPLZMA_CATCH_RET(isolate)
            obj->TypedWrap(args.This());
            args.GetReturnValue().Set(args.This());
        } else {
            Local<Function> constructor = args.Data().As<Object>()->GetInternalField(0).As<Function>();
            int argc = 0;
            Local<Value> argv[2];
            if (args.Length() > 0) { argv[0] = args[0]; argc++; }
            if (args.Length() > 1) { argv[1] = args[1]; argc++; }
            TryCatch trycatch(isolate);
            MaybeLocal<Object> maybeResult = constructor->NewInstance(context, argc, argv);
            if (maybeResult.IsEmpty()) {
                if (trycatch.HasCaught()) {
                    trycatch.ReThrow();
                } else {
                    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Can't instantiate Item.").ToLocalChecked()));
                }
                return;
            }
            args.GetReturnValue().Set(maybeResult.ToLocalChecked());
        }
    }
    
    void PathIterator::Next(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        PathIterator * iterator = ObjectWrap::Unwrap<PathIterator>(args.Holder());
        bool next = false;
        NPLZMA_TRY
        next = iterator->_iterator->next();
        NPLZMA_CATCH_RET(isolate)
        args.GetReturnValue().Set(Boolean::New(isolate, next));
    }
    
    void PathIterator::Close(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        PathIterator * iterator = ObjectWrap::Unwrap<PathIterator>(args.Holder());
        iterator->_iterator->close();
    }
    
    void PathIterator::GetPath(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        PathIterator * iterator = ObjectWrap::Unwrap<PathIterator>(info.Holder());
        Local<Function> constructor = Path::_constructor.Get(isolate)->GetFunction(context).ToLocalChecked();
        Local<Value> argv[1];
        TryCatch trycatch(isolate);
        MaybeLocal<Object> maybePathObject = constructor->NewInstance(context, 0, argv);
        if (maybePathObject.IsEmpty()) {
            if (trycatch.HasCaught()) {
                trycatch.ReThrow();
            } else {
                isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Can't instantiate Path.").ToLocalChecked()));
            }
            return;
        }
        Local<Object> pathObject = maybePathObject.ToLocalChecked();
        Path * path = Path::TypedUnwrap(pathObject);
        NPLZMA_TRY
        auto iteratorPath = iterator->_iterator->path();
        path->_path = std::move(iteratorPath);
        NPLZMA_CATCH_RET(isolate)
        info.GetReturnValue().Set(pathObject);
    }
    
    void PathIterator::Component(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        PathIterator * iterator = ObjectWrap::Unwrap<PathIterator>(info.Holder());
        Local<Function> constructor = Path::_constructor.Get(isolate)->GetFunction(context).ToLocalChecked();
        Local<Value> argv[1];
        TryCatch trycatch(isolate);
        MaybeLocal<Object> maybePathObject = constructor->NewInstance(context, 0, argv);
        if (maybePathObject.IsEmpty()) {
            if (trycatch.HasCaught()) {
                trycatch.ReThrow();
            } else {
                isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Can't instantiate Path.").ToLocalChecked()));
            }
            return;
        }
        Local<Object> pathObject = maybePathObject.ToLocalChecked();
        Path * path = Path::TypedUnwrap(pathObject);
        NPLZMA_TRY
        auto comp = iterator->_iterator->component();
        path->_path = std::move(comp);
        NPLZMA_CATCH_RET(isolate)
        info.GetReturnValue().Set(pathObject);
    }
    
    void PathIterator::FullPath(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        PathIterator * iterator = ObjectWrap::Unwrap<PathIterator>(info.Holder());
        Local<Function> constructor = Path::_constructor.Get(isolate)->GetFunction(context).ToLocalChecked();
        Local<Value> argv[1];
        TryCatch trycatch(isolate);
        MaybeLocal<Object> maybePathObject = constructor->NewInstance(context, 0, argv);
        if (maybePathObject.IsEmpty()) {
            if (trycatch.HasCaught()) {
                trycatch.ReThrow();
            } else {
                isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Can't instantiate Path.").ToLocalChecked()));
            }
            return;
        }
        Local<Object> pathObject = maybePathObject.ToLocalChecked();
        Path * path = Path::TypedUnwrap(pathObject);
        NPLZMA_TRY
        auto fullPath = iterator->_iterator->fullPath();
        path->_path = std::move(fullPath);
        NPLZMA_CATCH_RET(isolate)
        info.GetReturnValue().Set(pathObject);
    }
    
    void PathIterator::IsDir(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        PathIterator * iterator = ObjectWrap::Unwrap<PathIterator>(info.Holder());
        info.GetReturnValue().Set(Boolean::New(isolate, iterator->_iterator->isDir()));
    }
    
    void Path::Count(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Path * path = ObjectWrap::Unwrap<Path>(info.Holder());
        plzma_size_t count = 0;
        NPLZMA_TRY
        count = path->_path.count();
        NPLZMA_CATCH_RET(isolate)
        info.GetReturnValue().Set(Uint32::NewFromUnsigned(isolate, count));
    }
    
    void Path::Exists(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Path * path = ObjectWrap::Unwrap<Path>(info.Holder());
        bool exists = false, isDir = false;
        NPLZMA_TRY
        exists = path->_path.exists(&isDir);
        NPLZMA_CATCH_RET(isolate)
        info.GetReturnValue().Set(Int32::New(isolate, (exists ? (isDir ? 2 : 1) : 0)));
    }
    
    void Path::Readable(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Path * path = ObjectWrap::Unwrap<Path>(info.Holder());
        bool readable = false;
        NPLZMA_TRY
        readable = path->_path.readable();
        NPLZMA_CATCH_RET(isolate)
        info.GetReturnValue().Set(Boolean::New(isolate, readable));
    }
    
    void Path::Writable(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Path * path = ObjectWrap::Unwrap<Path>(info.Holder());
        bool writable = false;
        NPLZMA_TRY
        writable = path->_path.writable();
        NPLZMA_CATCH_RET(isolate)
        info.GetReturnValue().Set(Boolean::New(isolate, writable));
    }
    
    void Path::ReadableAndWritable(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Path * path = ObjectWrap::Unwrap<Path>(info.Holder());
        bool writable = false;
        NPLZMA_TRY
        writable = path->_path.readableAndWritable();
        NPLZMA_CATCH_RET(isolate)
        info.GetReturnValue().Set(Boolean::New(isolate, writable));
    }
    
    void Path::Stat(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        Path * path = ObjectWrap::Unwrap<Path>(info.Holder());
        Local<Object> statObject = Object::New(isolate);
        plzma_path_stat stat;
        NPLZMA_TRY
        stat = path->_path.stat();
        NPLZMA_CATCH_RET(isolate)
        statObject->DefineOwnProperty(context, String::NewFromUtf8(isolate, "size").ToLocalChecked(), BigInt::NewFromUnsigned(isolate, stat.size), static_cast<PropertyAttribute>(ReadOnly | DontDelete)).Check();
        statObject->DefineOwnProperty(context, String::NewFromUtf8(isolate, "creation").ToLocalChecked(), Date::New(context, stat.timestamp.creation * 1000).ToLocalChecked(), static_cast<PropertyAttribute>(ReadOnly | DontDelete)).Check();
        statObject->DefineOwnProperty(context, String::NewFromUtf8(isolate, "lastAccess").ToLocalChecked(), Date::New(context, stat.timestamp.last_access * 1000).ToLocalChecked(), static_cast<PropertyAttribute>(ReadOnly | DontDelete)).Check();
        statObject->DefineOwnProperty(context, String::NewFromUtf8(isolate, "lastModification").ToLocalChecked(), Date::New(context, stat.timestamp.last_modification * 1000).ToLocalChecked(), static_cast<PropertyAttribute>(ReadOnly | DontDelete)).Check();
        info.GetReturnValue().Set(statObject);
    }
    
    void Path::OpenDir(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        Local<ObjectTemplate> tpl = ObjectTemplate::New(isolate);
        tpl->SetInternalFieldCount(1);
        
        tpl->Set(String::NewFromUtf8(isolate, "next").ToLocalChecked(), FunctionTemplate::New(isolate, PathIterator::Next), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        tpl->Set(String::NewFromUtf8(isolate, "close").ToLocalChecked(), FunctionTemplate::New(isolate, PathIterator::Close), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        
        tpl->SetAccessor(String::NewFromUtf8(isolate, "path").ToLocalChecked(), PathIterator::GetPath, nullptr, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        tpl->SetAccessor(String::NewFromUtf8(isolate, "component").ToLocalChecked(), PathIterator::Component, nullptr, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        tpl->SetAccessor(String::NewFromUtf8(isolate, "fullPath").ToLocalChecked(), PathIterator::FullPath, nullptr, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        tpl->SetAccessor(String::NewFromUtf8(isolate, "isDir").ToLocalChecked(), PathIterator::IsDir, nullptr, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        
        Local<Object> iteratorObject = tpl->NewInstance(context).ToLocalChecked();
        Path * path = ObjectWrap::Unwrap<Path>(args.Holder());
        uint32_t mode = 0;
        if (args.Length() > 0) {
            bool unsupportedArg = true;
            if (args[0]->IsArray()) {
                unsupportedArg = false;
                Local<Array> arr = Local<Array>::Cast(args[0]);
                for (uint32_t i = 0, n = arr->Length(); i < n; i++) {
                    Local<Value> obj = arr->Get(context, i).ToLocalChecked();
                    uint32_t modeValue = 0;
                    bool modeValueDefined = false;
                    NPLZMA_GET_UINT32_FROM_VALUE(context, obj, modeValue, modeValueDefined)
                    if (modeValueDefined) {
                        mode |= modeValue;
                    } else {
                        NPLZMA_THROW_ARG1_TYPE_ERROR_RET(isolate, "openDir(mode[%llu?])", static_cast<unsigned long long>(i))
                    }
                }
            } else {
                uint32_t modeValue = 0;
                bool modeValueDefined = false;
                NPLZMA_GET_UINT32_FROM_VALUE(context, args[0], modeValue, modeValueDefined)
                if (modeValueDefined) {
                    mode |= modeValue;
                    unsupportedArg = false;
                }
            }
            if (unsupportedArg) {
                NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "openDir(?)")
            }
        }
        
        PathIterator * iterator = nullptr;
        NPLZMA_TRY
        auto it = path->_path.openDir(static_cast<plzma_open_dir_mode_t>(mode));
        iterator = new PathIterator(std::move(it));
        NPLZMA_CATCH_RET(isolate)
        
        iterator->Wrap(iteratorObject);
        args.GetReturnValue().Set(iteratorObject);
    }
    
    void Path::ToString(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Path * path = ObjectWrap::Unwrap<Path>(args.Holder());
        const char * utf8 = nullptr;
        NPLZMA_TRY
        utf8 = path->_path.utf8();
        NPLZMA_CATCH_RET(isolate)
        args.GetReturnValue().Set(String::NewFromUtf8(isolate, utf8).ToLocalChecked());
    }
    
    void Path::Clear(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        Local<Object> pathObject = args.Holder();
        Path * path = ObjectWrap::Unwrap<Path>(pathObject);
        plzma_erase erase = plzma_erase_none;
        if (args.Length() > 0) {
            uint32_t eraseValue = 0;
            bool eraseValueDefined = false;
            NPLZMA_GET_UINT32_FROM_VALUE(context, args[0], eraseValue, eraseValueDefined)
            if (eraseValueDefined) {
                erase = static_cast<plzma_erase>(eraseValue);
            } else {
                NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "erase(?)")
            }
        }
        path->_path.clear(erase);
        args.GetReturnValue().Set(pathObject);
    }
    
    void Path::Set(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        Local<Object> pathObject = args.Holder();
        Path * path = ObjectWrap::Unwrap<Path>(pathObject);
        bool unsupportedArg = true;
        if (args.Length() > 0) {
            if (args[0]->IsString()) {
                String::Utf8Value str(isolate, args[0]);
                NPLZMA_TRY
                path->_path.set(*str);
                unsupportedArg = false;
                NPLZMA_CATCH_RET(isolate)
            } else if (args[0]->IsObject()) {
                Local<Object> inObj = args[0]->ToObject(context).ToLocalChecked();
                Path * inPath = Path::TypedUnwrap(inObj);
                if (inPath) {
                    NPLZMA_TRY
                    path->_path = inPath->_path;
                    unsupportedArg = false;
                    NPLZMA_CATCH_RET(isolate)
                }
            }
        }
        if (unsupportedArg) {
            NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "set(?)")
        }
        args.GetReturnValue().Set(pathObject);
    }
    
    void Path::Append(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        Local<Object> pathObject = args.Holder();
        Path * path = ObjectWrap::Unwrap<Path>(pathObject);
        bool unsupportedArg = true;
        if (args.Length() > 0) {
            if (args[0]->IsString()) {
                String::Utf8Value str(isolate, args[0]);
                NPLZMA_TRY
                path->_path.append(*str);
                unsupportedArg = false;
                NPLZMA_CATCH_RET(isolate)
            } else if (args[0]->IsObject()) {
                auto inObj = args[0]->ToObject(context).ToLocalChecked();
                Path * inPath = Path::TypedUnwrap(inObj);
                if (inPath) {
                    NPLZMA_TRY
                    path->_path.append(inPath->_path);
                    unsupportedArg = false;
                    NPLZMA_CATCH_RET(isolate)
                }
            }
        }
        if (unsupportedArg) {
            NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "append(?)")
        }
        args.GetReturnValue().Set(pathObject);
    }

    void Path::Appending(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        Local<Object> pathObject = args.Holder();
        Path * path = ObjectWrap::Unwrap<Path>(pathObject);
        plzma::Path resultPath;
        bool unsupportedArg = true;
        if (args.Length() > 0) {
            if (args[0]->IsString()) {
                String::Utf8Value str(isolate, args[0]);
                NPLZMA_TRY
                resultPath = path->_path.appending(*str);
                unsupportedArg = false;
                NPLZMA_CATCH_RET(isolate)
            } else if (args[0]->IsObject()) {
                auto inObj = args[0]->ToObject(context).ToLocalChecked();
                Path * inPath = Path::TypedUnwrap(inObj);
                if (inPath) {
                    NPLZMA_TRY
                    resultPath = path->_path.appending(inPath->_path);
                    unsupportedArg = false;
                    NPLZMA_CATCH_RET(isolate)
                }
            }
        }
        if (unsupportedArg) {
            NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "appending(?)")
        }
        Local<Function> constructor = _constructor.Get(isolate)->GetFunction(context).ToLocalChecked();
        Local<Value> argv[1];
        TryCatch trycatch(isolate);
        MaybeLocal<Object> maybeResultPathObject = constructor->NewInstance(context, 0, argv);
        if (maybeResultPathObject.IsEmpty()) {
            if (trycatch.HasCaught()) {
                trycatch.ReThrow();
            } else {
                isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Can't instantiate Path.").ToLocalChecked()));
            }
            return;
        }
        Local<Object> resultPathObject = maybeResultPathObject.ToLocalChecked();
        Path * resultPathObjectPath = Path::TypedUnwrap(resultPathObject);
        resultPathObjectPath->_path = std::move(resultPath);
        args.GetReturnValue().Set(resultPathObject);
    }
    
    void Path::AppendRandomComponent(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Object> pathObject = args.Holder();
        Path * path = ObjectWrap::Unwrap<Path>(pathObject);
        NPLZMA_TRY
        path->_path.appendRandomComponent();
        NPLZMA_CATCH_RET(isolate)
        args.GetReturnValue().Set(pathObject);
    }
    
    void Path::AppendingRandomComponent(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        Local<Object> pathObject = args.Holder();
        Path * path = ObjectWrap::Unwrap<Path>(pathObject);
        plzma::Path resultPath;
        NPLZMA_TRY
        resultPath = path->_path.appendingRandomComponent();
        NPLZMA_CATCH_RET(isolate)
        Local<Function> constructor = _constructor.Get(isolate)->GetFunction(context).ToLocalChecked();
        Local<Value> argv[1];
        TryCatch trycatch(isolate);
        MaybeLocal<Object> maybeResultPathObject = constructor->NewInstance(context, 0, argv);
        if (maybeResultPathObject.IsEmpty()) {
            if (trycatch.HasCaught()) {
                trycatch.ReThrow();
            } else {
                isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Can't instantiate Path.").ToLocalChecked()));
            }
            return;
        }
        Local<Object> resultPathObject = maybeResultPathObject.ToLocalChecked();
        Path * resultPathObjectPath = Path::TypedUnwrap(resultPathObject);
        resultPathObjectPath->_path = std::move(resultPath);
        args.GetReturnValue().Set(resultPathObject);
    }

    void Path::LastComponent(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        Local<Function> constructor = _constructor.Get(isolate)->GetFunction(context).ToLocalChecked();
        Local<Value> argv[1];
        TryCatch trycatch(isolate);
        MaybeLocal<Object> maybeLastPathObject = constructor->NewInstance(context, 0, argv);
        if (maybeLastPathObject.IsEmpty()) {
            if (trycatch.HasCaught()) {
                trycatch.ReThrow();
            } else {
                isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Can't instantiate Path.").ToLocalChecked()));
            }
            return;
        }
        Local<Object> lastPathObject = maybeLastPathObject.ToLocalChecked();
        Path * lastPath = Path::TypedUnwrap(lastPathObject);
        Path * path = ObjectWrap::Unwrap<Path>(args.Holder());
        NPLZMA_TRY
        auto last = path->_path.lastComponent();
        lastPath->_path = std::move(last);
        NPLZMA_CATCH_RET(isolate)
        args.GetReturnValue().Set(lastPathObject);
    }
    
    void Path::RemoveLastComponent(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Object> pathObject = args.Holder();
        Path * path = ObjectWrap::Unwrap<Path>(pathObject);
        NPLZMA_TRY
        path->_path.removeLastComponent();
        NPLZMA_CATCH_RET(isolate)
        args.GetReturnValue().Set(pathObject);
    }

    void Path::RemovingLastComponent(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        Local<Object> pathObject = args.Holder();
        Path * path = ObjectWrap::Unwrap<Path>(pathObject);
        plzma::Path resultPath;
        NPLZMA_TRY
        resultPath = path->_path.removingLastComponent();
        NPLZMA_CATCH_RET(isolate)
        Local<Function> constructor = _constructor.Get(isolate)->GetFunction(context).ToLocalChecked();
        Local<Value> argv[1];
        TryCatch trycatch(isolate);
        MaybeLocal<Object> maybeResultPathObject = constructor->NewInstance(context, 0, argv);
        if (maybeResultPathObject.IsEmpty()) {
            if (trycatch.HasCaught()) {
                trycatch.ReThrow();
            } else {
                isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Can't instantiate Path.").ToLocalChecked()));
            }
            return;
        }
        Local<Object> resultPathObject = maybeResultPathObject.ToLocalChecked();
        Path * resultPathObjectPath = Path::TypedUnwrap(resultPathObject);
        resultPathObjectPath->_path = std::move(resultPath);
        args.GetReturnValue().Set(resultPathObject);
    }
    
    void Path::Remove(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Object> pathObject = args.Holder();
        Path * path = ObjectWrap::Unwrap<Path>(pathObject);
        const bool skipErrors = (args.Length() > 0) ? args[0]->BooleanValue(isolate) : false;
        bool removed = false;
        NPLZMA_TRY
        removed = path->_path.remove(skipErrors);
        NPLZMA_CATCH_RET(isolate)
        args.GetReturnValue().Set(Boolean::New(isolate, removed));
    }
    
    void Path::CreateDir(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Object> pathObject = args.Holder();
        Path * path = ObjectWrap::Unwrap<Path>(pathObject);
        const bool withIntermediates = (args.Length() > 0) ? args[0]->BooleanValue(isolate) : false;
        bool created = false;
        NPLZMA_TRY
        created = path->_path.createDir(withIntermediates);
        NPLZMA_CATCH_RET(isolate)
        args.GetReturnValue().Set(Boolean::New(isolate, created));
    }
    
    void Path::TmpPath(Local<String> property, const PropertyCallbackInfo<Value> & info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        Local<Function> constructor = _constructor.Get(isolate)->GetFunction(context).ToLocalChecked();
        Local<Value> argv[1];
        TryCatch trycatch(isolate);
        MaybeLocal<Object> maybePathObject = constructor->NewInstance(context, 0, argv);
        if (maybePathObject.IsEmpty()) {
            if (trycatch.HasCaught()) {
                trycatch.ReThrow();
            } else {
                isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Can't instantiate Path.").ToLocalChecked()));   
            }
            return;
        }
        Local<Object> pathObject = maybePathObject.ToLocalChecked();
        Path * path = Path::TypedUnwrap(pathObject);
        NPLZMA_TRY
        auto tmpPath = plzma::Path::tmpPath();
        path->_path = std::move(tmpPath);
        NPLZMA_CATCH_RET(isolate)
        info.GetReturnValue().Set(pathObject);
    }
    
    void Path::Init(Local<Object> exports) {
        Isolate * isolate = exports->GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        Local<ObjectTemplate> dataTpl = ObjectTemplate::New(isolate);
        dataTpl->SetInternalFieldCount(1);
        Local<Object> dataObject = dataTpl->NewInstance(context).ToLocalChecked();
        
        // Constructor template: 'Path' func tpl
        Local<FunctionTemplate> ctorTpl = FunctionTemplate::New(isolate, Path::New, dataObject);
        ctorTpl->SetClassName(String::NewFromUtf8(isolate, "Path").ToLocalChecked());
        
        // Path.<tmpPath>
        ctorTpl->SetNativeDataProperty(String::NewFromUtf8(isolate, "tmpPath").ToLocalChecked(), Path::TmpPath, nullptr, Local<Value>(), static_cast<PropertyAttribute>(ReadOnly | DontDelete | DontEnum));
        
        _constructor.Reset(isolate, ctorTpl);
        
        Local<ObjectTemplate> ctorInstTpl = ctorTpl->InstanceTemplate();
        ctorInstTpl->SetInternalFieldCount(2);
        
        // (new Path(...)).<func>(...)
        Local<ObjectTemplate> ctorProtoTpl = ctorTpl->PrototypeTemplate();
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "toString").ToLocalChecked(), FunctionTemplate::New(isolate, Path::ToString), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "openDir").ToLocalChecked(), FunctionTemplate::New(isolate, Path::OpenDir), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "clear").ToLocalChecked(), FunctionTemplate::New(isolate, Path::Clear), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "set").ToLocalChecked(), FunctionTemplate::New(isolate, Path::Set), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "append").ToLocalChecked(), FunctionTemplate::New(isolate, Path::Append), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "appending").ToLocalChecked(), FunctionTemplate::New(isolate, Path::Appending), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "appendRandomComponent").ToLocalChecked(), FunctionTemplate::New(isolate, Path::AppendRandomComponent), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "appendingRandomComponent").ToLocalChecked(), FunctionTemplate::New(isolate, Path::AppendingRandomComponent), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "lastComponent").ToLocalChecked(), FunctionTemplate::New(isolate, Path::LastComponent), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "removeLastComponent").ToLocalChecked(), FunctionTemplate::New(isolate, Path::RemoveLastComponent), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "removingLastComponent").ToLocalChecked(), FunctionTemplate::New(isolate, Path::RemovingLastComponent), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "remove").ToLocalChecked(), FunctionTemplate::New(isolate, Path::Remove), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        ctorProtoTpl->Set(String::NewFromUtf8(isolate, "createDir").ToLocalChecked(), FunctionTemplate::New(isolate, Path::CreateDir), static_cast<PropertyAttribute>(ReadOnly | DontEnum | DontDelete));
        
        // (new Path(...)).<prop>
        ctorInstTpl->SetAccessor(String::NewFromUtf8(isolate, "count").ToLocalChecked(), Path::Count, nullptr, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(ReadOnly | DontDelete | DontEnum));
        ctorInstTpl->SetAccessor(String::NewFromUtf8(isolate, "exists").ToLocalChecked(), Path::Exists, nullptr, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(ReadOnly | DontDelete | DontEnum));
        ctorInstTpl->SetAccessor(String::NewFromUtf8(isolate, "stat").ToLocalChecked(), Path::Stat, nullptr, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(ReadOnly | DontDelete | DontEnum));
        ctorInstTpl->SetAccessor(String::NewFromUtf8(isolate, "readable").ToLocalChecked(), Path::Readable, nullptr, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(ReadOnly | DontDelete | DontEnum));
        ctorInstTpl->SetAccessor(String::NewFromUtf8(isolate, "writable").ToLocalChecked(), Path::Writable, nullptr, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(ReadOnly | DontDelete | DontEnum));
        ctorInstTpl->SetAccessor(String::NewFromUtf8(isolate, "readableAndWritable").ToLocalChecked(), Path::ReadableAndWritable, nullptr, Local<Value>(), DEFAULT, static_cast<PropertyAttribute>(ReadOnly | DontDelete | DontEnum));
        
        Local<Function> constructor = ctorTpl->GetFunction(context).ToLocalChecked();
        dataObject->SetInternalField(0, constructor);
        exports->Set(context, String::NewFromUtf8(isolate, "Path").ToLocalChecked(), constructor).FromJust();
    }
    
    void Path::New(const FunctionCallbackInfo<Value> & args) {
        Isolate * isolate = args.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        if (args.IsConstructCall()) {
            plzma::Path path;
            if (args.Length() > 0) {
                bool valueDefined = false;
                NPLZMA_GET_PATH_FROM_VALUE(isolate, context, args[0], path, valueDefined)
                if (!valueDefined && args[0]->IsNullOrUndefined()) {
                    valueDefined = true;
                }
                if (!valueDefined) {
                    NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "Path(?)")
                }
            }
            Path * obj = nullptr;
            NPLZMA_TRY
            obj = new Path(std::move(path));
            NPLZMA_CATCH_RET(isolate)
            obj->TypedWrap(args.This());
            args.GetReturnValue().Set(args.This());
        } else {
            Local<Function> constructor = args.Data().As<Object>()->GetInternalField(0).As<Function>();
            int argc = 0;
            Local<Value> argv[1];
            if (args.Length() > 0) { argv[argc++] = args[0]; }
            TryCatch trycatch(isolate);
            MaybeLocal<Object> maybeResult = constructor->NewInstance(context, argc, argv);
            if (maybeResult.IsEmpty()) {
                if (trycatch.HasCaught()) {
                    trycatch.ReThrow();
                } else {
                    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Can't instantiate Path.").ToLocalChecked()));
                }
                return;
            }
            args.GetReturnValue().Set(maybeResult.ToLocalChecked());
        }
    }
    
    static void GetVersion(Local<Name> property, const PropertyCallbackInfo<Value>& info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        info.GetReturnValue().Set(String::NewFromUtf8(isolate, plzma_version()).ToLocalChecked());
    }
    
    static void GetGlobalUInt32Property(Local<Name> property, const PropertyCallbackInfo<Value>& info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        const uint32_t propertyNumber = info.Data()->ToUint32(context).ToLocalChecked()->Value();
        uint32_t retVal = 0;
        switch (propertyNumber) {
            case 1: retVal = plzma::kStreamReadSize; break;
            case 2: retVal = plzma::kStreamWriteSize; break;
            case 3: retVal = plzma::kDecoderReadSize; break;
            case 4: retVal = plzma::kDecoderWriteSize; break;
            default: break;
        }
        info.GetReturnValue().Set(Uint32::New(isolate, retVal));
    }
    
    static void SetGlobalUInt32Property(Local<Name> property, Local<Value> value, const PropertyCallbackInfo<void>& info) {
        Isolate * isolate = info.GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        const uint32_t propertyNumber = info.Data()->ToUint32(context).ToLocalChecked()->Value();
        uint32_t size = 0;
        bool sizeDefined = false;
        NPLZMA_GET_UINT32_FROM_VALUE(context, value, size, sizeDefined)
        if (sizeDefined) {
            switch (propertyNumber) {
                case 1: plzma::kStreamReadSize = size; break;
                case 2: plzma::kStreamWriteSize = size; break;
                case 3: plzma::kDecoderReadSize = size; break;
                case 4: plzma::kDecoderWriteSize = size; break;
                default: break;
            }
        } else {
            switch (propertyNumber) {
                case 1: { NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "streamReadSize") } break;
                case 2: { NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "streamWriteSize") } break;
                case 3: { NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "decoderReadSize") } break;
                case 4: { NPLZMA_THROW_ARG_TYPE_ERROR_RET(isolate, "decoderWriteSize") } break;
                default: break;
            }
        }
    }
    
    void InitConstsAndEnums(Local<Object> exports) {
        Isolate * isolate = exports->GetIsolate();
        HandleScope handleScope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        
        // plzma_error_code
        Local<Object> errorCodeObject = Object::New(isolate);
        errorCodeObject->DefineOwnProperty(context, String::NewFromUtf8(isolate, "unknown").ToLocalChecked(), Uint32::NewFromUnsigned(isolate, plzma_error_code_unknown), static_cast<PropertyAttribute>(ReadOnly | DontDelete)).Check();
        errorCodeObject->DefineOwnProperty(context, String::NewFromUtf8(isolate, "invalidArguments").ToLocalChecked(), Uint32::NewFromUnsigned(isolate, plzma_error_code_invalid_arguments), static_cast<PropertyAttribute>(ReadOnly | DontDelete)).Check();
        errorCodeObject->DefineOwnProperty(context, String::NewFromUtf8(isolate, "notEnoughMemory").ToLocalChecked(), Uint32::NewFromUnsigned(isolate, plzma_error_code_not_enough_memory), static_cast<PropertyAttribute>(ReadOnly | DontDelete)).Check();
        errorCodeObject->DefineOwnProperty(context, String::NewFromUtf8(isolate, "io").ToLocalChecked(), Uint32::NewFromUnsigned(isolate, plzma_error_code_io), static_cast<PropertyAttribute>(ReadOnly | DontDelete)).Check();
        errorCodeObject->DefineOwnProperty(context, String::NewFromUtf8(isolate, "internal").ToLocalChecked(), Uint32::NewFromUnsigned(isolate, plzma_error_code_internal), static_cast<PropertyAttribute>(ReadOnly | DontDelete)).Check();
        exports->Set(context, String::NewFromUtf8(isolate, "ErrorCode").ToLocalChecked(), errorCodeObject).FromJust();
        
        // plzma_erase
        Local<Object> eraseObject = Object::New(isolate);
        eraseObject->DefineOwnProperty(context, String::NewFromUtf8(isolate, "none").ToLocalChecked(), Uint32::NewFromUnsigned(isolate, plzma_erase_none), static_cast<PropertyAttribute>(ReadOnly | DontDelete)).Check();
        eraseObject->DefineOwnProperty(context, String::NewFromUtf8(isolate, "zero").ToLocalChecked(), Uint32::NewFromUnsigned(isolate, plzma_erase_zero), static_cast<PropertyAttribute>(ReadOnly | DontDelete)).Check();
        exports->Set(context, String::NewFromUtf8(isolate, "Erase").ToLocalChecked(), eraseObject).FromJust();
        
        // plzma_file_type
        Local<Object> fileTypeObject = Object::New(isolate);
        fileTypeObject->DefineOwnProperty(context, String::NewFromUtf8(isolate, "sevenZ").ToLocalChecked(), Uint32::NewFromUnsigned(isolate, plzma_file_type_7z), static_cast<PropertyAttribute>(ReadOnly | DontDelete)).Check();
        fileTypeObject->DefineOwnProperty(context, String::NewFromUtf8(isolate, "xz").ToLocalChecked(), Uint32::NewFromUnsigned(isolate, plzma_file_type_xz), static_cast<PropertyAttribute>(ReadOnly | DontDelete)).Check();
        fileTypeObject->DefineOwnProperty(context, String::NewFromUtf8(isolate, "tar").ToLocalChecked(), Uint32::NewFromUnsigned(isolate, plzma_file_type_tar), static_cast<PropertyAttribute>(ReadOnly | DontDelete)).Check();
        exports->Set(context, String::NewFromUtf8(isolate, "FileType").ToLocalChecked(), fileTypeObject).FromJust();
        
        // plzma_method
        Local<Object> methodObject = Object::New(isolate);
        methodObject->DefineOwnProperty(context, String::NewFromUtf8(isolate, "LZMA").ToLocalChecked(), Uint32::NewFromUnsigned(isolate, plzma_method_LZMA), static_cast<PropertyAttribute>(ReadOnly | DontDelete)).Check();
        methodObject->DefineOwnProperty(context, String::NewFromUtf8(isolate, "LZMA2").ToLocalChecked(), Uint32::NewFromUnsigned(isolate, plzma_method_LZMA2), static_cast<PropertyAttribute>(ReadOnly | DontDelete)).Check();
        methodObject->DefineOwnProperty(context, String::NewFromUtf8(isolate, "PPMd").ToLocalChecked(), Uint32::NewFromUnsigned(isolate, plzma_method_PPMd), static_cast<PropertyAttribute>(ReadOnly | DontDelete)).Check();
        exports->Set(context, String::NewFromUtf8(isolate, "Method").ToLocalChecked(), methodObject).FromJust();
        
        // plzma_open_dir_mode
        Local<Object> openDirModeObject = Object::New(isolate);
        openDirModeObject->DefineOwnProperty(context, String::NewFromUtf8(isolate, "followSymlinks").ToLocalChecked(), Uint32::NewFromUnsigned(isolate, plzma_open_dir_mode_follow_symlinks), static_cast<PropertyAttribute>(ReadOnly | DontDelete)).Check();
        exports->Set(context, String::NewFromUtf8(isolate, "OpenDirMode").ToLocalChecked(), openDirModeObject).FromJust();
        
        // plzma_plzma_multi_stream_part_name_format
        Local<Object> multiStreamPartNameFormatObject = Object::New(isolate);
        multiStreamPartNameFormatObject->DefineOwnProperty(context, String::NewFromUtf8(isolate, "nameExt00x").ToLocalChecked(), Uint32::NewFromUnsigned(isolate, plzma_plzma_multi_stream_part_name_format_name_ext_00x), static_cast<PropertyAttribute>(ReadOnly | DontDelete)).Check();
        exports->Set(context, String::NewFromUtf8(isolate, "MultiStreamPartNameFormat").ToLocalChecked(), multiStreamPartNameFormatObject).FromJust();
        
        // module properties
        exports->SetNativeDataProperty(context, String::NewFromUtf8(isolate, "version").ToLocalChecked(), GetVersion, nullptr, Local<Value>(), static_cast<PropertyAttribute>(ReadOnly | DontDelete)).Check();
        exports->SetNativeDataProperty(context, String::NewFromUtf8(isolate, "streamReadSize").ToLocalChecked(), GetGlobalUInt32Property, SetGlobalUInt32Property, Uint32::NewFromUnsigned(isolate, 1), static_cast<PropertyAttribute>(DontDelete)).Check();
        exports->SetNativeDataProperty(context, String::NewFromUtf8(isolate, "streamWriteSize").ToLocalChecked(), GetGlobalUInt32Property, SetGlobalUInt32Property, Uint32::NewFromUnsigned(isolate, 2), static_cast<PropertyAttribute>(DontDelete)).Check();
        exports->SetNativeDataProperty(context, String::NewFromUtf8(isolate, "decoderReadSize").ToLocalChecked(), GetGlobalUInt32Property, SetGlobalUInt32Property, Uint32::NewFromUnsigned(isolate, 3), static_cast<PropertyAttribute>(DontDelete)).Check();
        exports->SetNativeDataProperty(context, String::NewFromUtf8(isolate, "decoderWriteSize").ToLocalChecked(), GetGlobalUInt32Property, SetGlobalUInt32Property, Uint32::NewFromUnsigned(isolate, 4), static_cast<PropertyAttribute>(DontDelete)).Check();
    }
}

static void PlzmaModuleInit(Local<Object> exports) {
    nplzma::InitConstsAndEnums(exports);
    nplzma::Path::Init(exports);
    nplzma::Item::Init(exports);
    nplzma::InStream::Init(exports);
    nplzma::OutStream<plzma::OutStream>::Init(exports);
    nplzma::OutStream<plzma::OutMultiStream>::Init(exports);
    nplzma::Decoder::Init(exports);
    nplzma::Encoder::Init(exports);
}

NODE_MODULE(plzma, PlzmaModuleInit)
