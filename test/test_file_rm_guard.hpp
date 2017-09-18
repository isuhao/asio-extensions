/// Copyright (c) 2016 Tim Niederhausen (tim@rnc-ag.de)
/// Distributed under the Boost Software License, Version 1.0.
/// (See accompanying file LICENSE_1_0.txt or copy at
/// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ASIOEXT_TEST_TESTFILERMGUARD_HPP
#define ASIOEXT_TEST_TESTFILERMGUARD_HPP

#include "asioext/detail/config.hpp"

#if ASIOEXT_HAS_PRAGMA_ONCE
# pragma once
#endif

#include <boost/filesystem.hpp>

#include <iostream>
#include <utility>

ASIOEXT_NS_BEGIN

struct test_file_rm_guard
{
  test_file_rm_guard(boost::filesystem::path filename)
    : filename_(filename)
  {
    // ctor
  }

  test_file_rm_guard(test_file_rm_guard&& o)
    : filename_(std::move(o.filename_))
  {
    // ctor
  }

  ~test_file_rm_guard()
  {
    if (filename_.empty())
      return;

    boost::system::error_code ec;
    boost::filesystem::remove(filename_, ec);
    if (ec) {
      std::cerr << "Failed to delete " << filename_ << " with " << ec;
      std::cerr << std::endl;
      std::abort();
    }
  }

  test_file_rm_guard& operator=(test_file_rm_guard&& o)
  {
    filename_ = std::move(filename_);
    return *this;
  }

  boost::filesystem::path filename_;
};

ASIOEXT_NS_END

#endif
