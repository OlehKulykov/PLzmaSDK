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


#ifndef __LIBPLZMA_HPP__
#define __LIBPLZMA_HPP__ 1

#include <cstddef>
#include <cstdarg>
#include <cstdlib>

#include "libplzma.h"

/// @file libplzma.hpp
/// @brief Single public header of the Core C++ library part.
///
/// This header file uses types, preprocessor definitions and common C functions presented in a single public header of the C library(libplzma.h).
/// Everything what you need to use this library in C++ | Objective-C++ | Node.js env. is here.

namespace plzma {
    
    /// @brief The presentation of the allocated raw heap memory.
    ///
    /// Uses \a plzma_malloc, \a plzma_realloc and \a plzma_free functions for handling the raw heap memory.
    /// Following the \a Resource \a Acquisition \a Is \a Initialization or \a RAII technique.
    /// The allocated memory will be automatically deallocated via \a plzma_free function.
    struct LIBPLZMA_CPP_CLASS_API RawHeapMemory final {
    protected:
#if defined(DEBUG)
        union {
            void * LIBPLZMA_NULLABLE _memory = nullptr;
            char * LIBPLZMA_NULLABLE _cstring;
            wchar_t * LIBPLZMA_NULLABLE _wstring;
        };
#else
        void * LIBPLZMA_NULLABLE _memory = nullptr;
#endif
        RawHeapMemory & operator = (const RawHeapMemory &) = delete;
        RawHeapMemory(const RawHeapMemory &) = delete;
        
    public:
        /// @brief Moves allocated memory from \a memory instance.
        ///
        /// The previously allocated memory will be released via \a plzma_free function.
        RawHeapMemory & operator = (RawHeapMemory && memory) noexcept;
        
        
        /// @brief Checks the memory is allocated.
        /// @return \a true if memory allocated and exists, otherwise \a false.
        inline operator bool () const noexcept { return _memory != nullptr; }
        
        
        /// @return The memory pointer casted to a specific type via \a static_cast function.
        /// @tparam T The type of memory.
        template<typename T>
        inline operator T () noexcept { return static_cast<T>(_memory); }
        
        
        /// @return The memory pointer casted to a specific type via \a static_cast function.
        /// @tparam T The type of memory.
        template<typename T>
        inline operator T () const noexcept { return static_cast<T>(_memory); }
        
        
        /// @brief Returns raw pointer to the memory and nullifies the internal storage.
        void * LIBPLZMA_NULLABLE take() noexcept;
        
        
        /// @brief Changes the size of the raw heap memory.
        ///
        /// Uses \a plzma_realloc to change the size of existed memory or \a plzma_malloc to allocate new one.
        /// @param size The new size in bytes of memory. In case if \a size is zero, the allocated memory will be released via \a plzma_free function.
        /// @exception The \a Exception with \a plzma_error_code_not_enough_memory code in case if required amount of memory can't be allocated.
        void resize(const size_t size);
        
        
        /// @brief Erases the allocated memory.
        /// @param eraseType The type of the erasing of the memory. See \a plzma_erase enum.
        /// @param size The number bytes to erase.
        void erase(const plzma_erase eraseType, const size_t size) noexcept;
        
        
        /// @brief Erases and releases the allocated memory.
        /// @param eraseType The type of the erasing of the memory. See \a plzma_erase enum.
        /// @param size The number bytes to erase.
        void clear(const plzma_erase eraseType = plzma_erase_none, const size_t size = 0) noexcept;
        
        
        /// @brief Contructs the \a RawHeapMemory instance and allocates the memory if needed.
        ///
        /// Uses \a plzma_malloc function to allocate the memory.
        /// @param size The size in bytes of memory to allocate.
        /// @param eraseType The type of the erasing of newly allocated memory. See \a plzma_erase enum.
        /// @exception The \a Exception with \a plzma_error_code_not_enough_memory code in case if required amount of memory can't be allocated.
        RawHeapMemory(const size_t size, const plzma_erase eraseType = plzma_erase_none);
        
        
        /// @brief Contructs the \a RawHeapMemory instance and moves previously allocated memory
        /// from another \a RawHeapMemory instance.
        RawHeapMemory(RawHeapMemory && memory) noexcept;
        
        
        /// @brief Contructs empty \a RawHeapMemory instance without allocating memory.
        RawHeapMemory() noexcept = default;
        
        
        /// @brief Destroys the \a RawHeapMemory instance and releases the allocating memory via \a plzma_free function.
        ~RawHeapMemory() noexcept;
    };
    
    template<bool B, typename T = void>
    struct enableIf { };
    
    template <typename T>
    struct enableIf<true, T> { typedef T type; };
    
    template <bool B, class T = void>
    using enableIfT = typename enableIf<B, T>::type;
    
    /// @brief The struct template that provides a way to store 2 instances/objects as a single unit.
    /// Same as \a std::pair template.
    /// @tparam FIRST The type of the \a first instance/object.
    /// @tparam SECOND The type of the \a second instance/object.
    template<typename FIRST, typename SECOND, bool FIRST_COPYABLE = true>
    struct Pair final {
        FIRST first;
        SECOND second;
        
        Pair<FIRST, SECOND, FIRST_COPYABLE> & operator = (const Pair<FIRST, SECOND, FIRST_COPYABLE> &) = default;
        Pair<FIRST, SECOND, FIRST_COPYABLE> & operator = (Pair<FIRST, SECOND, FIRST_COPYABLE> && pair) noexcept {
            first = static_cast<FIRST &&>(pair.first);
            second = static_cast<SECOND &&>(pair.second);
            return *this;
        }
        template <typename T = void, typename = enableIfT<FIRST_COPYABLE, T> >
        Pair(const FIRST & f, const SECOND & s) : first(f), second(s) { }
        Pair(FIRST && f, SECOND && s) : first(static_cast<FIRST &&>(f)), second(static_cast<SECOND &&>(s)) { }
        Pair(Pair<FIRST, SECOND, FIRST_COPYABLE> && pair) noexcept : first(static_cast<FIRST &&>(pair.first)), second(static_cast<SECOND &&>(pair.second)) { }
        Pair(const Pair<FIRST, SECOND, FIRST_COPYABLE> &) = default;
        Pair() noexcept = default;
    };
    
    
    /// @brief The struct template that provides a way to store 3 instances/objects as a single unit.
    /// @tparam FIRST The type of the \a first instance/object.
    /// @tparam SECOND The type of the \a second instance/object.
    /// @tparam THIRD The type of the \a third instance/object.
    template<typename FIRST, typename SECOND, typename THIRD>
    struct Trio final {
        FIRST first;
        SECOND second;
        THIRD third;
        
        Trio<FIRST, SECOND, THIRD> & operator = (const Trio<FIRST, SECOND, THIRD> &) = default;
        Trio<FIRST, SECOND, THIRD> & operator = (Trio<FIRST, SECOND, THIRD> && trio) noexcept {
            first = static_cast<FIRST &&>(trio.first);
            second = static_cast<SECOND &&>(trio.second);
            third = static_cast<THIRD &&>(trio.third);
            return *this;
        }
        Trio(const FIRST & f, const SECOND & s, const THIRD & t) : first(f), second(s), third(t) { }
        Trio(FIRST && f, SECOND && s, THIRD && t) noexcept : first(static_cast<FIRST &&>(f)), second(static_cast<SECOND &&>(s)), third(static_cast<THIRD &&>(t)) { }

        Trio(Trio<FIRST, SECOND, THIRD> && trio) noexcept : first(static_cast<FIRST &&>(trio.first)), second(static_cast<SECOND &&>(trio.second)), third(static_cast<THIRD &&>(trio.third)) { }
        Trio(const Trio<FIRST, SECOND, THIRD> &) = default;
        Trio() noexcept = default;
    };
    
    
    /// @brief Shared pointer for \a Automatic \a Reference \a Counting \a (ARC) classes.
    /// Similar to the \a std::shared_ptr template, but not compatible due to underlying code-base.
    /// @tparam T The Class type which supports ARC, i.e. class which contains \a retain and \a release methods.
    template<class T>
    struct SharedPtr final {
    private:
        T * LIBPLZMA_NULLABLE _ptr = nullptr;
        
