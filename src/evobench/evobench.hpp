#pragma once

#include <algorithm>
#include <cstdint>
#include <mutex>
#include <stdexcept>
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

// The `_log_*` functions are used by the macros (quasi private); only
// call if is_enabled is true!

// `num_calls`: how many calls this log entry represents; it is the
// `every_n` parameter from `EVOBENCH_SCOPE_EVERY` or statically 1, or
// 0 when unknown (0 does not make sense as a value and is hence used
// as a null value; evobench-evaluator checks that it never uses 0 for
// spans, the spans always have the valid number from the start timing
// record, hence the macro doesn't need to specify the value when
// ending the scope).
void _log_any(const char* probe_name, PointKind kind, uint32_t num_calls);

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
      _log_any(probe_name, PointKind::T, 1);
   }
}

template <std::size_t N>
struct fixed_string {
   char data[N + 1];

   consteval size_t size() { return N; }

   consteval fixed_string()
       : data{} {};

   consteval fixed_string(std::string_view str)
       : data{} {
      if (str.size() != N) {
         throw std::invalid_argument("str is of wrong size");
      }
      std::copy_n(str.data(), N, data);
   }

   constexpr operator const char*() const { return data; }
};

template <size_t N1, size_t N2, size_t N3>
consteval fixed_string<N1 + N2 + N3> concat3(
   fixed_string<N1> str1,
   fixed_string<N2> str2,
   fixed_string<N3> str3
) {
   fixed_string<N1 + N2 + N3> result;
   std::copy_n(str1.data, N1, result.data);
   std::copy_n(str2.data, N2, result.data + N1);
   std::copy_n(str3.data, N3, result.data + N1 + N2);
   return result;
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
         _log_any(ProbeName, PointKind::TS, 1);
      }
   }
   inline ~Scope() {
      if (output.is_enabled) {
         // For consistency with `~ScopeEveryN`, just send 0 as the
         // `num_calls` value for this scope end, too (it doesn't
         // currently matter what we send as evobench-evaluator
         // ignores the value for end scope timings)
         _log_any(ProbeName, PointKind::TE, 0);
      }
   }
};

/// Same as `Scope` but only logs every `every_n` steps.
template <fixed_string ProbeName>
class ScopeEveryN {
   bool log_this_time;

  public:
   inline ScopeEveryN(uint32_t every_n, uint32_t* skip) {
      if (output.is_enabled) {
         if (*skip > 0) {
            (*skip)--;
            log_this_time = false;
         } else {
            *skip = every_n - 1;
            log_this_time = true;
            _log_any(ProbeName, PointKind::TS, every_n);
         }
      } else {
         log_this_time = false;
      }
   }
   inline ~ScopeEveryN() {
      if (output.is_enabled && log_this_time) {
         // There's no need to remember the `every_n` value, since
         // start and end timing records are always paired anyway,
         // thus just give the non-value 0 as the `num_calls` value.
         _log_any(ProbeName, PointKind::TE, 0);
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
#define EVOBENCH_SCOPE_EVERY(n, module, action)

#else

#define CONCAT_WITH_PIPE(left, right)                                                    \
   (evobench::concat3<std::string_view{left}.size(), 1, std::string_view{right}.size()>( \
      evobench::fixed_string<std::string_view{left}.size()>{std::string_view{left}},     \
      evobench::fixed_string<1>{"|"},                                                    \
      evobench::fixed_string<std::string_view{right}.size()>{std::string_view{right}}    \
   ))

#define EVOBENCH_SCOPE_INTERNAL(scope_name, line) \
   const evobench::Scope<scope_name> __evobench_scope##line{};
#define EVOBENCH_SCOPE_INTERNAL2(module, action, line) \
   EVOBENCH_SCOPE_INTERNAL(CONCAT_WITH_PIPE(module, action), line)
#define EVOBENCH_SCOPE(module, action) EVOBENCH_SCOPE_INTERNAL2(module, action, __LINE__)

#define EVOBENCH_SCOPE_EVERY_INTERNAL(n, scope_name, line)                 \
   static thread_local uint32_t __evobench_scope_every_n_skip_##line = 0;  \
   const evobench::ScopeEveryN<scope_name> __evobench_scope_everyn_##line{ \
      n, &__evobench_scope_every_n_skip_##line                             \
   };
#define EVOBENCH_SCOPE_EVERY_INTERNAL2(n, module, action, line) \
   EVOBENCH_SCOPE_EVERY_INTERNAL(n, CONCAT_WITH_PIPE(module, action), line)
#define EVOBENCH_SCOPE_EVERY(n, module, action) \
   EVOBENCH_SCOPE_EVERY_INTERNAL2(n, module, action, __LINE__)

#define EVOBENCH_POINT(module, action) evobench::log_point(CONCAT_WITH_PIPE(module, action));
#define EVOBENCH_KEY_VALUE(key, value) evobench::log_key_value(key, value)

#endif
