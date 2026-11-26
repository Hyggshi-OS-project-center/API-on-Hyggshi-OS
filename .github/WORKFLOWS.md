## GitHub Actions Status

| Workflow | Status |
| :--- | :--- |
| CI (Build & Test) | [![CI](../../actions/workflows/ci.yml/badge.svg)](../../actions/workflows/ci.yml) |
| Code Quality | [![Code Quality](../../actions/workflows/code-quality.yml/badge.svg)](../../actions/workflows/code-quality.yml) |
| Release | [![Release](../../actions/workflows/release.yml/badge.svg)](../../actions/workflows/release.yml) |

## Workflows

### `ci.yml` — Build & Test

Triggered on every push and pull request to `main`, `master`, or `develop`.

| Job | Description |
| :--- | :--- |
| `build-and-test` | Matrix build: **GCC** and **Clang** × **Debug** and **Release** on `ubuntu-24.04`; builds all libraries, examples, and tests; runs `ctest`; smoke-tests all 5 example binaries |
| `python-sdk` | Tests the Python `ctypes` SDK against Python 3.11, 3.12, and 3.13 |
| `lua-sdk` | Tests the LuaJIT FFI SDK |
| `typescript-sdk` | Tests the TypeScript/Node.js pure-proc adapter against Node.js 20 and 22 |
| `c-header-check` | Validates `hapi_c.h` compiles cleanly as C99, C11, and C17 |

### `code-quality.yml` — Static Analysis & Formatting

Triggered on every push and pull request to `main`, `master`, or `develop`.

| Job | Description |
| :--- | :--- |
| `clang-tidy` | Runs `clang-tidy` with readability, modernize, performance, bugprone, and analyzer checks on all C++ sources |
| `clang-format` | Checks all `.cpp`, `.hpp`, and `.h` files against `.clang-format` rules |
| `cmake-lint` | Lints `CMakeLists.txt` with `cmakelang` |

### `release.yml` — Release Packaging

Triggered on `v*.*.*` tags (e.g., `git tag v0.2.0 && git push --tags`).

Builds a `hapi-<version>-linux-x86_64.tar.gz` release archive containing:
- `libhapi_c.so` (public C ABI)
- `include/` (C++ and C headers)
- `sdk/python/`, `sdk/lua/`, `sdk/typescript/` (language bindings)
- `Documentation/`
- `README.md`, `LICENSE`

Publishes the archive as a GitHub Release asset automatically.