    public:
        /// @brief Simply casts the reference of \a T type to an another type. Uses \a static_cast function.
        /// @return The new shared pointer of \a R type which refers to the same +retained reference.
        template<class R>
        SharedPtr<R> cast() const {
            return SharedPtr<R>(static_cast<R *>(_ptr));
        }
        
        
        /// @return The const reference to the ARC class.
        const T * LIBPLZMA_NULLABLE get() const noexcept {
            return _ptr;
        }
        
        
        /// @return The reference to the ARC class.
        T * LIBPLZMA_NULLABLE get() noexcept {
            return _ptr;
        }
        
        
        /// @brief Takes the reference without changing the ownership and nullifies internal pointer.
        /// @return The reference to the ARC class.
        /// @note Retain counter keeps unchanged.
        T * LIBPLZMA_NULLABLE take() noexcept {
            T * ptr = _ptr;
            _ptr = nullptr;
            return ptr;
        }
        
        
        /// @brief Assigns the new reference without changing the ownership.
        /// The previous reference will be released.
        /// @note Retain counter of the argument \a ptr unchanged.
        void assign(T * LIBPLZMA_NULLABLE ptr) {
            if (_ptr != ptr && _ptr) {
                _ptr->release();
            }
            _ptr = ptr;
        }
        
        
        /// @brief Releases the reference and nullifies the internal storage.
        void clear() {
            if (_ptr) {
                _ptr->release();
                _ptr = nullptr;
            }
        }
        
        
        /// @return The reference to the ARC class.
        inline T * LIBPLZMA_NULLABLE operator -> () noexcept { return _ptr; }
        
        
        /// @return The const reference to the ARC class.
        inline const T * LIBPLZMA_NULLABLE operator -> () const noexcept { return _ptr; }
        
        
        /// @brief Checks the reference exists.
        inline operator bool () const noexcept { return _ptr != nullptr; }
        
        
        /// @brief Checks the reference does not exists.
        inline bool operator ! () const noexcept { return _ptr == nullptr; }
        
        
        /// @brief Moves the reference from an another shared pointer without changing the ownership.
        /// The previuos reference will be released.
        SharedPtr & operator = (SharedPtr && ptr) {
            T * prev = _ptr;
            _ptr = ptr._ptr;
            ptr._ptr = nullptr;
            if (_ptr != prev && prev) {
                prev->release();
            }
            return *this;
        }
        
        
        /// @brief Copies the reference from an another shared pointer with changing the ownership.
        /// The new reference will be retained.
        /// The previuos reference will be released.
        SharedPtr & operator = (const SharedPtr & ptr) {
            T * prev = _ptr;
            if ( (_ptr = ptr._ptr) ) {
                _ptr->retain();
            }
            if (prev) {
                prev->release();
            }
            return *this;
        }
        
        
        /// @brief Constructs the \a SharedPtr instance with another shared pointer instance.
        /// The ownership is moved, transferred from the \a ptr shared instance.
        SharedPtr(SharedPtr && ptr) noexcept : _ptr(ptr._ptr) {
            ptr._ptr = nullptr;
        }
        
        
        /// @brief Constructs the \a SharedPtr instance with another shared pointer instance.
        /// The ownership is shared between 2 instances, i.e. the existed reference is retained.
        SharedPtr(const SharedPtr & ptr) : _ptr(ptr._ptr) {
            if (_ptr) {
                _ptr->retain();
            }
        }
        
        
        /// @brief Constructs the \a SharedPtr instance with the reference to the ARC class.
        /// The reference is retained.
        SharedPtr(T * LIBPLZMA_NULLABLE ptr) : _ptr(ptr) {
            if (_ptr) {
                _ptr->retain();
            }
        }
        
        
        /// @brief Constructs the empty \a SharedPtr instance without any reference to the ARC class.
        SharedPtr() noexcept = default;
        
        
        /// @brief Destroys the \a SharedPtr instance and releases the existed reference to the ARC class.
        ~SharedPtr() {
            if (_ptr) {
                _ptr->release();
            }
        }
    };
    
    
    /// @brief Constructs a \a SharedPtr instance with movable arguments.
    /// Similar to the \a std::make_shared function.
    /// @tparam T The Class type which supports ARC
    /// @tparam ARGS The movable arguments for a contructor of the \a T ARC class.
    template<class T, class ... ARGS>
    SharedPtr<T> makeShared(ARGS && ... args) {
        return SharedPtr<T>(new T(static_cast<ARGS &&>(args)...));
    }
    
    
    /// @brief The exception class for all cases in the library.
    /// Similar to the \a std::exception, plus contains extended information.
    class LIBPLZMA_CPP_CLASS_API Exception final {
    private:
        mutable char * LIBPLZMA_NULLABLE _what = nullptr;
        mutable char * LIBPLZMA_NULLABLE _file = nullptr;
        mutable char * LIBPLZMA_NULLABLE _reason = nullptr;
        mutable plzma_error_code _code = plzma_error_code_unknown;
        mutable int _line = 0; // `int` cause of compiller's preprocessor definition type.
        
        Exception & operator = (const Exception &) = delete;
        Exception & operator = (Exception &&) = delete;
        Exception(const Exception &) = delete;
        
    public:
        /// @return Returns an explanatory string.
        const char * LIBPLZMA_NULLABLE what() const noexcept;
        
        
        /// @return Returns the file name in which the exception was thrown.
        const char * LIBPLZMA_NULLABLE file() const noexcept;
        
        
        /// @return Returns the reason why the exception was thrown.
        const char * LIBPLZMA_NULLABLE reason() const noexcept;
        
        
        /// @return Returns predefined code, type of the exception. See \a plzma_error_code enum.
        plzma_error_code code() const noexcept;
        
        
        /// @return Returns the line number of the file where exception was thrown.
        int line() const noexcept;
        
        
        /// @brief Changes the \a what property of the exception by concatinating all provided strings.
        LIBPLZMA_REQUIRES_LAST_NULL_ARG void setWhat(const char * LIBPLZMA_NULLABLE what = nullptr, ...) noexcept;
        
        
        /// @brief Changes the \a reason property of the exception by concatinating all provided strings.
        LIBPLZMA_REQUIRES_LAST_NULL_ARG void setReason(const char * LIBPLZMA_NULLABLE reason = nullptr, ...) noexcept;
        
        
        /// @brief Contructs the \a Exception instance with information from another one.
        Exception(Exception && exception) noexcept;
        
        
        /// @brief Contructs the \a Exception instance with \a code, \a what, \a file and \a line information.
        Exception(const plzma_error_code code,
                  const char * LIBPLZMA_NULLABLE what,
                  const char * LIBPLZMA_NULLABLE file,
                  const int line) noexcept;
        
        
        /// @brief Contructs empty \a Exception instance.
        Exception() noexcept = default;
        
        ~Exception() noexcept;
        
        
        /// @brief Creates new heap copy of the exception and moves all info from a source instance.
        /// The operator \a new will be used.
        /// @return The new heap copy or \a nullptr if the exception instance can't be created.
        Exception * LIBPLZMA_NULLABLE moveToHeapCopy() const noexcept;
        
        
        /// @brief Creates new heap copy of the exception with \a code, \a what, \a file and \a line information.
        /// The operator \a new will be used.
        /// @return The new heap copy or \a nullptr if the exception instance can't be created.
        static Exception * LIBPLZMA_NULLABLE create(const plzma_error_code code,
                                                    const char * LIBPLZMA_NULLABLE what,
                                                    const char * LIBPLZMA_NULLABLE file,
                                                    const int line) noexcept;
    };
    
    template struct LIBPLZMA_CPP_CLASS_API Pair<size_t, size_t>;
    
