# Asio Extensions (_AsioExt_)

[![Build Status](https://travis-ci.org/timniederhausen/asio-extensions.svg?branch=master)](https://travis-ci.org/timniederhausen/asio-extensions)
[![Build status](https://ci.appveyor.com/api/projects/status/696yog08f0fbpck0/branch/master?svg=true)](https://ci.appveyor.com/project/timniederhausen/asio-extensions/branch/master)

## Feature overview

* `file_handle`: A thin wrapper around a native file handle that supports
  Asio's Stream* concepts.
* Utility functions for reading/writing files

AsioExt is compatible with standalone Asio, as well as the Boost variant.
To use the Boost version, #define ``ASIOEXT_USE_BOOST_ASIO``.

## Building

AsioExt requires at least a C++03-compliant compiler.
Not all AsioExt features are usable from C++03 though
(e.g. movable `scoped_file_handle` objects).
These features are enabled based on the C++11/C++14 support of the used compiler.

Currently, AsioExt is being tested with the following compilers:

* GCC 4.8
* Clang 3.6
* MSVC 2015

Like Asio, AsioExt is header-only by default.
However, it is also possible to build the AsioExt library as a separate compilation unit.

## Library dependencies:

* Asio 1.10.0+ (Only if not using Boost.Asio)
* [optional] Boost 1.46.0+ (If _Boost.Filesystem_ support is enabled)

## Simple example

```cpp
#include <asioext/file_handle.hpp>
#include <asioext/open_flags.hpp>

#include <asio/write.hpp>

#include <iostream>

int main(int argc, const char* argv[])
{
  try {
    asioext::scoped_file_handle file("myfile.txt",
                                     asioext::open_flags::access_write |
                                     asioext::open_flags::create_always);

    const std::string content = "Hello world";
    asio::write(file, asio::buffer(content));
    return 0;
  } catch (std::exception& e) {
    // Exceptions are used for error reporting here.
    // All functions also offer a non-throwing overload,
    // which takes an asio::error_code& instead.
    std::cerr << "error: " << e.what() << std::endl;
    return 1;
  }
}
```

## Documentation

The documentation can be found at http://timniederhausen.github.io/asio-extensions,
or inside the `gh-pages` branch.

Additionally, the documentation can be generated by building the special
`asioext.doc` target. Note that this target is only available if you have
[Doxygen](http://www.stack.nl/~dimitri/doxygen/) installed.

## License

Please see [LICENSE_1_0.txt](LICENSE_1_0.txt).
