/// @copyright Copyright (c) 2015 Tim Niederhausen (tim@rnc-ag.de)
/// Distributed under the Boost Software License, Version 1.0.
/// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "asioext/open_flags.hpp"
#include "asioext/open_args.hpp"

#include "asioext/detail/win_file_ops.hpp"
#include "asioext/detail/chrono.hpp"
#include "asioext/detail/error.hpp"

#if defined(ASIOEXT_WINDOWS_USE_UTF8_FILENAMES) || defined(ASIOEXT_WINDOWS_APP)
# include "asioext/detail/win_path.hpp"
#endif

#include <windows.h>

ASIOEXT_NS_BEGIN

namespace detail {
namespace win_file_ops {

// these two are not in the header because they use a Windows.h type
ASIOEXT_DECL file_time_type filetime_to_chrono(FILETIME ft) ASIOEXT_NOEXCEPT
{
  ULARGE_INTEGER temp;
  temp.LowPart = ft.dwLowDateTime;
  temp.HighPart = ft.dwHighDateTime;
  return file_time_type(file_clock::duration(temp.QuadPart));
}

ASIOEXT_DECL FILETIME chrono_to_filetime(file_time_type t) ASIOEXT_NOEXCEPT
{
  ULARGE_INTEGER temp;
  temp.QuadPart = t.time_since_epoch().count();

  FILETIME ft;
  ft.dwLowDateTime = temp.LowPart;
  ft.dwHighDateTime = temp.HighPart;
  return ft;
}

void set_error(error_code& ec) ASIOEXT_NOEXCEPT
{
  ec = error_code(::GetLastError(), asio::error::get_system_category());
}

uint32_t file_attrs_to_native(file_attrs attrs) ASIOEXT_NOEXCEPT
{
  uint32_t native = 0;
  if ((attrs & file_attrs::hidden) != file_attrs::none)
    native |= FILE_ATTRIBUTE_HIDDEN;
  if ((attrs & file_attrs::system) != file_attrs::none)
    native |= FILE_ATTRIBUTE_SYSTEM;
  if ((attrs & file_attrs::not_indexed) != file_attrs::none)
    native |= FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
  return native;
}

file_attrs native_to_file_attrs(uint32_t native) ASIOEXT_NOEXCEPT
{
  file_attrs attrs = file_attrs::none;
  if (native & FILE_ATTRIBUTE_HIDDEN)
    attrs |= file_attrs::hidden;
  if (native & FILE_ATTRIBUTE_SYSTEM)
    attrs |= file_attrs::system;
  if (native & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)
    attrs |= file_attrs::not_indexed;
  return attrs;
}

void parse_open_flags(open_flags flags, file_perms perms, file_attrs attrs,
                      open_args& args) ASIOEXT_NOEXCEPT
{
  uint32_t creation_disposition;
  if ((flags & open_flags::create_new) != open_flags::none)
    creation_disposition = CREATE_NEW;
  else if ((flags & open_flags::create_always) != open_flags::none)
    creation_disposition = CREATE_ALWAYS;
  else if ((flags & open_flags::open_existing) != open_flags::none)
    creation_disposition = OPEN_EXISTING;
  else if ((flags & open_flags::open_always) != open_flags::none)
    creation_disposition = OPEN_ALWAYS;
  else if ((flags & open_flags::truncate_existing) != open_flags::none)
    creation_disposition = TRUNCATE_EXISTING;
  else
    creation_disposition = 0;

  uint32_t desired_access = 0;
  if ((flags & open_flags::access_read) != open_flags::none)
    desired_access |= GENERIC_READ;
  if ((flags & open_flags::access_write) != open_flags::none)
    desired_access |= GENERIC_WRITE;

  uint32_t native_attrs = file_attrs_to_native(attrs);

  ASIOEXT_CONSTEXPR file_perms write_perms = file_perms::owner_write |
                                             file_perms::group_write |
                                             file_perms::others_write;

  if ((perms & write_perms) != file_perms::none)
    native_attrs &= ~FILE_ATTRIBUTE_READONLY;
  else
    native_attrs |= FILE_ATTRIBUTE_READONLY;

  uint32_t native_flags = 0;
  if ((flags & open_flags::internal_async) != open_flags::none)
    native_flags |= FILE_FLAG_OVERLAPPED;

  // TODO: FILE_SHARE_DELETE?
  uint32_t share_mode = FILE_SHARE_READ | FILE_SHARE_WRITE;
  if ((flags & open_flags::exclusive_read) != open_flags::none)
    share_mode &= ~FILE_SHARE_READ;
  if ((flags & open_flags::exclusive_write) != open_flags::none)
    share_mode &= ~FILE_SHARE_WRITE;

  args.creation_disposition(creation_disposition);
  args.desired_access(desired_access);
  args.attrs(native_attrs);
  args.flags(native_flags);
  args.share_mode(share_mode);
}

handle_type open(const char* filename, const open_args& args,
                 error_code& ec) ASIOEXT_NOEXCEPT
{
#if defined(ASIOEXT_WINDOWS_USE_UTF8_FILENAMES) || defined(ASIOEXT_WINDOWS_APP)
  detail::win_path p(filename, std::strlen(filename), ec);
  if (ec) return INVALID_HANDLE_VALUE;
#endif

#if !defined(ASIOEXT_WINDOWS_APP)
  const handle_type h =
# if defined(ASIOEXT_WINDOWS_USE_UTF8_FILENAMES)
      ::CreateFileW(p.c_str(),
# else
      ::CreateFileA(filename,
# endif
                    args.desired_access(), args.share_mode(), NULL,
                    args.creation_disposition(),
                    args.attrs() | args.flags(), NULL);
#else
  CREATEFILE2_EXTENDED_PARAMETERS params = {};
  params.dwSize = sizeof(params);
  params.dwFileAttributes = args.attrs();
  params.dwFileFlags = args.flags();
  const handle_type h =
      ::CreateFile2(p.c_str(), args.desired_access(), args.share_mode(),
                    args.creation_disposition(), &params);
#endif

  if (h == INVALID_HANDLE_VALUE)
    set_error(ec);
  else
    ec = error_code();

  return h;
}

handle_type open(const wchar_t* filename, const open_args& args,
                 error_code& ec) ASIOEXT_NOEXCEPT
{
#if !defined(ASIOEXT_WINDOWS_APP)
  const handle_type h =
      ::CreateFileW(filename, args.desired_access(), args.share_mode(), NULL,
                    args.creation_disposition(),
                    args.attrs() | args.flags(), NULL);
#else
  CREATEFILE2_EXTENDED_PARAMETERS params = {};
  params.dwSize = sizeof(params);
  params.dwFileAttributes = args.attrs();
  params.dwFileFlags = args.flags();
  const handle_type h =
      ::CreateFile2(filename, args.desired_access(), args.share_mode(),
                    args.creation_disposition(), &params);
#endif

  if (h != INVALID_HANDLE_VALUE)
    ec = error_code();
  else
    set_error(ec);

  return h;
}

void close(handle_type fd, error_code& ec) ASIOEXT_NOEXCEPT
{
  if (::CloseHandle(fd))
    ec = error_code();
  else
    set_error(ec);
}

handle_type duplicate(handle_type fd, error_code& ec) ASIOEXT_NOEXCEPT
{
  const handle_type current_process = ::GetCurrentProcess();
  handle_type new_fd = INVALID_HANDLE_VALUE;

  if (::DuplicateHandle(current_process, fd, current_process, &new_fd,
                        0, FALSE, DUPLICATE_SAME_ACCESS))
    ec = error_code();
  else
    set_error(ec);

  return new_fd;
}

handle_type get_stdin(error_code& ec) ASIOEXT_NOEXCEPT
{
#if !defined(ASIOEXT_WINDOWS_APP)
  const handle_type h = ::GetStdHandle(STD_INPUT_HANDLE);
  if (h != INVALID_HANDLE_VALUE)
    ec = error_code();
  else
    set_error(ec);

  return h;
#else
  ec = asio::error::operation_not_supported;
  return INVALID_HANDLE_VALUE;
#endif
}

handle_type get_stdout(error_code& ec) ASIOEXT_NOEXCEPT
{
#if !defined(ASIOEXT_WINDOWS_APP)
  const handle_type h = ::GetStdHandle(STD_OUTPUT_HANDLE);
  if (h != INVALID_HANDLE_VALUE)
    ec = error_code();
  else
    set_error(ec);

  return h;
#else
  ec = asio::error::operation_not_supported;
  return INVALID_HANDLE_VALUE;
#endif
}

handle_type get_stderr(error_code& ec) ASIOEXT_NOEXCEPT
{
#if !defined(ASIOEXT_WINDOWS_APP)
  const handle_type h = ::GetStdHandle(STD_ERROR_HANDLE);
  if (h != INVALID_HANDLE_VALUE)
    ec = error_code();
  else
    set_error(ec);

  return h;
#else
  ec = asio::error::operation_not_supported;
  return INVALID_HANDLE_VALUE;
#endif
}

uint64_t size(handle_type fd, error_code& ec) ASIOEXT_NOEXCEPT
{
  LARGE_INTEGER size;
  if (::GetFileSizeEx(fd, &size)) {
    ec = error_code();
    return static_cast<uint64_t>(size.QuadPart);
  }

  set_error(ec);
  return 0;
}

void size(handle_type fd, uint64_t new_size, error_code& ec) ASIOEXT_NOEXCEPT
{
#if (_WIN32_WINNT >= 0x0600)
  // On Vista+ we can avoid having to modify the file pointer.
  FILE_ALLOCATION_INFO ai;
  FILE_END_OF_FILE_INFO ei;

  ai.AllocationSize.QuadPart = new_size;
  ei.EndOfFile.QuadPart = new_size;

  if (::SetFileInformationByHandle(fd, FileEndOfFileInfo, &ei, sizeof(ei)) &&
      // Explicitly deallocate space for |new_size| < |old size|
      ::SetFileInformationByHandle(fd, FileAllocationInfo, &ai, sizeof(ai))) {
    ec = error_code();
  } else {
    set_error(ec);
  }
#else
  LARGE_INTEGER old_pointer, zero, size;

  zero.QuadPart = 0;
  size.QuadPart = new_size;

  if (::SetFilePointerEx(fd, zero, &old_pointer, FILE_CURRENT) &&
      ::SetFilePointerEx(fd, size, NULL, FILE_BEGIN)) {
    if (::SetEndOfFile(fd)) {
      if (::SetFilePointerEx(fd, old_pointer, NULL, FILE_BEGIN)) {
        ec = error_code();
        return;
      }
    } else {
      // We failed to set the new size, but we can still reset the file
      // pointer to avoid surprising the user.
      ::SetFilePointerEx(fd, old_pointer, NULL, FILE_BEGIN);
    }
  }
  set_error(ec);
#endif
}

// Make sure our origin mappings match the system headers.
static_assert(static_cast<DWORD>(seek_origin::from_begin) == FILE_BEGIN &&
              static_cast<DWORD>(seek_origin::from_current) == FILE_CURRENT &&
              static_cast<DWORD>(seek_origin::from_end) == FILE_END,
              "whence mapping must match the system headers");

uint64_t seek(handle_type fd, seek_origin origin, int64_t offset,
              error_code& ec) ASIOEXT_NOEXCEPT
{
  LARGE_INTEGER pos, res;
  pos.QuadPart = offset;

  if (::SetFilePointerEx(fd, pos, &res, static_cast<DWORD>(origin)))
    return res.QuadPart;

  set_error(ec);
  return 0;
}

file_perms permissions(handle_type fd, error_code& ec) ASIOEXT_NOEXCEPT
{
#if (_WIN32_WINNT >= 0x0600)
  ASIOEXT_CONSTEXPR file_perms write_perms = file_perms::owner_write |
                                             file_perms::group_write |
                                             file_perms::others_write;
  FILE_BASIC_INFO info;
  if (::GetFileInformationByHandleEx(fd, FileBasicInfo, &info, sizeof(info))) {
    ec = error_code();
    return info.FileAttributes & FILE_ATTRIBUTE_READONLY ?
           file_perms::all & ~write_perms : file_perms::all;
  }
  set_error(ec);
#else
  ec = asio::error::operation_not_supported;
#endif
  return file_perms::none;
}

void permissions(handle_type fd, file_perms perms, file_perm_options opts,
                 error_code& ec) ASIOEXT_NOEXCEPT
{
#if (_WIN32_WINNT >= 0x0600)
  ASIOEXT_CONSTEXPR file_perms write_perms = file_perms::owner_write |
                                             file_perms::group_write |
                                             file_perms::others_write;

  // Quit early if the changed values are without effect (i.e. not implemented)
  if ((opts & (file_perm_options::add | file_perm_options::remove)) !=
      static_cast<file_perm_options>(0) && (perms & write_perms) == file_perms::none) {
    ec = error_code();
    return;
  }

  FILE_BASIC_INFO info;
  if (::GetFileInformationByHandleEx(fd, FileBasicInfo, &info, sizeof(info))) {
    if ((opts & file_perm_options::add) !=
        static_cast<file_perm_options>(0))
      info.FileAttributes &= ~FILE_ATTRIBUTE_READONLY;
    else if ((opts & file_perm_options::remove) !=
             static_cast<file_perm_options>(0))
      info.FileAttributes |= FILE_ATTRIBUTE_READONLY;
    else if ((perms & write_perms) != file_perms::none)
      info.FileAttributes &= ~FILE_ATTRIBUTE_READONLY;
    else
      info.FileAttributes |= FILE_ATTRIBUTE_READONLY;
  } else {
    set_error(ec);
    return;
  }

  // We deliberately set all the other fields (i.e. file times) to zero,
  // so they are ignored by the kernel. Otherwise we'd risk overwriting
  // changes that happened in the meantime.
  info.CreationTime.QuadPart = 0;
  info.LastAccessTime.QuadPart = 0;
  info.LastWriteTime.QuadPart = 0;
  info.ChangeTime.QuadPart = 0;

  if (::SetFileInformationByHandle(fd, FileBasicInfo, &info, sizeof(info)))
    return;
  set_error(ec);
#else
  ec = asio::error::operation_not_supported;
#endif
}

file_attrs attributes(handle_type fd, error_code& ec) ASIOEXT_NOEXCEPT
{
#if (_WIN32_WINNT >= 0x0600)
  FILE_BASIC_INFO info;
  if (::GetFileInformationByHandleEx(fd, FileBasicInfo, &info, sizeof(info))) {
    ec = error_code();
    return native_to_file_attrs(info.FileAttributes);
  }
  set_error(ec);
#else
  ec = asio::error::operation_not_supported;
#endif
  return file_attrs::none;
}

void attributes(handle_type fd, file_attrs attrs, file_attr_options opts,
                error_code& ec) ASIOEXT_NOEXCEPT
{
#if (_WIN32_WINNT >= 0x0600)
  FILE_BASIC_INFO info;
  if (!::GetFileInformationByHandleEx(fd, FileBasicInfo, &info, sizeof(info))) {
    set_error(ec);
    return;
  }

  const uint32_t new_attrs = file_attrs_to_native(attrs);
  if ((opts & (file_attr_options::add | file_attr_options::remove)) !=
      static_cast<file_attr_options>(0)) {
    if ((opts & file_attr_options::add) != static_cast<file_attr_options>(0))
      info.FileAttributes |= new_attrs;
    else
      info.FileAttributes &= ~new_attrs;
  } else {
    info.FileAttributes = new_attrs;
  }

  // 0 means "don't change any attributes" and not "no attributes set"
  // as one might expect.
  if (info.FileAttributes == 0)
    info.FileAttributes = FILE_ATTRIBUTE_NORMAL;

  // We deliberately set all the other fields (file times) to zero,
  // so they are ignored by the kernel. Otherwise we'd risk overwriting
  // changes that happened in the meantime.
  info.CreationTime.QuadPart = 0;
  info.LastAccessTime.QuadPart = 0;
  info.LastWriteTime.QuadPart = 0;
  info.ChangeTime.QuadPart = 0;

  if (::SetFileInformationByHandle(fd, FileBasicInfo, &info, sizeof(info)))
    return;
  set_error(ec);
#else
  ec = asio::error::operation_not_supported;
#endif
}

void get_times(handle_type fd, file_time_type& ctime, file_time_type& atime,
               file_time_type& mtime, error_code& ec) ASIOEXT_NOEXCEPT
{
  FILETIME cft, mft, aft;
  if (::GetFileTime(fd, &cft, &aft, &mft)) {
    ctime = filetime_to_chrono(cft);
    atime = filetime_to_chrono(aft);
    mtime = filetime_to_chrono(mft);
    ec = error_code();
    return;
  }
  set_error(ec);
}

void set_times(handle_type fd, file_time_type ctime, file_time_type atime,
               file_time_type mtime, error_code& ec) ASIOEXT_NOEXCEPT
{
  const FILETIME cft = chrono_to_filetime(ctime);
  const FILETIME aft = chrono_to_filetime(atime);
  const FILETIME mft = chrono_to_filetime(mtime);
  if (::SetFileTime(fd, &cft, &aft, &mft)) {
    ec = error_code();
    return;
  }
  set_error(ec);
}

uint32_t read(handle_type fd, void* buffer, uint32_t size,
              error_code& ec) ASIOEXT_NOEXCEPT
{
  DWORD bytesRead = 0;
  if (!::ReadFile(fd, buffer, size, &bytesRead, NULL)) {
    set_error(ec);
    return 0;
  }

  if (bytesRead == 0 && size != 0)
    ec = asio::error::eof;

  return bytesRead;
}

uint32_t write(handle_type fd, const void* buffer, uint32_t size,
               error_code& ec) ASIOEXT_NOEXCEPT
{
  DWORD bytesWritten = 0;
  if (!::WriteFile(fd, buffer, size, &bytesWritten, NULL)) {
    set_error(ec);
    return 0;
  }

  return bytesWritten;
}

uint32_t pread(handle_type fd, void* buffer, uint32_t size, uint64_t offset,
               error_code& ec) ASIOEXT_NOEXCEPT
{
  LARGE_INTEGER offset2;
  offset2.QuadPart = offset;

  OVERLAPPED overlapped = {0};
  overlapped.Offset = offset2.LowPart;
  overlapped.OffsetHigh = offset2.HighPart;

  DWORD bytesRead = 0;
  if (!::ReadFile(fd, buffer, size, &bytesRead, &overlapped)) {
    set_error(ec);
    return 0;
  }

  if (bytesRead == 0 && size != 0)
    ec = asio::error::eof;

  return bytesRead;
}

uint32_t pwrite(handle_type fd, const void* buffer, uint32_t size,
                uint64_t offset, error_code& ec) ASIOEXT_NOEXCEPT
{
  LARGE_INTEGER offset2;
  offset2.QuadPart = offset;

  OVERLAPPED overlapped = {0};
  overlapped.Offset = offset2.LowPart;
  overlapped.OffsetHigh = offset2.HighPart;

  DWORD bytesWritten = 0;
  if (!::WriteFile(fd, buffer, size, &bytesWritten, &overlapped)) {
    set_error(ec);
    return 0;
  }

  return bytesWritten;
}

}
}

ASIOEXT_NS_END
