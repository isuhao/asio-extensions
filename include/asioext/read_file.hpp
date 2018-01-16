/// @file
/// Declares the various asioext::read_file overloads.
///
/// @copyright Copyright (c) 2016 Tim Niederhausen (tim@rnc-ag.de)
/// Distributed under the Boost Software License, Version 1.0.
/// (See accompanying file LICENSE_1_0.txt or copy at
/// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ASIOEXT_READFILE_HPP
#define ASIOEXT_READFILE_HPP

#include "asioext/detail/config.hpp"

#if ASIOEXT_HAS_PRAGMA_ONCE
# pragma once
#endif

#include "asioext/is_raw_byte_container.hpp"
#include "asioext/error_code.hpp"

#include "asioext/detail/asio_version.hpp"
#include "asioext/detail/buffer.hpp"

#if defined(ASIOEXT_HAS_BOOST_FILESYSTEM) || defined(ASIOEXT_IS_DOCUMENTATION)
# include <boost/filesystem/path.hpp>
#endif

#include <type_traits>

ASIOEXT_NS_BEGIN

/// @ingroup files
/// @defgroup read_file asioext::read_file()
/// Reads the entire contents of a file into memory.
///
///@{

/// @name RawByteContainer overloads
/// See @ref concept-RawByteContainer for RawByteContainer requirements.
/// @{

#if !defined(ASIOEXT_IS_DOCUMENTATION)
# define ASIOEXT_DETAIL_RF_RAW_RET(T) \
    typename std::enable_if<is_raw_byte_container<T>::value>::type
#else
# define ASIOEXT_DETAIL_RF_RAW_RET(T) void
#endif

/// Read a file into a container.
///
/// This function loads the contents of @c filename into @c c.
///
/// @param filename The path of the file to load.
///
/// @param c The container object which shall contain the file's
/// content. The container is resized to the file's size and any previous
/// data is overwritten. The container type must satisfy the
///  @ref concept-RawByteContainer requirements.
///
/// @throws asio::system_error Thrown on failure.
template <class RawByteContainer>
ASIOEXT_DETAIL_RF_RAW_RET(RawByteContainer)
    read_file(const char* filename, RawByteContainer& c);

/// Read a file into a container.
///
/// This function loads the contents of @c filename into @c c.
///
/// @param filename The path of the file to load.
///
/// @param c The container object which shall contain the file's
/// content. The container is resized to the file's size and any previous
/// data is overwritten. The container type must satisfy the
///  @ref concept-RawByteContainer requirements.
///
/// @param ec Set to indicate what error occurred. If no error occurred,
/// the object is reset.
template <class RawByteContainer>
ASIOEXT_DETAIL_RF_RAW_RET(RawByteContainer)
    read_file(const char* filename, RawByteContainer& c, error_code& ec);

#if defined(ASIOEXT_WINDOWS)  || defined(ASIOEXT_IS_DOCUMENTATION)
/// @copydoc read_file(const char*,RawByteContainer&)
///
/// @note Only available on Windows.
template <class RawByteContainer>
ASIOEXT_DETAIL_RF_RAW_RET(RawByteContainer)
    read_file(const wchar_t* filename, RawByteContainer& c);

/// @copydoc read_file(const char*,RawByteContainer&,error_code&)
///
/// @note Only available on Windows.
template <class RawByteContainer>
ASIOEXT_DETAIL_RF_RAW_RET(RawByteContainer)
    read_file(const wchar_t* filename, RawByteContainer& c, error_code& ec);
#endif

#if defined(ASIOEXT_HAS_BOOST_FILESYSTEM) || defined(ASIOEXT_IS_DOCUMENTATION)
/// @copydoc read_file(const char*,RawByteContainer&)
///
/// @note Only available if using Boost.Filesystem
/// (i.e. if @c ASIOEXT_HAS_BOOST_FILESYSTEM is defined)
template <class RawByteContainer>
ASIOEXT_DETAIL_RF_RAW_RET(RawByteContainer)
    read_file(const boost::filesystem::path& filename, RawByteContainer& c);

/// @copydoc read_file(const char*,RawByteContainer&,error_code&)
///
/// @note Only available if using Boost.Filesystem
/// (i.e. if @c ASIOEXT_HAS_BOOST_FILESYSTEM is defined)
template <class RawByteContainer>
ASIOEXT_DETAIL_RF_RAW_RET(RawByteContainer)
    read_file(const boost::filesystem::path& filename,
              RawByteContainer& c, error_code& ec);
#endif

class file_handle;

/// Read a file into a container.
///
/// This function loads the contents of @c file into @c c.
///
/// @param file The file_handle object to read from.
/// The file_handle's file pointer is expected to point at the beginning
/// of the file. Upon completion, the file pointer points at the end.
///
/// @param c The container object which shall contain the file's
/// content. The container is resized to the file's size and any previous
/// data is overwritten. The container type must satisfy the
///  @ref concept-RawByteContainer requirements.
///
/// @throws asio::system_error Thrown on failure.
template <class RawByteContainer>
ASIOEXT_DETAIL_RF_RAW_RET(RawByteContainer)
    read_file(file_handle file, RawByteContainer& c);

/// Read a file into a container.
///
/// This function loads the contents of @c file into @c c.
///
/// @param file The file_handle object to read from.
/// The file_handle's file pointer is expected to point at the beginning
/// of the file. Upon completion, the file pointer points at the end.
///
/// @param c The container object which shall contain the file's
/// content. The container is resized to the file's size and any previous
/// data is overwritten. The container type must satisfy the
///  @ref concept-RawByteContainer requirements.
///
/// @param ec Set to indicate what error occurred. If no error occurred,
/// the object is reset.
template <class RawByteContainer>
ASIOEXT_DETAIL_RF_RAW_RET(RawByteContainer)
    read_file(file_handle file, RawByteContainer& c,
              error_code& ec) ASIOEXT_NOEXCEPT;

