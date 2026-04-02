# Subconverter Agent Guidelines

This document provides guidelines for AI agents working on the subconverter codebase.

## 1. Build, Lint, and Test

### Development Environment

**⚠️ NixOS Users:** Always enter the Nix shell environment **in project root** before any build/test operations:
```bash
nix-shell
```

The `shell.nix` in project root provides:
- **Build tools:** gcc, cmake, ninja, pkg-config, clang-tools, gdb
- **Dependencies:** curlFull, rapidjson, toml11, yaml-cpp, pcre2, quickjspp, libcron

### Build System
- **Language:** C++20
- **Build System:** CMake (Version 3.5+)
- **Dependencies:**
  - `curl` (with mbedTLS preferred for static builds)
  - `yaml-cpp` (>=0.6.3)
  - `quickjspp` (specific commit `0c00c48` recommended in scripts)
  - `libcron`
  - `toml11` (v4.3.0)
  - `pcre2`
  - `rapidjson`
  - `libevent` (optional/commented out)

### Build Commands

**Local development build:**
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

**Static library build (without JS runtime and webget):**
```bash
mkdir build && cd build
cmake -DBUILD_STATIC_LIBRARY=ON ..
make -j$(nproc)
```

**With malloc_trim for memory optimization:**
```bash
cmake -DUSING_MALLOC_TRIM=ON ..
```

**Release builds (reference CI scripts):**
- Linux: `scripts/build.alpine.release.sh`
- macOS: `scripts/build.macos.release.sh`
- Windows: `scripts/build.windows.release.sh`

**Default compile flags:** `-Wall -Wextra -Wno-unused-parameter -Wno-unused-result`

### Testing
- **Status:** No automated tests exist in this project
- **Manual verification required:**
  - Run the built `subconverter` executable with test configurations
  - Test subscription conversion: `./subconverter -url <subscription_url> -target clash`
  - Check logs for errors or warnings
  - Validate output config files for correctness
- **Do not run** `make test` or `ctest` - not configured

### Linting & Formatting
- **Status:** No `.clang-format`, `.clang-tidy`, or similar tools configured
- **Guideline:** Match existing code style in the file you're editing

## 2. Code Style Guidelines

### General
- **Indentation:** 4 spaces (no tabs)
- **Line Endings:** Unix (`\n`)
- **File Encoding:** UTF-8
- **Line Length:** Generally under 120 characters, but not strictly enforced

### Naming Conventions
- **Variables:** Mixed style - prefer `camelCase` or `snake_case` matching the file's pattern
  - Example: `webServer`, `global.listenPort`, `nodeCount`, `proxy_config`
  - Avoid Hungarian notation in new code (legacy `szTemp` exists)
- **Functions:** Mixed - use existing pattern in the file
  - Global: `camelCase` (e.g., `chkArg`, `webGet`) or `snake_case` (e.g., `importItems`)
  - Class methods: `snake_case` (e.g., `stop_web_server`) or `camelCase`
- **Classes:** `PascalCase` (e.g., `WebServer`, `Proxy`, `Settings`)
- **Macros/Constants:** `UPPER_CASE` with underscores (e.g., `LOG_LEVEL_INFO`, `RULESET_CLASH_DOMAIN`)
- **Enums:** `UPPER_CASE` values
- **File names:** `snake_case` (e.g., `subexport.cpp`, `logger.h`)

### Header Guards
- Use `_INCLUDED` suffix: `#ifndef FILENAME_H_INCLUDED`
- Closing comment: `#endif // FILENAME_H_INCLUDED`

### Formatting
- **Braces:** Allman style (opening brace on new line)
  ```cpp
  void function()
  {
      if (condition)
      {
          // code
      }
      else
      {
          // other code
      }
  }
  ```
- **Include order:** Standard library headers → System headers → Local headers
  ```cpp
  #include <string>
  #include <vector>
  #include <unistd.h>
  #include "utils/logger.h"
  #include "handler/settings.h"
  ```
- **Spacing:** Space after keywords (`if (condition)`, `while (true)`), no space inside parentheses
- **Pointers/References:** `Type* name` or `Type &name` (style varies - match file)

### Types and Declarations
- **Function parameters:** Use `const Type&` for non-modifiable parameters
- **Return values:** Prefer returning by value for small types, use references/pointers carefully
- **Class members:** Use `private` by default, public only when needed
- **Structs:** Use for simple data containers without invariants

### Error Handling
- **Primary approach:** Return codes (int, bool, or special values)
- **Logging:** Use `writeLog(type, content, level)` from `utils/logger.h`
  - Types: `LOG_TYPE_INFO`, `LOG_TYPE_ERROR`, `LOG_TYPE_WARN`, `LOG_TYPE_FILEDL`, etc.
  - Levels: `LOG_LEVEL_FATAL`, `LOG_LEVEL_ERROR`, `LOG_LEVEL_WARNING`, `LOG_LEVEL_INFO`, `LOG_LEVEL_DEBUG`, `LOG_LEVEL_VERBOSE`
  - Example: `writeLog(0, "Failed to parse config", LOG_LEVEL_ERROR);`
- **Exceptions:** Rarely used; prefer return codes and logging
- **Assertions:** Use sparingly for invariants that should never fail

### Memory Management
- Prefer RAII and standard containers (`std::vector`, `std::string`)
- Use `std::move()` and `emplace_back()` for efficiency
- Manual `delete` exists in legacy code; use smart pointers for new code
- Enable `USING_MALLOC_TRIM` for long-running processes to free memory

### Conditional Compilation
- Use `#ifndef NO_JS_RUNTIME` for QuickJS-dependent code
- Use `#ifndef NO_WEBGET` for network-dependent code
- Platform-specific: `#ifdef WIN32`, `#ifdef __APPLE__`, etc.

### Global Variables
- Declare with `extern` in headers: `extern Settings global;`
- Define in one source file: `Settings global;`
- Prefer avoiding new globals; use dependency injection where possible

## 3. Project Structure

- `src/generator/` - Config generation logic (Clash, Surge, SingBox, etc.)
- `src/handler/` - HTTP request handlers, settings management
- `src/parser/` - Subscription format parsers
- `src/server/` - Web server implementation (currently httplib-based)
- `src/utils/` - Utility functions (string, network, file, logging)
- `src/script/` - QuickJS runtime for custom scripts
- `base/` - Default config templates and assets
- `scripts/` - Build scripts for different platforms

## 4. Common Tasks

### Adding a new conversion target
1. Add `proxyToNewTarget()` function in `src/generator/config/subexport.cpp/h`
2. Add target case in `src/handler/interfaces.cpp` subconverter()
3. Add base config option in `src/handler/settings.h` and loader functions
4. Add example config in `base/pref.example.*` files

### Adding configuration options
1. Add field to `Settings` struct in `src/handler/settings.h`
2. Add parsing logic in `readConf()`, `readTOMLConf()`, or `readYAMLConf()`
3. Add to example config files in `base/`
4. Document usage in comments

### Updating dependencies
1. Modify version in `scripts/build.*.release.sh`
2. Check CMakeLists.txt for include path changes
3. Verify compatibility with existing code
4. Test on all target platforms if possible