    /// @brief The class \a String stores and manipulates sequences of the UTF-8 and wide character strings.
    ///
    /// During the lifetime, the string might be updated with the UTF-8 or wide character string arguments,
    /// but internally stores only one type of characters, i.e. UTF-8 or wide.
    /// That's why, the conversion between UTF-8 <-> wide(16|32 bit) might take place.
    struct LIBPLZMA_CPP_CLASS_API String {
    protected:
        mutable RawHeapMemory _cs;
        mutable RawHeapMemory _ws;
        mutable plzma_size_t _cslen = 0;
        plzma_size_t _size = 0;
        
        void syncWide() const;
        void syncUtf8() const;
        void moveFrom(String && str, const plzma_erase eraseType = plzma_erase_none) noexcept;
        void copyFrom(const String & str, const plzma_erase eraseType = plzma_erase_none);
        void copyFrom(const wchar_t * LIBPLZMA_NULLABLE str, const plzma_erase eraseType = plzma_erase_none);
        void copyFrom(const char * LIBPLZMA_NULLABLE str, const plzma_erase eraseType = plzma_erase_none);
        void append(const wchar_t * LIBPLZMA_NONNULL * LIBPLZMA_NONNULL stringsList,
                    const Pair<size_t, size_t> * LIBPLZMA_NONNULL sizesList,
                    const size_t count,
                    const plzma_erase eraseType = plzma_erase_none);
        void append(const char * LIBPLZMA_NONNULL * LIBPLZMA_NONNULL stringsList,
                    const Pair<size_t, size_t> * LIBPLZMA_NONNULL sizesList,
                    const size_t count,
                    const plzma_erase eraseType = plzma_erase_none);
    public:
        /// @return The UTF-8, null-terminated charater string.
        /// @note If the internal string presentation is wide, then the wide chars will be converted to UTF-8 and returned.
        /// If the \a count of characters is zero, then the \a plzma_empty_cstring will be returned.
        /// @exception The \a Exception with \a plzma_error_code_internal code in case of conversion error or any nested.
        const char * LIBPLZMA_NONNULL utf8() const;
        
        
        /// @return The wide, null-terminated charater string.
        /// @note If the internal string presentation is UTF-8, then the UTF-8 chars will be converted to wide and returned.
        /// If the \a count of characters is zero, then the \a plzma_empty_wstring will be returned.
        /// @exception The \a Exception with \a plzma_error_code_internal code in case of conversion error or any nested.
        const wchar_t * LIBPLZMA_NONNULL wide() const;
        
        
        /// @return The number of unicode characters in a string.
        plzma_size_t count() const noexcept;
        
        
        /// @brief Clears the string. Optionaly, the string's container might be erased.
        /// @param eraseType The type of the erasing the string's content before releasing. See \a plzma_erase enum.
        void clear(const plzma_erase eraseType = plzma_erase_none) noexcept;
        
        
        /// @brief Copies the content of another string instance.
        virtual void set(const String & str);
        
        
        /// @brief Copies the content of the wide, null-terminated character \a str string reference.
        virtual void set(const wchar_t * LIBPLZMA_NULLABLE str);
        
        
        /// @brief Copies the content of the UTF-8, null-terminated character \a str string reference.
        virtual void set(const char * LIBPLZMA_NULLABLE str);
        
        
        /// @brief Appends string with the content of the wide, null-terminated character \a str string reference.
        virtual void append(const wchar_t * LIBPLZMA_NULLABLE str);
        
        
        /// @brief Appends string with the content of the UTF-8, null-terminated character \a str string reference.
        virtual void append(const char * LIBPLZMA_NULLABLE str);
        
        
        /// @brief Moves the content of the \a str string.
        String & operator = (String && str) noexcept;
        
        
        /// @brief Copies the content of another string instance.
        String & operator = (const String & str);
        
        
        /// @brief Constructs the \a String instance and moves the content from the \a str string.
        String(String && str) noexcept;
        
        
        /// @brief Constructs the \a String instance and copies the content of the \a str string.
        String(const String & str);
        
        
        /// @brief Constructs the \a String instance and copies the content of the \a str wide, null-terminated character string.
        String(const wchar_t * LIBPLZMA_NULLABLE str);
        
        
        /// @brief Constructs the \a String instance and copies the content of the \a str UTF-8, null-terminated character string.
        String(const char * LIBPLZMA_NULLABLE str);
        
        
        /// @brief Constructs the empty \a String instance.
        String() noexcept = default;
        virtual ~String() noexcept = default;
        
        
        /// @return The number of bytes/characters after the control character.
        /// @link https://tools.ietf.org/html/rfc3629#section-3
        static size_t utf8TrailingBytes(const uint8_t c) noexcept;
        
        
        /// @brief Calculates the length and number of unicode characters of the UTF-8 string limited to the maximum number of unicode characters.
        /// @param str The UTF-8, null-terminated character string.
        /// @param maxSize The maximum number of unicode characters.
        /// @return The pair: <length, number of unicode characters>
        static Pair<size_t, size_t> lengthMaxCount(const char * LIBPLZMA_NULLABLE str, const size_t maxSize) noexcept;
        
        
        /// @brief Calculates the length and number of unicode characters of the UTF-8 string limited to the maximum C-length.
        /// @param str The UTF-8, null-terminated character string.
        /// @param maxLength The maximum length of the C-string.
        /// @return The pair: <length, number of unicode characters>
        static Pair<size_t, size_t> lengthMaxLength(const char * LIBPLZMA_NULLABLE str, const size_t maxLength) noexcept;
    };
    
    
    /// @brief The \a Path class stores path's string presentation.
    ///
    /// During the manipulation of the path's string presentation:
    /// @li 1. The path separator will be automatically replaced with the platform specific one.
    /// @li 2. The previous content will be erased with \a plzma_erase_zero, i.e. zero-filled.
    struct LIBPLZMA_CPP_CLASS_API Path final : public String {
    public:
        class Iterator;
        virtual void set(const String & str) override final;
        virtual void set(const wchar_t * LIBPLZMA_NULLABLE str) override final;
        virtual void set(const char * LIBPLZMA_NULLABLE str) override final;
        virtual void append(const wchar_t * LIBPLZMA_NULLABLE str) override final;
        virtual void append(const char * LIBPLZMA_NULLABLE str) override final;
        
        void append(const Path & path);
        Path appending(const wchar_t * LIBPLZMA_NULLABLE str) const;
        Path appending(const char * LIBPLZMA_NULLABLE str) const;
        Path appending(const Path & path) const;
        
        
        /// @brief Appends the random component to the path.
        ///
        /// If path successfully updated, then the updated path doesn't exists in a root directory.
        /// The component consists of ASCII characters in range ['a'; 'z'].
        /// @exception The \a Exception with \a plzma_error_code_internal code in case if path can't be appended.
        void appendRandomComponent();
        
        
        /// @return New path with new random component.
        /// @see \a appendRandomComponent method.
        Path appendingRandomComponent() const;
        
        
        /// @brief Receives the last component of the path.
        Path lastComponent() const;
        
        
        /// @brief Removes the last component from the path.
        void removeLastComponent();
        
        
        /// @return New path by without last component.
        /// @see \a removeLastComponent method.
        Path removingLastComponent() const;
        
        
        /// @brief Checks the path exists with optional checking for a directory.
        /// @param isDir The optional pointer to a boolean variable to store the result of checking for a directory.
        /// @return \a true if path exists, otherwise \a false.
        bool exists(bool * LIBPLZMA_NULLABLE isDir = nullptr) const;
        
        
        /// @brief Checks the path exists and has read permissions.
        /// @return \a true if path exists and readable, otherwise \a false.
        bool readable() const;
        
        
        /// @brief Checks the path exists and has write permissions.
        /// @return \a true if path exists and writable, otherwise \a false.
        bool writable() const;
        
        
        /// @brief Checks the path exists and has read-write permissions.
        /// @return \a true if path exists, readable and writable, otherwise \a false.
        bool readableAndWritable() const;
        
        
        /// @brief Provides the stat info of the path.
        /// @return The stat info of the path or empty/zero-filled struct if operation was failed.
        plzma_path_stat stat() const;
        
        
        /// @brief Physically removes the directory with all content or file associated with the path.
        /// @param skipErrors Skips errors and continuing removing or stop on first error.
        /// @return The result of removing directory or file.
        bool remove(const bool skipErrors = false) const;
        
        
        /// @brief Creates the directory at specific path.
        /// @param withIntermediates Create intermediate directories for each component or not.
        /// @return The creation result of the directory.
        bool createDir(const bool withIntermediates) const;
        
        
        /// @brief Opens a file associated with path.
        /// @param mode The open file mode string. For Windows, it's possible to provide encoded character set to use(one of UTF-8, UTF-16LE, or UNICODE).
        /// @return The file reference or nullptr.
        /// @see \a fopen C function.
        FILE * LIBPLZMA_NULLABLE openFile(const char * LIBPLZMA_NONNULL mode) const;
        
        
        /// @brief Opens a directory associated with path for iterating the content.
        /// @param mode The open directory mode.
        /// @return The \a SharedPtr of the path iterator.
        /// @exception The \a Exception with \a plzma_error_code_io code in case if a directory can't be opened.
        SharedPtr<Iterator> openDir(const plzma_open_dir_mode_t mode = 0) const;
        
        
        bool operator == (const Path & path) const;
        Path & operator = (Path && path) noexcept;
        Path(Path && path) noexcept;
        Path & operator = (const Path & path);
        Path(const Path & path);
        Path(const wchar_t * LIBPLZMA_NULLABLE path);
        Path(const char * LIBPLZMA_NULLABLE path);
        Path() noexcept : String() { }
        