/// @}

/// @name MutableBufferSequence overloads
/// @{

#if !defined(ASIOEXT_IS_DOCUMENTATION)
# if ASIOEXT_ASIO_VERSION < 101100
#  define ASIOEXT_DETAIL_RF_BUF_RET(T) \
    typename std::enable_if<!is_raw_byte_container<T>::value>::type
# else
#  define ASIOEXT_DETAIL_RF_BUF_RET(T) \
    typename std::enable_if<asio::is_mutable_buffer_sequence<T>::value>::type
# endif
#else
# define ASIOEXT_DETAIL_RF_BUF_RET(T) void
#endif

/// Read a file into a sequence of buffers.
///
/// This function loads the contents of @c filename into @c buffers.
///
/// @param filename The path of the file to load.
///
/// @param buffers The sequence of buffers to read the file into.
/// If the file size is less than the total size of all buffers, the
/// call will fail. Otherwise the buffer is filled entirely. Trailing
/// data is ignored.
///
/// @throws asio::system_error Thrown on failure.
template <class MutableBufferSequence>
ASIOEXT_DETAIL_RF_BUF_RET(MutableBufferSequence)
    read_file(const char* filename, const MutableBufferSequence& buffers);

/// Read a file into a sequence of buffers.
///
/// This function loads the contents of @c filename into @c buffers.
///
/// @param filename The path of the file to load.
///
/// @param buffers The sequence of buffers to read the file into.
/// If the file size is less than the total size of all buffers, the
/// call will fail. Otherwise the buffer is filled entirely. Trailing
/// data is ignored.
///
/// @param ec Set to indicate what error occurred. If no error occurred,
/// the object is reset.
template <class MutableBufferSequence>
ASIOEXT_DETAIL_RF_BUF_RET(MutableBufferSequence)
    read_file(const char* filename, const MutableBufferSequence& buffers,
              error_code& ec) ASIOEXT_NOEXCEPT;

#if defined(ASIOEXT_WINDOWS)  || defined(ASIOEXT_IS_DOCUMENTATION)
/// @copydoc read_file(const char*,const MutableBufferSequence&)
///
/// @note Only available on Windows.
template <class MutableBufferSequence>
ASIOEXT_DETAIL_RF_BUF_RET(MutableBufferSequence)
    read_file(const wchar_t* filename, const MutableBufferSequence& buffers);

/// @copydoc read_file(const char*,const MutableBufferSequence&,error_code&)
///
/// @note Only available on Windows.
template <class MutableBufferSequence>
ASIOEXT_DETAIL_RF_BUF_RET(MutableBufferSequence)
    read_file(const wchar_t* filename, const MutableBufferSequence& buffers,
              error_code& ec) ASIOEXT_NOEXCEPT;
#endif

#if defined(ASIOEXT_HAS_BOOST_FILESYSTEM) || defined(ASIOEXT_IS_DOCUMENTATION)
/// @copydoc read_file(const char*,const MutableBufferSequence&)
///
/// @note Only available if using Boost.Filesystem
/// (i.e. if @c ASIOEXT_HAS_BOOST_FILESYSTEM is defined)
template <class MutableBufferSequence>
ASIOEXT_DETAIL_RF_BUF_RET(MutableBufferSequence)
    read_file(const boost::filesystem::path& filename,
              const MutableBufferSequence& buffers);

/// @copydoc read_file(const char*,const MutableBufferSequence&,error_code&)
///
/// @note Only available if using Boost.Filesystem
/// (i.e. if @c ASIOEXT_HAS_BOOST_FILESYSTEM is defined)
template <class MutableBufferSequence>
ASIOEXT_DETAIL_RF_BUF_RET(MutableBufferSequence)
    read_file(const boost::filesystem::path& filename,
              const MutableBufferSequence& buffers,
              error_code& ec) ASIOEXT_NOEXCEPT;
#endif

/// Read a file into a sequence of buffers.
///
/// This function loads the contents of @c file into @c c.
///
/// @param file The file_handle object to read from.
/// The file_handle's file pointer is expected to point at the beginning
/// of the file. Upon completion, the file pointer points at the end.
///
/// @param buffers The sequence of buffers to read the file into.
/// If the file size is less than the total size of all buffers, the
/// call will fail. Otherwise the buffer is filled entirely. Trailing
/// data is ignored.
///
/// @throws asio::system_error Thrown on failure.
template <class MutableBufferSequence>
ASIOEXT_DETAIL_RF_BUF_RET(MutableBufferSequence)
    read_file(file_handle file, const MutableBufferSequence& buffers);

/// Read a file into a sequence of buffers.
///
/// This function loads the contents of @c file into @c c.
///
/// @param file The file_handle object to read from.
/// The file_handle's file pointer is expected to point at the beginning
/// of the file. Upon completion, the file pointer points at the end.
///
/// @param buffers The sequence of buffers to read the file into.
/// If the file size is less than the total size of all buffers, the
/// call will fail. Otherwise the buffer is filled entirely. Trailing
/// data is ignored.
///
/// @param ec Set to indicate what error occurred. If no error occurred,
/// the object is reset.
template <class MutableBufferSequence>
ASIOEXT_DETAIL_RF_BUF_RET(MutableBufferSequence)
    read_file(file_handle file, const MutableBufferSequence& buffers,
              error_code& ec) ASIOEXT_NOEXCEPT;

/// @}

// TODO(tim): Add support for asio's dynamic buffers,
// once they are released.

/// @}

ASIOEXT_NS_END

#include "asioext/impl/read_file.hpp"

#endif
