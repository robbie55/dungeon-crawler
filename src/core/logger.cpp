#include "logger.hpp"

#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <mutex>
#include <source_location>
#include <string>
#include <string_view>
#include <system_error>

namespace fs = std::filesystem;

namespace core::log {
  namespace {

    // All shared state hangs off one function-local static (see State() below),
    // not a namespace-scope global — keeps the singleton without tripping the
    // cppcoreguidelines no-mutable-global rule.
    struct LoggerState {
      std::mutex mutex;
      std::ofstream file;
      fs::path log_path;
      Config config;
      std::uintmax_t current_bytes = 0;
    };

    LoggerState& State() {
      static LoggerState state;
      return state;
    }

    std::string_view LevelName(Level level) {
      switch (level) {
        case Level::kTrace:
          return "TRACE";
        case Level::kDebug:
          return "DEBUG";
        case Level::kInfo:
          return "INFO";
        case Level::kWarn:
          return "WARN";
        case Level::kError:
          return "ERROR";
      }
      return "?????";
    }

    // [timestamp] uses local wall-clock with millisecond precision. localtime is
    // not reentrant, so we use the platform reentrant variant; the whole write is
    // serialized by the logger mutex regardless.
    std::string Timestamp() {
      const auto kNow = std::chrono::system_clock::now();
      const auto kSecs = std::chrono::time_point_cast<std::chrono::seconds>(kNow);
      const auto kMillis =
          std::chrono::duration_cast<std::chrono::milliseconds>(kNow - kSecs).count();

      const std::time_t kRaw = std::chrono::system_clock::to_time_t(kNow);
      std::tm local{};
#ifdef _WIN32
      localtime_s(&local, &kRaw);
#else
      localtime_r(&kRaw, &local);
#endif
      std::array<char, 20> buf{};
      std::strftime(buf.data(), buf.size(), "%Y-%m-%d %H:%M:%S", &local);
      return std::format("{}.{:03}", buf.data(), kMillis);
    }

    // Per-platform user log directory. Spec targets Windows; the macOS/Linux
    // branches keep the logger usable on dev machines. Falls back to ./logs.
    fs::path ResolveLogDir(const std::string& app_name) {
#ifdef _WIN32
      const char* base = std::getenv("LOCALAPPDATA");  // NOLINT(concurrency-mt-unsafe) -- init-time
      if (base != nullptr) {
        return fs::path(base) / app_name / "logs";
      }
#elif defined(__APPLE__)
      const char* home = std::getenv("HOME");  // NOLINT(concurrency-mt-unsafe) -- init-time only
      if (home != nullptr) {
        return fs::path(home) / "Library" / "Logs" / app_name;
      }
#else
      const char* home = std::getenv("HOME");  // NOLINT(concurrency-mt-unsafe) -- init-time only
      if (home != nullptr) {
        return fs::path(home) / ".local" / "state" / app_name / "logs";
      }
#endif
      return {"logs"};
    }

    void OpenActiveFile(LoggerState& state, bool truncate) {
      const auto kMode =
          truncate ? (std::ios::out | std::ios::trunc) : (std::ios::out | std::ios::app);
      state.file.open(state.log_path, kMode);

      std::error_code ec;
      const auto kSize = fs::file_size(state.log_path, ec);
      state.current_bytes = (ec || truncate) ? 0 : kSize;
    }

    // Size-based rotation: app.log -> app.1.log -> ... -> app.N.log, oldest dropped.
    void RotateFiles(LoggerState& state) {
      state.file.close();
      const std::string& name = state.config.app_name;
      const fs::path kDir = state.log_path.parent_path();
      std::error_code ec;

      fs::remove(kDir / std::format("{}.{}.log", name, state.config.max_files), ec);
      for (int i = state.config.max_files - 1; i >= 1; --i) {
        fs::rename(kDir / std::format("{}.{}.log", name, i),
                   kDir / std::format("{}.{}.log", name, i + 1), ec);
      }
      fs::rename(state.log_path, kDir / std::format("{}.1.log", name), ec);

      OpenActiveFile(state, /*truncate=*/true);
    }

  }  // namespace

  void Init(const Config& config) noexcept {
    try {
      LoggerState& state = State();
      const std::scoped_lock kLock(state.mutex);
      state.config = config;

      if (!config.log_to_file) {
        return;
      }

      const fs::path kDir = ResolveLogDir(config.app_name);
      std::error_code ec;
      fs::create_directories(kDir, ec);
      if (ec) {
        // Couldn't make the directory — degrade to console-only rather than crash.
        state.config.log_to_file = false;
        return;
      }

      state.log_path = kDir / (config.app_name + ".log");
      OpenActiveFile(state, /*truncate=*/false);
    } catch (...) {  // NOLINT(bugprone-empty-catch) -- a logger cannot log its own failure
    }
  }

  void Shutdown() noexcept {
    try {
      LoggerState& state = State();
      const std::scoped_lock kLock(state.mutex);
      if (state.file.is_open()) {
        state.file.flush();
        state.file.close();
      }
    } catch (...) {  // NOLINT(bugprone-empty-catch) -- a logger cannot log its own failure
    }
  }

  namespace detail {

    void WriteLine(Level level, const std::source_location& loc, std::string_view message) {
      LoggerState& state = State();
      const std::scoped_lock kLock(state.mutex);

      if (level < state.config.runtime_level) {
        return;
      }

      const std::string kFile = fs::path(loc.file_name()).filename().string();
      const std::string kLine = std::format("[{}] [{}] [{}:{}] {}\n", Timestamp(), LevelName(level),
                                            kFile, loc.line(), message);

      if (state.config.log_to_console) {
        (level >= Level::kWarn ? std::cerr : std::cout) << kLine;
      }

      if (state.config.log_to_file && state.file.is_open()) {
        state.file << kLine;
        state.current_bytes += kLine.size();
        if (state.current_bytes >= state.config.max_file_bytes) {
          RotateFiles(state);
        }
      }
    }

  }  // namespace detail

}  // namespace core::log
