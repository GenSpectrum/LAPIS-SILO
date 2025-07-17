#pragma once

#include <algorithm>
#include <mutex>
#include <string>
#include <string_view>

#include "evobench_version.hpp"

namespace evobench {
// If you change this, you'll want to update `log_message.rs` to
// have a new protocol variant that mirrors the C++ side, for that
// same `EVOBENCH_LOG_VERSION` value.
const uint32_t EVOBENCH_LOG_VERSION = 1;

#define pub
#include "_evobench_point_kind.hpp"
;
#undef pub

class Output {
   // Only if maybe_output_path was true. Stays with that value
   // even if `is_enabled` is turned off later.
   const char* path;
   // Opened for reading for holding a lock
   int lock_fd;
   // Opened for writing
   int fd;
   std::mutex write_mutex;

   friend class Buffer;
   void write_all(const std::string_view buffer);

  public:
   // This is turned to false on errors, too. It is public for
   // performance (inlined access from `log_point` function).  XX
   // use an atomic
   bool is_enabled;

   /// Opens the file at `maybe_output_path`. Exits the process
   /// with status 1 on errors (this is run during init, better
   /// don't throw?). Logs a `TStart` message.
   Output(const char* maybe_output_path);

   /// Logs a `TEnd` message.
   ~Output();
};

extern Output output;

/// A thread-local output buffer. Uses the global
/// `evobench::output` `Output` instance for writing.
class Buffer {
   const size_t BUF_MAX_SIZE = 8192;

   /// Whether constructor and destructor should remain silent;
   /// used by `Output` for temporary buffers to do start and end
   /// of process logging.
   bool is_manual;

  public:
   /// The buffer contents
   std::string string;

   void flush();

   /// Flushes if the buffer is overly full. Returns true if it
   /// did flush.
   bool possibly_flush();

   /// Writes a `TThreadStart` message, unless `is_manual` or
   /// output is not enabled.
   Buffer(bool is_manual_);

   /// Writes a `TThreadEnd` message, unless `is_manual` or
   /// output is not enabled.
   ~Buffer();
};

// These are private; only call if is_enabled is true!
void _log_any(const char* probe_name, PointKind kind);
void _log_key_value(std::string_view key, std::string_view value);

/// Log a key value pair, without timings, for information
/// tracking.
inline static void log_key_value(std::string_view key, std::string_view value) {
   if (output.is_enabled) {
      _log_key_value(key, value);
   }
}

/// Log at the point of call as a single "T" event.
inline static void log_point(const char* probe_name) {
   if (output.is_enabled) {
      _log_any(probe_name, PointKind::T);
   }
}

template <std::size_t N>
struct fixed_string {
   char data[N];

   consteval fixed_string(std::string_view str) { std::copy_n(str.data(), str.size(), data); }

   constexpr operator const char*() const { return data; }
};

consteval std::string concat3(std::string_view str1, std::string_view str2, std::string_view str3) {
   std::string res{str1};
   res += str2;
   res += str3;
   return res;
}

/// Log at the object creation as "TS" and its destruction as
/// "TE" event.
// Keep this object small, to keep overhead low when
// !output.is_enabled.
template <fixed_string ProbeName>
class Scope {
  public:
   inline Scope() {
      if (output.is_enabled) {
         _log_any(ProbeName, PointKind::TS);
      }
   }
   inline ~Scope() {
      if (output.is_enabled) {
         _log_any(ProbeName, PointKind::TE);
      }
   }
};

}  // namespace evobench

/// `NO_EVOBENCH` disables the probe points; the logfile will still be
/// written but with just the TStart and TEnd points.
#ifdef NO_EVOBENCH
#define EVOBENCH_SCOPE(module, action)
#define EVOBENCH_POINT(module, action)
#define EVOBENCH_KEY_VALUE(key, value)
#else
#define EVOBENCH_SCOPE_INTERNAL(scope_name) \
   evobench::Scope<evobench::fixed_string<scope_name.size()>{scope_name.data()}> __evobench_scope{};
#define EVOBENCH_SCOPE(module, action) \
   EVOBENCH_SCOPE_INTERNAL(evobench::concat3(module, "|", action))
#define EVOBENCH_POINT(module, action) \
   evobench::log_point(evobench::concat3(module, "|", action).data());
#define EVOBENCH_KEY_VALUE(key, value) evobench::log_key_value(key, value)
#endif