        /// @note For a security reasons, the content will be erased with zeros, i.e. '.erase(plzma_erase_zero)'.
        virtual ~Path() noexcept;
        
        /// @brief Provides the path with the platform specific temporary directory for the library.
        /// The provided directory path, if such exists, has a read-write permissions.
        /// @return The path with existed temporary directory or empty path.
        static Path tmpPath();
    };
    
    
    /// @brief Interface to a platform specific directory path iterator.
    class LIBPLZMA_CPP_CLASS_API Path::Iterator {
    private:
        friend struct SharedPtr<Path::Iterator>;
        virtual void retain() noexcept = 0;
        virtual void release() noexcept = 0;
        
    protected:
        Path _root;
        Path _path;
        Path _component;
        uint16_t _flags = 0;
        plzma_open_dir_mode_t _mode = 0;
        
        void clearPaths() noexcept;
        Iterator() = delete;
        Iterator(const Path & root, const plzma_open_dir_mode_t mode);
        virtual ~Iterator() noexcept = default;
        
    public:
        /// @brief Recevies the current file or directory component.
        const Path & component() const noexcept;
        
        
        /// @brief Recevies the current file or directory path.
        Path path() const;
        
        
        /// @brief Recevies the current file or directory full path, prefixed with root path.
        Path fullPath() const;
        
        
        /// @brief Checks the current iterator's path is directory.
        /// @return \a true the iterator's path is directory.
        bool isDir() const noexcept;
        
        
        /// @brief Continue iteration.
        /// @return \a true The next file or directory located, otherwise iteration is finished.
        virtual bool next() = 0;
        
        
        /// @brief Closes iteration and all open directory descriptors/handlers.
        virtual void close() noexcept = 0;
    };
    
    template struct LIBPLZMA_CPP_CLASS_API SharedPtr<Path::Iterator>;
    
    /// @brief The archive item.
    class LIBPLZMA_CPP_CLASS_API Item final {
    private:
        friend struct SharedPtr<Item>;
        Path _path;
        uint64_t _size = 0;
        uint64_t _packSize = 0;
        time_t _creationTime = 0;
        time_t _accessTime = 0;
        time_t _modificationTime = 0;
        uint32_t _crc32 = 0;
        plzma_size_t _index = 0;
        plzma_size_t _referenceCounter = 0;
        bool _encrypted = false;
        bool _isDir = false;
        
        void retain() noexcept;
        void release() noexcept;
        
        Item(Item &&) = delete;
        Item & operator = (Item &&) = delete;
        Item & operator = (const Item &) = delete;
        Item(const Item &) = delete;
        Item() = delete;
        
    public:
        /// @return Receives the item's path inside the archive.
        const Path & path() const noexcept;
        
        
        /// @return Receives the item's index inside the archive.
        plzma_size_t index() const noexcept;
        
        
        /// @return Receives the size in bytes of the item.
        uint64_t size() const noexcept;
        
        
        /// @return Receives the packed size in bytes of the item.
        uint64_t packSize() const noexcept;
        
        
        /// @return Receives the CRC-32 checksum of the item's content.
        uint32_t crc32() const noexcept;
        
        
        /// @return The creation time of the item. Unix timestamp.
        time_t creationTime() const noexcept;
        
        
        /// @return The last access time of the item. Unix timestamp.
        time_t accessTime() const noexcept;
        
        
        /// @return The last modification time of the item. Unix timestamp.
        time_t modificationTime() const noexcept;
        
        
        /// @return Checks the item is encrypted.
        bool encrypted() const noexcept;
        
        
        /// @return Checks the item is directory or file.
        bool isDir() const noexcept;
        
        
        /// @brief Updates the size of the item.
        /// @param size The size in bytes.
        void setSize(const uint64_t size) noexcept;
        
        
        /// @brief Updates the packed size of the item.
        /// @param size The size in bytes.
        void setPackSize(const uint64_t size) noexcept;
        
        
        /// @brief Updates the CRC-32 checksum of the item.
        /// @param crc The CRC-32 checksum value.
        void setCrc32(const uint32_t crc) noexcept;
        
        
        /// @brief Updates creation time of the item.
        /// @param time The unix timestamp.
        void setCreationTime(const time_t time) noexcept;
        
        
        /// @brief Updates last access time of the item.
        /// @param time The unix timestamp.
        void setAccessTime(const time_t time) noexcept;
        
        
        /// @brief Updates modification time of the item.
        /// @param time The unix timestamp.
        void setModificationTime(const time_t time) noexcept;
        
        
        /// @brief Marks the item is encrypted.
        void setEncrypted(const bool encrypted) noexcept;
        
        
        /// @brief Marks the item is directory.
        void setIsDir(const bool dir) noexcept;
        
        
        /// @brief Constructs the \a Item instance with path and index in the archive.
        /// @param path The associated item's path.
        /// @param index The index of the item in the archive.
        Item(const Path & path, const plzma_size_t index);
        
        
        /// @brief Constructs the \a Item instance with movable path and index in the archive.
        /// @param path The associated item's path. After the successfull creation of the item, the path is empty.
        /// @param index The index of the item in the archive.
        Item(Path && path, const plzma_size_t index) noexcept;
    };
    
    template struct LIBPLZMA_CPP_CLASS_API SharedPtr<Item>;
    template struct LIBPLZMA_CPP_CLASS_API Pair<void *, size_t>;
    
    /// @brief Interface to the input file stream.
    class InStream {
    private:
        friend struct SharedPtr<InStream>;
        virtual void retain() = 0;
        virtual void release() = 0;
        
    protected:
        virtual void * LIBPLZMA_NONNULL base() noexcept = 0;
        virtual ~InStream() noexcept = default;
        
    public:
        /// @brief Checks the input file stream is opened.
        /// @note Thread-safe.
        virtual bool opened() const = 0;
        
        
        /// @brief Erases and removes the content of the stream.
        /// @param eraseType The type of erasing the content.
        /// @return The erasing result.
        /// @note Thread-safe.
        virtual bool erase(const plzma_erase eraseType = plzma_erase_none) = 0;
    };
    
    template struct LIBPLZMA_CPP_CLASS_API SharedPtr<InStream>;
    
