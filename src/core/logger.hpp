#pragma once

#include <cstdint>
#include <format>
#include <source_location>
#include <string>
#include <string_view>
#include <utility>

// In-house, minimal logger. See docs/TECH.md "Logging" for the locked spec.
//
// Usage:
//   core::log::Init();                       // once, at startup (optional)
//   LOG_INFO("player hp now {}", hp);        // std::format-style messages
//   core::log::Shutdown();                   // once, at exit (flushes the file)
//
// Levels below kMinLevel are removed at compile time: the LOG_* macro expands to
// a discarded `if constexpr` branch, so neither the call nor its argument
// evaluation reach the binary. Release builds carry zero TRACE/DEBUG overhead.
namespace core::log {

  enum class Level : std::uint8_t {
    kTrace,
    kDebug,
    kInfo,
    kWarn,
    kError,
  };

  // Compile-time floor. Spec: DEBUG in debug builds, INFO in release. Define
  // DUN_LOG_MIN_LEVEL_TRACE to opt a single build into TRACE-level firehose.
#if defined(DUN_LOG_MIN_LEVEL_TRACE)
  inline constexpr Level kMinLevel = Level::kTrace;
#elif defined(NDEBUG)
  inline constexpr Level kMinLevel = Level::kInfo;
#else
  inline constexpr Level kMinLevel = Level::kDebug;
#endif

  struct Config {
    // Game name isn't locked yet (DESIGN: end of Week 2); this seeds the log
    // directory and file name and should change when the game is named.
    std::string app_name = "dungeon-crawler";
    bool log_to_console = true;
    bool log_to_file = true;
    // Runtime floor, applied on top of the compile-time kMinLevel. Lets a build
    // be quieted without recompiling; it can never re-enable compiled-out levels.
    Level runtime_level = kMinLevel;
    std::uintmax_t max_file_bytes = 5UL * 1024 * 1024;  // rotate past ~5 MiB
    int max_files = 5;                                  // rotated backups kept
  };

  // Resolves the platform log directory, creates it, and opens the active file
  // in append mode. Safe to skip — logging then goes straight to the console.
  // noexcept: a failed log must never propagate into game code.
  void Init(const Config& config = {}) noexcept;

  // Flushes and closes the active file. Logging after this falls back to console.
  void Shutdown() noexcept;

  namespace detail {
    // Non-template sink: all formatting/rotation/locking lives in logger.cpp.
    void WriteLine(Level level, const std::source_location& loc, std::string_view message);

    // Formats then sinks. noexcept and self-contained: std::format / the file
    // write can throw (bad_alloc), but a logger swallows its own failures rather
    // than taking down the caller. The std::format_string type checks the format
    // string against its args at compile time, so misuse never reaches here.
    template <typename... Args>
    void Write(Level level, const std::source_location& loc, std::format_string<Args...> fmt,
               Args&&... args) noexcept {
      try {
        WriteLine(level, loc, std::format(fmt, std::forward<Args>(args)...));
      } catch (...) {  // NOLINT(bugprone-empty-catch) -- a logger cannot log its own failure
      }
    }
  }  // namespace detail

}  // namespace core::log

// An immediately-invoked lambda makes the macro a single expression-statement:
// no dangling-else (unlike a bare `if constexpr`), and no do/while (which the
// cppcoreguidelines-avoid-do-while lint would flag at every call site). The
// `if constexpr` guard discards the body wholesale below the floor, so a
// compiled-out LOG_* never evaluates its arguments. The empty body folds away.
#define DUN_LOG(level, ...)                                                                \
  [&] {                                                                                    \
    if constexpr ((level) >= ::core::log::kMinLevel) {                                     \
      ::core::log::detail::Write((level), ::std::source_location::current(), __VA_ARGS__); \
    }                                                                                      \
  }()

#define LOG_TRACE(...) DUN_LOG(::core::log::Level::kTrace, __VA_ARGS__)
#define LOG_DEBUG(...) DUN_LOG(::core::log::Level::kDebug, __VA_ARGS__)
#define LOG_INFO(...) DUN_LOG(::core::log::Level::kInfo, __VA_ARGS__)
#define LOG_WARN(...) DUN_LOG(::core::log::Level::kWarn, __VA_ARGS__)
#define LOG_ERROR(...) DUN_LOG(::core::log::Level::kError, __VA_ARGS__)
