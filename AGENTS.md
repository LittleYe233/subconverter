# Subconverter Agent Guidelines

This document provides guidelines for AI agents working on the subconverter codebase.

## 1. Build, Lint, and Test

### Build System
- **Language:** C++20
- **Build System:** CMake (Version 3.5+)
- **Dependencies:**
  - `curl` (with mbedTLS preferred for static builds)
  - `yaml-cpp`
  - `quickjspp` (specific commit `0c00c48` recommended in scripts)
  - `libcron`
  - `toml11` (v4.3.0)
  - `pcre2`
  - `rapidjson`
  - `libevent` (optional/commented out)

### Build Commands
To build the project locally:

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

For a static release build (similar to CI):
Refer to `scripts/build.alpine.release.sh` for exact dependency versions and linking flags.

### Testing
- **Status:** There are currently **NO** automated tests in the codebase or CI pipelines.
- **Action:**
  - When modifying code, verify changes manually by running the `subconverter` executable.
  - If adding new features, consider adding a basic test framework (e.g., Google Test or Catch2) if permitted, or create a simple shell script to verify output.
  - **Do not** attempt to run `make test` or `ctest` as they are not configured.

### Linting & Formatting
- **Status:** No strict linting or formatting configuration (`.clang-format`, `.clang-tidy`) exists in the repository.
- **Guideline:**
  - Follow the existing code style (see below).
  - Avoid reformatting entire files to minimize diff noise.

## 2. Code Style Guidelines

### General
- **Indentation:** 4 spaces.
- **Line Endings:** Unix (`\n`).
- **File Encoding:** UTF-8.

### Naming Conventions
- **Variables:** Mixed. Often `camelCase` (e.g., `webServer`, `global`) or `snake_case` (e.g., `signal_handler`).
  - Member variables often use `camelCase` (e.g., `global.listenPort`).
  - Some legacy Hungarian notation exists (e.g., `szTemp`), but avoid for new code.
- **Functions:** Mixed.
  - Global functions: `camelCase` (e.g., `chkArg`) or `snake_case` (e.g., `cron_tick_caller`).
  - Class methods: `snake_case` (e.g., `stop_web_server`) or `camelCase`.
  - **Recommendation:** Match the style of the file/class you are editing.
- **Classes:** `PascalCase` (e.g., `WebServer`).
- **Macros/Constants:** `UPPER_CASE` (e.g., `LOG_LEVEL_INFO`).

### Formatting
- **Braces:** Allman style (opening brace on a new line) is prevalent.
  ```cpp
  void function()
  {
      if (condition)
      {
          // code
      }
  }
  ```
- **Includes:**
  - Standard library headers first (e.g., `<iostream>`).
  - System headers (e.g., `<unistd.h>`).
  - Local headers (e.g., `"config/ruleset.h"`).

### Error Handling
- The project uses a mix of return codes and logging.
- **Logging:** Use `writeLog(type, content, level)` from `utils/logger.h`.
  - Example: `writeLog(0, "Message", LOG_LEVEL_ERROR);`
- **Exceptions:** Not heavily used; prefer error codes or logging for recoverable errors.

### Project Structure
- `src/`: Source code.
  - `handler/`: Request handlers (webget, settings, etc.).
  - `generator/`: Config generation logic.
  - `parser/`: Subscription parsers.
  - `server/`: Web server implementation.
  - `utils/`: Utility functions (string, network, file).
- `base/`: Default configuration files and assets.
- `scripts/`: Build and utility scripts.

## 3. Common Tasks & Workflow

### Adding a New Feature
1.  **Identify the Component:** Determine if the feature belongs in `handler/` (API logic), `parser/` (input format), or `generator/` (output format).
2.  **Implementation:**
    - Create new files in the appropriate directory if the feature is large.
    - Update `CMakeLists.txt` to include new source files.
    - Follow the existing code style.
3.  **Verification:**
    - Build the project locally.
    - Run the executable with test arguments or configuration.
    - Verify the output manually.

### Updating Dependencies
- Dependencies are managed manually in `scripts/build.alpine.release.sh` (and other platform scripts).
- To update a dependency:
    1.  Modify the version/tag in the build script.
    2.  Verify compatibility with the codebase.
    3.  Update `CMakeLists.txt` if include paths or library names change.

### Git Workflow
- **Branching:** Create feature branches for changes.
- **Commits:** Use clear, descriptive commit messages.
- **Pull Requests:** Describe the changes and manual verification steps performed.

## 4. Troubleshooting

### Common Build Errors
- **Missing Headers:** Ensure all dependencies are installed. The project relies on system-installed libraries for dynamic builds or manually compiled ones for static builds.
- **Linker Errors:** Check `CMakeLists.txt` for missing libraries or incorrect paths.
- **C++ Standard:** Ensure your compiler supports C++20.

### Runtime Issues
- **Segfaults:** Use `gdb` to debug.
  ```bash
  gdb ./subconverter
  run
  bt
  ```
- **Config Errors:** Check `base/pref.toml` or `base/pref.ini` for syntax errors. The application logs errors to the console or log file.