    /// @brief Creates the input file stream with path.
    /// @param path The non-empty input file path.
    /// @return The shared pointer with input file stream.
    /// @exception The \a Exception with \a plzma_error_code_invalid_arguments code in case if path is empty.
    LIBPLZMA_CPP_API(SharedPtr<InStream>) makeSharedInStream(const Path & path);
    
    
    /// @brief Creates the input file stream with path.
    /// @param path The movable non-empty input file path.
    /// @return The shared pointer with input file stream.
    /// @exception The \a Exception with \a plzma_error_code_invalid_arguments code in case if path is empty.
    LIBPLZMA_CPP_API(SharedPtr<InStream>) makeSharedInStream(Path && path);
    
    
    /// @brief Creates the input file stream with the file memory content.
    /// During the creation, the memory will copyed.
    /// @param memory The file memory content.
    /// @param size The memory size in bytes.
    /// @return The shared pointer with input file stream.
    /// @exception The \a Exception with \a plzma_error_code_not_enough_memory code in case if can't allocate required size of memory.
    /// @exception The \a Exception with \a plzma_error_code_invalid_arguments code in case if memory/size is empty.
    LIBPLZMA_CPP_API(SharedPtr<InStream>) makeSharedInStream(const void * LIBPLZMA_NONNULL memory, const size_t size);
    
    
    /// @brief Creates the input file stream with the file memory content.
    /// During the creation, the memory will not be copyed.
    /// @param memory The file memory content.
    /// @param size The memory size in bytes.
    /// @param freeCallback The callback, which will be triggered with provided \a memory pointer at the end of stream's lifetime.
    /// @return The shared pointer with input file stream.
    /// @exception The \a Exception with \a plzma_error_code_invalid_arguments code in case if memory/size/freeCallback is empty.
    LIBPLZMA_CPP_API(SharedPtr<InStream>) makeSharedInStream(void * LIBPLZMA_NONNULL memory, const size_t size, plzma_free_callback LIBPLZMA_NONNULL freeCallback);
    
    
    /// @brief Creates the input file stream with user defined callbacks.
    /// @param openCallback Opens the file stream for reading. Similar to \a fopen C function.
    /// @param closeCallback Closes the file stream. Similar to \a fclose C function.
    /// @param seekCallback Sets the read offset of the stream. Similar to \a fseek C function.
    /// @param readCallback Reads the number of bytes into provided byffer. Similar to \a fread C function.
    /// @param context The user defined context provided to all callbacks.
    /// @return The shared pointer with input file stream.
    /// @exception The \a Exception with \a plzma_error_code_invalid_arguments code in case if not all callbacks are provided.
    LIBPLZMA_CPP_API(SharedPtr<InStream>) makeSharedInStream(plzma_in_stream_open_callback LIBPLZMA_NONNULL openCallback,
                                                             plzma_in_stream_close_callback LIBPLZMA_NONNULL closeCallback,
                                                             plzma_in_stream_seek_callback LIBPLZMA_NONNULL seekCallback,
                                                             plzma_in_stream_read_callback LIBPLZMA_NONNULL readCallback,
                                                             const plzma_context context = plzma_context{nullptr, nullptr}); // C2059 = { .context = nullptr, .deinitializer = nullptr }

    template<typename T>
    class Vector;

    typedef Vector<SharedPtr<InStream> > InStreamArray;

    /// @brief Creates the multi input stream with a list of input streams.
    /// The list should not be empty. The order: file.001, file.002, ..., file.XXX
    /// @param streams The non-empty list of input streams. Each stream inside the list should also exist.
    /// @return The shared pointer with input file stream.
    /// @exception The \a Exception with \a plzma_error_code_invalid_arguments code in case if streams list is empty or contains empty stream.
    LIBPLZMA_CPP_API(SharedPtr<InStream>) makeSharedInStream(InStreamArray && streams);

    template struct LIBPLZMA_CPP_CLASS_API Pair<RawHeapMemory, size_t, false>;
    typedef Pair<RawHeapMemory, size_t, false> RawHeapMemorySize;
    
    /// @brief Interface to the output file or memory stream.
    class OutStream {
    protected:
        friend struct SharedPtr<OutStream>;
        virtual void retain() = 0;
        virtual void release() = 0;
        
        virtual void * LIBPLZMA_NONNULL base() noexcept = 0;
        virtual ~OutStream() noexcept = default;
        
    public:
        /// @return Checks the output file stream is opened.
        /// @note Thread-safe.
        virtual bool opened() const = 0;
        
        
        /// @brief Erases and removes the content of the stream.
        /// @param eraseType The type of erasing the content.
        /// @return The erasing result.
        /// @note Thread-safe.
        virtual bool erase(const plzma_erase eraseType = plzma_erase_none) = 0;
        
        
        /// @brief Copies the content of the stream to a heap memory.
        ///
        /// The stream must be closed.
        /// @return The pair with newly allocated heap memory with the stream's content.
        /// @exception The \a Exception with \a plzma_error_code_not_enough_memory code in case if required amount of memory can't be allocated.
        /// @note Thread-safe.
        virtual RawHeapMemorySize copyContent() const = 0;
    };
    
    template struct LIBPLZMA_CPP_CLASS_API SharedPtr<OutStream>;
    
    /// @brief Creates the output file stream with path.
    /// @param path The non-empty output file path.
    /// @return The output file stream.
    /// @exception The \a Exception with \a plzma_error_code_invalid_arguments code in case if path is empty.
    LIBPLZMA_CPP_API(SharedPtr<OutStream>) makeSharedOutStream(const Path & path);
    
    
    /// @brief Creates the output file stream object with movable path.
    /// @param path The non-empty output file path. After the successfull creation of the stream, the path is empty.
    /// @return The output file stream.
    /// @exception The \a Exception with \a plzma_error_code_invalid_arguments code in case if path is empty.
    LIBPLZMA_CPP_API(SharedPtr<OutStream>) makeSharedOutStream(Path && path);
    
    
    /// @brief Creates the output file stream object for writing to memory.
    /// @return The output file stream.
    LIBPLZMA_CPP_API(SharedPtr<OutStream>) makeSharedOutStream(void);
    
    typedef Vector<SharedPtr<OutStream> > OutStreamArray;

    /// @brief Interface to the output multi volume/part stream.
    class OutMultiStream : public OutStream {
    private:
        friend struct SharedPtr<OutMultiStream>;
    
    protected:
        virtual ~OutMultiStream() noexcept = default;
    
    public:
        /// @return The list of created sub-streams. The stream must be closed.
        ///         If stream is opened, then the list is empty.
        virtual OutStreamArray streams() const = 0;
    };

    template struct LIBPLZMA_CPP_CLASS_API SharedPtr<OutMultiStream>;

    /// @brief Creates the output multi stream with directory path, part name, extension, format and part size.
    /// All sub-streams are file streams.
    /// @param dirPath The non-empty output directory path.
    /// @param partName The non-empty output file name.
    /// @param partExtension Optional extension.
    /// @param format Format of the result file name part.
    /// @param partSize The maximum size in bytes of each out file sub-stream.
    /// If the number of file parts/sub-streams will exceed the maximum for a \a format, then the runtime exception will be thrown.
    /// @return The output multi stream.
    /// @exception The \a Exception with \a plzma_error_code_io code in case if path doesn't exist and stream can't create new one.
    /// @exception The \a Exception with \a plzma_error_code_invalid_arguments code in case if path is empty.
    /// @exception The \a Exception with \a plzma_error_code_invalid_arguments code in case if path is not a directory or there are no write permissions.
    LIBPLZMA_CPP_API(SharedPtr<OutMultiStream>) makeSharedOutMultiStream(const Path & dirPath,
                                                                         const String & partName,
                                                                         const String & partExtension,
                                                                         const plzma_plzma_multi_stream_part_name_format format,
                                                                         const plzma_size_t partSize);


