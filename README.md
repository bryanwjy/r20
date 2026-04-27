# r20  
**Backporting C++23 Ranges to C++20**

`r20` is a lightweight, header-only library that backports selected C++23 `<ranges>` features to C++20.  
It fills the gap for developers who want to use modern range adaptors and views—without requiring the full C++23 toolchain or standard library support.

## ✨ Features

- Backports commonly used C++23 range adaptors and utilities  
  (e.g., `views::chunk`, `views::chunk_by`, `views::enumerate`, etc.)
- Implements missing C++20/23 range concepts where needed
- Header-only and dependency-free
- Works on major compilers supporting C++20:
  - GCC ≥ 12
  - Clang ≥ 19  (may work on Clang 16-18, but not tested)
  - MSVC (VS 2022) (Untested)

## 🔧 Installation

Simply add the `r20` include directory to your project:

```cpp
#include <rxx/ranges.h>
```

## 🧪 Testing

The comprehensive test suite for r20 is maintained in a separate repository to keep the core library lightweight. The tests are NOT supported on Windows.

👉 [r20-tests Repository](https://github.com/bryanwjy/r20-tests)

### Acknowledgments & Copyright

To ensure high reliability and parity with standard implementations, many tests in the suite are adapted from:

    The LLVM Project (libc++): Licensed under the Apache License v2.0 with LLVM Exceptions.

    GNU Toolchain (libstdc++): Licensed under the GNU General Public License v3.0 (or later) with Runtime Exceptions.

Running Tests

  1. Clone the [test repository](https://github.com/bryanwjy/r20-tests).

  2. Ensure you are using a supported C++20 compliant compiler.

  3. Run the following
  ```bash
  cd r20-tests
  RXX_SRC=/path/to/r20/repo make -j$(nproc) all
  ```