    /// @brief Creates the output multi stream with movable directory path, part name, extension, format and part size.
    /// All sub-streams are file streams.
    /// @param dirPath The non-empty output directory path.
    /// @param partName The non-empty output file name.
    /// @param partExtension Optional extension.
    /// @param format Format of the result file name part.
    /// @param partSize The maximum size in bytes of each out file sub-stream.
    /// If the number of file parts/sub-streams will exceed the maximum for a \a format, then the runtime exception will be thrown.
    /// @return The output multi stream.
    /// @exception The \a Exception with \a plzma_error_code_io code in case if path doesn't exist and stream can't create new one.
    /// @exception The \a Exception with \a plzma_error_code_invalid_arguments code in case if path is empty.
    /// @exception The \a Exception with \a plzma_error_code_invalid_arguments code in case if path is not a directory or there are no write permissions.
    LIBPLZMA_CPP_API(SharedPtr<OutMultiStream>) makeSharedOutMultiStream(Path && dirPath,
                                                                         String && partName,
                                                                         String && partExtension,
                                                                         const plzma_plzma_multi_stream_part_name_format format,
                                                                         const plzma_size_t partSize);


    /// @brief Creates the output multi stream object for writing to memory.
    /// All sub-streams are memory streams.
    /// @param partSize The maximum size in bytes of each out memory sub-stream.
    /// @return The output multi stream.
    LIBPLZMA_CPP_API(SharedPtr<OutMultiStream>) makeSharedOutMultiStream(const plzma_size_t partSize);

    //constexpr bool isPowerOf2(const size_t v) {
    //    return v && ((v & (v - 1)) == 0);
    //}
    
    /// @brief The primary template of the sorting comparator.
    template<typename T>
    struct SortComparator {
        static int comparator(const void * LIBPLZMA_NONNULL elementA, const void * LIBPLZMA_NONNULL elementB) noexcept {
            return 0;
        }
    };
    
    
    /// @brief The template specialization of the sorting comparator for a single shared pointer of the \a Item.
    /// Sorts by the item's index in the archive.
    template<>
    struct SortComparator<SharedPtr<Item> > {
        static int comparator(const void * LIBPLZMA_NONNULL elementA, const void * LIBPLZMA_NONNULL elementB) noexcept {
            const plzma_size_t indexA = (*static_cast<const SharedPtr<Item> *>(elementA))->index();
            const plzma_size_t indexB = (*static_cast<const SharedPtr<Item> *>(elementB))->index();
            return (indexA < indexB) ? -1 : ((indexA > indexB) ? 1 : 0);
        }
    };
    
    template struct LIBPLZMA_CPP_CLASS_API Pair<SharedPtr<Item>, SharedPtr<OutStream> >;
    
    /// @brief The template specialization of the sorting comparator
    /// for a pair of shared pointer of the \a Item and shared pointer of the \a OutStream.
    /// Sorts by the item's index in the archive.
    template<>
    struct SortComparator<Pair<SharedPtr<Item>, SharedPtr<OutStream> > > {
        static int comparator(const void * LIBPLZMA_NONNULL elementA, const void * LIBPLZMA_NONNULL elementB) noexcept {
            const plzma_size_t indexA = static_cast<const Pair<SharedPtr<Item>, SharedPtr<OutStream> > *>(elementA)->first->index();
            const plzma_size_t indexB = static_cast<const Pair<SharedPtr<Item>, SharedPtr<OutStream> > *>(elementB)->first->index();
            return (indexA < indexB) ? -1 : ((indexA > indexB) ? 1 : 0);
        }
    };
    
    
    /// @brief The primary template of the binary search comparator.
    template<typename T, typename K>
    struct BSearchComparator {
        static int comparator(const void * LIBPLZMA_NONNULL key, const void * LIBPLZMA_NONNULL element) noexcept {
            return 0;
        }
    };
    
    
    /// @brief The template specialization of the binary search comparator
    /// for a pair of shared pointer of the \a Item and shared pointer of the \a OutStream.
    template<>
    struct BSearchComparator<Pair<SharedPtr<Item>, SharedPtr<OutStream> >, plzma_size_t> {
        static int comparator(const void * LIBPLZMA_NONNULL keyPtr, const void * LIBPLZMA_NONNULL element) noexcept {
            const plzma_size_t key = *static_cast<const plzma_size_t *>(keyPtr);
            const plzma_size_t index = static_cast<const Pair<SharedPtr<Item>, SharedPtr<OutStream> > *>(element)->first->index();
            return (key < index) ? -1 : ((key > index) ? 1 : 0);
        }
    };
    
    /// @brief The template of vector.
    /// Similar to the \a std::vector.
    /// @tparam T Class type with move constructor and move assignment operator.
    template<typename T>
    class Vector {
    private:
        friend struct SharedPtr<Vector<T> >;
        
        void retain() noexcept {
            _referenceCounter++;
        }
        
        void release() noexcept {
            if (--_referenceCounter > 0) {
                return;
            }
            delete this;
        }
        
        void extendCapacity(const size_t capacityIncrement = 10) {
            const size_t dstCapacity = capacityIncrement + _capacity;
            if (dstCapacity > PLZMA_SIZE_T_MAX) {
                throw Exception(plzma_error_code_invalid_arguments, "Exceeded vector capacity.", __FILE__, __LINE__);
            }
            T * mem = new T[dstCapacity]();
            try {
                for (size_t i = 0; i < _size; i++) {
                    mem[i] = static_cast<T &&>(_mem[i]);
                }
                delete [] _mem;
            } catch (...) {
                delete [] mem;
                throw;
            }
            _mem = mem;
            _capacity = static_cast<plzma_size_t>(dstCapacity);
        }
        
        Vector & operator = (const Vector<T> &) = delete;
        Vector(const Vector<T> &) = delete;
        
    protected:
        T * LIBPLZMA_NULLABLE _mem = nullptr;
        plzma_size_t _size = 0;
        plzma_size_t _capacity = 0;
        plzma_size_t _referenceCounter = 0;
        
    public:
        typedef T ElementType;
        typedef Vector<T> VectorType;
        
        plzma_size_t count() const noexcept { return _size; }
        
        const T & at(const plzma_size_t index) const { return _mem[index]; }
        T & at(const plzma_size_t index) { return _mem[index]; }
        
        void push(const T & element) {
            if (_capacity == _size) {
                extendCapacity();
            }
            _mem[_size++] = element;
        }
        
        void push(T && element) {
            if (_capacity == _size) {
                extendCapacity();
            }
            _mem[_size++] = static_cast<T &&>(element);
        }
        
        void pop() noexcept {
            if (_size > 0) {
                T tmp = static_cast<T &&>(_mem[--_size]); // &&/move noexcept
                (void)tmp;
            }
        }
        
        void clear() noexcept {
            delete [] _mem;
            _mem = nullptr;
            _size = _capacity = 0;
        }
        
        void sort() noexcept {
            if (_size > 1) {
                ::qsort(_mem, _size, sizeof(T), SortComparator<T>::comparator);
            }
        }
        
        template<typename K>
        T * LIBPLZMA_NULLABLE bsearch(K key) noexcept {
            return (_size > 0) ? static_cast<T *>(::bsearch(&key, _mem, _size, sizeof(T), BSearchComparator<T, K>::comparator)) : nullptr;
        }
        
        Vector & operator = (Vector<T> && vector) noexcept {
            delete [] _mem;
            _mem = vector._mem;
            _size = vector._size;
            _capacity = vector._capacity;
            vector._mem = nullptr;
            vector._size = vector._capacity = 0;
            return *this;
        }
        
        /// Ignore default exceptional constructor, using implemented 'noexcept' one.
        Vector(Vector<T> && vector) noexcept : _mem(vector._mem), _size(vector._size), _capacity(vector._capacity) {
            vector._mem = nullptr;
            vector._size = vector._capacity = 0;
        }
        
        Vector(const plzma_size_t capacity = 0) {
            if (capacity > 0) {
                extendCapacity(capacity);
                _capacity = capacity;
            }
        }
        
        ~Vector() noexcept {
            delete [] _mem;
        }
    };
    
    template class LIBPLZMA_CPP_CLASS_API Vector<SharedPtr<Item> >;
    template struct LIBPLZMA_CPP_CLASS_API SharedPtr<Vector<SharedPtr<Item> > >;
    typedef Vector<SharedPtr<Item> > ItemArray;
    
    template struct LIBPLZMA_CPP_CLASS_API SharedPtr<Vector<Pair<SharedPtr<Item>, SharedPtr<OutStream> > > >;
    template class LIBPLZMA_CPP_CLASS_API Vector<Pair<SharedPtr<Item>, SharedPtr<OutStream> > >;
    typedef Vector<Pair<SharedPtr<Item>, SharedPtr<OutStream> > > ItemOutStreamArray;
    
    template class LIBPLZMA_CPP_CLASS_API Vector<SharedPtr<InStream> >;
    template class LIBPLZMA_CPP_CLASS_API Vector<SharedPtr<OutStream> >;

    /// @brief The interface to a progress delegate of the encoder and decoder.
    class ProgressDelegate {
    public:
        /// @brief Reports the progress of the operation.
        /// @param context The context provided to an encoder or decoder.
        /// @param path The currently processing item's path.
        /// @param progress The total progress of the operation.
        virtual void onProgress(void * LIBPLZMA_NULLABLE context, const String & path, const double progress) = 0;
    };
    
    
    /// @brief The \a Decoder for extracting or testing archive items.
    class Decoder {
    private:
        friend struct SharedPtr<Decoder>;
        virtual void retain() = 0;
        virtual void release() = 0;
        
    protected:
        virtual ~Decoder() = default;
        
    public:
        /// @brief Provides the archive password for opening, extracting or testing items.
        /// @param password The password wide character presentation.
        /// @note Thread-safe.
        /// @throws \a Exception in case if crypto disabled.
        virtual void setPassword(const wchar_t * LIBPLZMA_NULLABLE password) = 0;
        
        
        /// @brief Provides the archive password for opening, extracting or testing items.
        /// @param password The password UTF-8 character presentation. NULL or zero length password means no password provided.
        /// @note Thread-safe.
        /// @throws \a Exception in case if crypto disabled.
        virtual void setPassword(const char * LIBPLZMA_NULLABLE password) = 0;
        
        
        /// @brief Provides the extract or test progress delegate.
        /// @note Thread-safe.
        virtual void setProgressDelegate(ProgressDelegate * LIBPLZMA_NULLABLE delegate) = 0;
        
        
        /// @brief Opens the archive.
        ///
        /// During the process, the decoder is self-retained as long as the operation is in progress.
        /// @return \a true the archive was successfully opened, otherwice \a false.
        /// @note After successful opening, the input stream will be opened as long as the decoder exists.
        /// @note The opening progress might be executed in a separate thread.
        /// @note The opening progress might be aborted via \a abort() method.
        /// @note Thread-safe.
        virtual bool open() = 0;
        
        
        /// @brief Aborts opening, extracting or testing process.
        /// @note The aborted decoder is no longer valid.
        /// @note Thread-safe.
        virtual void abort() = 0;
        
        
        /// @return Receives the number of items in archive.
        /// @note The decoder must be opened.
        /// @note Thread-safe.
        virtual plzma_size_t count() const = 0;
        
        
        /// @brief Receives all archive items.
        /// @return The new array instance with all archive items.
        /// @note The decoder must be opened.
        /// @note Thread-safe.
        virtual SharedPtr<ItemArray> items() const = 0;
        
        
        /// @brief Receives a single archive item at a specific index.
        /// @param index The index of the item inside the arhive. Must be less than the number of items reported by the \a count() method.
        /// @return The archive item.
        /// @note The decoder must be opened.
        /// @note Thread-safe.
        virtual SharedPtr<Item> itemAt(const plzma_size_t index) const = 0;
        
        
        /// @brief Extracts all archive items to a specific path.
        ///
        /// During the process, the decoder is self-retained as long as the operation is in progress.
        /// @param path The directory path to extract all items.
        /// @param usingItemsFullPath Extract item using it's full path or only last path component.
        /// @note The extracting progress might be executed in a separate thread.
        /// @note The extracting progress might be aborted via \a abort() method.
        /// @note Thread-safe.
        virtual bool extract(const Path & path, const bool usingItemsFullPath = true) = 0;
        
        
        /// @brief Extracts some archive items to a specific path.
        ///
        /// During the process, the decoder is self-retained as long as the operation is in progress.
        /// @param items The array of items to extract.
        /// @param path The directory path to extract all items.
        /// @param usingItemsFullPath Extract item using it's full path or only the last path component.
        /// @note The extracting progress might be executed in a separate thread.
        /// @note The extracting progress might be aborted via \a abort() method.
        /// @note Thread-safe.
        virtual bool extract(const SharedPtr<ItemArray> & items,
                             const Path & path,
                             const bool usingItemsFullPath = true) = 0;
        
        
        /// @brief Extracts each archive item to a separate out-stream.
        ///
        /// During the process, the decoder is self-retained as long as the operation is in progress.
        /// @param items The array with item/out-stream pairs.
        /// @note The extracting progress might be executed in a separate thread.
        /// @note The extracting progress might be aborted via \a abort() method.
        /// @note Thread-safe.
        virtual bool extract(const SharedPtr<ItemOutStreamArray> & items) = 0;
        
        
        /// @brief Tests specific archive items.
        ///
        /// During the process, the decoder is self-retained as long as the operation is in progress.
        /// @param items The array with items to test.
        /// @note The testing progress might be executed in a separate thread.
        /// @note The testing progress might be aborted via \a abort() method.
        /// @note Thread-safe.
        virtual bool test(const SharedPtr<ItemArray> & items) = 0;
        
        
        /// @brief Tests all archive items.
        ///
        /// During the process, the decoder is self-retained as long as the operation is in progress.
        /// @note The testing progress might be executed in a separate thread.
        /// @note The testing progress might be aborted via \a abort() method.
        /// @note Thread-safe.
        virtual bool test() = 0;
    };
    
    template struct LIBPLZMA_CPP_CLASS_API SharedPtr<Decoder>;
    
    
    /// @brief Creates the decoder for extracting or testing archive items.
    ///
    /// @param stream The input stream which contains the archive file content.
    ///               After successful opening, the input stream will be opened as long as the decoder exists.
    /// @param type The type of the arhive file content.
    /// @param context The user provided context.
    /// @exception The \a Exception with \a plzma_error_code_invalid_arguments code in case if provided stream is empty.
    LIBPLZMA_CPP_API(SharedPtr<Decoder>) makeSharedDecoder(const SharedPtr<InStream> & stream,
                                                           const plzma_file_type type,
                                                           const plzma_context context = plzma_context{nullptr, nullptr}); // C2059 = { .context = nullptr, .deinitializer = nullptr }
    
    
    /// @brief The \a Encoder for compressing archive items.
    class Encoder {
    private:
        friend struct SharedPtr<Encoder>;
        virtual void retain() = 0;
        virtual void release() = 0;
        
    protected:
        virtual ~Encoder() = default;
        
    public:
        /// @brief Provides the password for archive.
        ///
        /// This password will be used for encrypting header and the content if such options are enabled
        /// and selected type supports password protection.
        /// See \a setShouldEncryptHeader, \a setShouldEncryptContent methods and \a plzma_file_type enum.
        /// @param password The password wide character presentation. NULL or zero length password means no password provided.
        /// @note Thread-safe. Must be set before opening.
        /// @throws \a Exception in case if crypto disabled.
        virtual void setPassword(const wchar_t * LIBPLZMA_NULLABLE password) = 0;
        
        
        /// @brief Provides the password for archive.
        ///
        /// This password will be used for encrypting header and the content if such options are enabled
        /// and selected type supports password protection.
        /// See \a setShouldEncryptHeader, \a setShouldEncryptContent methods and \a plzma_file_type enum.
        /// @param password The password UTF-8 character presentation. NULL or zero length password means no password provided.
        /// @note Thread-safe. Must be set before opening.
        /// @throws \a Exception in case if crypto disabled.
        virtual void setPassword(const char * LIBPLZMA_NULLABLE password) = 0;
        
        
        /// @brief Provides the compression progress delegate.
        /// @note Thread-safe.
        virtual void setProgressDelegate(ProgressDelegate * LIBPLZMA_NULLABLE delegate) = 0;
        
        
        /// @brief Adds the physical file or directory path to the encoder.
        /// @param path The file or directory path. Duplicated path is not allowed.
        /// @param openDirMode The mode for opening directory in case if \a path is a directory path.
        /// @param archivePath The optional path of how the item's \a path will be presented in archive.
        /// @note Thread-safe. Must be set before opening.
        virtual void add(const Path & path, const plzma_open_dir_mode_t openDirMode = 0, const Path & archivePath = Path()) = 0;
        
        
        /// @brief Adds the in-stream to the encoder.
        /// @param stream The input file stream to add. Empty stream is not allowed.
        /// @param archivePath The optional path of how the item's \a path will be presented in archive. Empty path is not allowed.
        /// @note Thread-safe. Must be set before opening.
        virtual void add(const SharedPtr<InStream> & stream, const Path & archivePath) = 0;
        
        
        /// @brief Opens the encoder for compressing.
        ///
        /// During the process, the encoder is self-retained as long as the operation is in progress.
        /// @return \a false if nothing to compress or encoder aborted or incorrect number of items or number of items greater than supported,
        /// otherwise \a true.
        /// @note The opening progress might be executed in a separate thread.
        /// @note The opening progress might be aborted via \a abort() method.
        /// @note Thread-safe.
        /// @throws \a Exception in case if crypto disabled and archive's type/settings depends on it.
        virtual bool open() = 0;
        
        
        /// @brief Aborts the opening or compressing process.
        /// @note The aborted encoder is no longer valid.
        virtual void abort() = 0;
        
        
        /// @brief Compresses the provided paths and streams.
        ///
        /// During the process, the encoder is self-retained as long as the operation is in progress.
        /// @note The compress progress might be executed in a separate thread.
        /// @note The compress progress might be aborted via \a abort() method.
        /// @note Thread-safe.
        virtual bool compress() = 0;
        
        
        /// @brief Getter for a 'solid' archive property.
        /// @note Enabled by default, the value is \a true.
        /// @note Thread-safe.
        virtual bool shouldCreateSolidArchive() const = 0;
        
        
        /// @brief Setter for a 'solid' archive property.
        /// @note Thread-safe. Must be set before opening.
        virtual void setShouldCreateSolidArchive(const bool solid) = 0;
        
        
        /// @brief Getter for a compression level.
        /// @return The level in a range [0; 9].
        /// @note Thread-safe.
        virtual uint8_t compressionLevel() const = 0;
        
        
        /// @brief Setter for an archive compression level.
        /// @param level The level in a range [0; 9].
        /// @note Thread-safe. Must be set before opening.
        virtual void setCompressionLevel(const uint8_t level) = 0;
        
        
        /// @brief Should encoder compress the archive header.
        /// @note Enabled by default, the value is \a true.
        /// @note Thread-safe.
        virtual bool shouldCompressHeader() const = 0;
        
        
        /// @brief Set encoder will compress the archive header.
        /// @note Thread-safe. Must be set before opening.
        virtual void setShouldCompressHeader(const bool compress) = 0;
        
        
        /// @brief Should encoder fully compress the archive header.
        /// @note Enabled by default, the value is \a true.
        /// @note Thread-safe.
        virtual bool shouldCompressHeaderFull() const = 0;
        
        
        /// @brief Set encoder will fully compress the archive header.
        /// @note Thread-safe. Must be set before opening.
        virtual void setShouldCompressHeaderFull(const bool compress) = 0;
        
        
        /// @brief Should encoder encrypt the content of archive items.
        /// @note The password will be required to decode/extract archive items.
        /// @note Thread-safe.
        virtual bool shouldEncryptContent() const = 0;
        
        
        /// @brief Set encoder will encrypt the content of archive items.
        ///
        /// The encryption will take place only if this option enabled, the type supports password protection
        /// and the password has been provided.
        /// See \a setPassword method and \a plzma_file_type enum.
        /// @note The password will be required to decode/extract archive items.
        /// @note Thread-safe. Must be set before opening.
        virtual void setShouldEncryptContent(const bool encrypt) = 0;
        
        
        /// @brief Should encoder encrypt the header with the list of archive items.
        /// @note The password will be required to open archive and list the items.
        /// @note Thread-safe.
        virtual bool shouldEncryptHeader() const = 0;
        
        
        /// @brief Set encoder will encrypt the header with the list of archive items.
        ///
        /// The encryption will take place only if this option enabled, the type supports password protection
        /// and the password has been provided.
        /// See \a setPassword method and \a plzma_file_type enum.
        /// @note The password will be required to open archive and list the items.
        /// @note Thread-safe. Must be set before opening.
        virtual void setShouldEncryptHeader(const bool encrypt) = 0;
        
        
        /// @brief Should encoder store the creation time of each item to the archive header, if such available.
        /// @note Enabled by default, the value is \a true.
        /// @note Thread-safe.
        virtual bool shouldStoreCreationTime() const = 0;
        
        
        /// @brief Set encoder will store the creation time of each item to the archive header, if such available.
        /// @note Thread-safe. Must be set before opening.
        virtual void setShouldStoreCreationTime(const bool store) = 0;
        
        
        /// @brief Should encoder store the access time of each item to the archive header, if such available.
        /// @note Enabled by default, the value is \a true.
        /// @note Thread-safe.
        virtual bool shouldStoreAccessTime() const = 0;
        
        
        /// @brief Set encoder will store the access time of each item to the archive header, if such available.
        /// @note Thread-safe. Must be set before opening.
        virtual void setShouldStoreAccessTime(const bool store) = 0;
        
        
        /// @brief Should encoder store the last modification time of each item to the archive header, if such available.
        /// @note Enabled by default, the value is \a true.
        /// @note Thread-safe.
        virtual bool shouldStoreModificationTime() const = 0;
        
        
        /// @brief Set encoder will store the last modification time of each item to the archive header, if such available.
        /// @note Thread-safe. Must be set before opening.
        virtual void setShouldStoreModificationTime(const bool store) = 0;
    };
    
    
    /// @brief Creates the encoder with output file stream.
    /// @param stream The output file stream to write the archive's file content.
    /// @param type The type of the archive.
    /// @param method The compresion method.
    /// @param context The user provided context to inform the progress of the operation.
    /// @exception The \a Exception with \a plzma_error_code_invalid_arguments code in case if provided stream is empty.
    LIBPLZMA_CPP_API(SharedPtr<Encoder>) makeSharedEncoder(const SharedPtr<OutStream> & stream,
                                                           const plzma_file_type type,
                                                           const plzma_method method,
                                                           const plzma_context context = plzma_context{nullptr, nullptr}); // C2059 = { .context = nullptr, .deinitializer = nullptr }


    /// @brief Creates the encoder with output multi stream.
    /// @param stream The output multi stream to write the archive's file content.
    /// @param type The type of the archive. Currently supports only 7-zip archive type.
    /// @param method The compresion method.
    /// @param context The user provided context to inform the progress of the operation.
    /// @exception The \a Exception with \a plzma_error_code_invalid_arguments code in case if provided stream is empty.
    /// @exception The \a Exception with \a plzma_error_code_invalid_arguments code in case if provided archive type is not 7-zip.
    LIBPLZMA_CPP_API(SharedPtr<Encoder>) makeSharedEncoder(const SharedPtr<OutMultiStream> & stream,
                                                           const plzma_file_type type,
                                                           const plzma_method method,
                                                           const plzma_context context = plzma_context{nullptr, nullptr}); // C2059 = { .context = nullptr, .deinitializer = nullptr }

} // namespace plzma

#endif // !__LIBPLZMA_HPP__
