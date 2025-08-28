#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "evobench/evobench.hpp"

#include <charconv>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <time.h>
#include <unistd.h>

// Getting thread statistics on macOS
#ifdef __MACH__
#include <mach/mach.h>

int mac_get_thread_info(struct timeval& utime, struct timeval& stime) {
   thread_basic_info tinfo;
   mach_msg_type_number_t count = THREAD_BASIC_INFO_COUNT;
   kern_return_t kr =
      thread_info(mach_thread_self(), THREAD_BASIC_INFO, (thread_info_t)&tinfo, &count);
   if (kr == KERN_SUCCESS) {
      utime.tv_sec = tinfo.user_time.seconds;
      utime.tv_usec = tinfo.user_time.microseconds;
      stime.tv_sec = tinfo.system_time.seconds;
      stime.tv_usec = tinfo.system_time.microseconds;
      return 0;
   }
   // Does thread_info set errno?
   return -1;
}

uint64_t our_get_thread_id() {
   uint64_t thread_id;
   pthread_threadid_np(NULL, &thread_id);
   return thread_id;
}

#else

// Linux
__pid_t our_get_thread_id() {
   return gettid();
}

#endif

// Necessary on macOS
#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 512
#endif
#ifndef LOGIN_NAME_MAX
#define LOGIN_NAME_MAX 512
#endif

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define WARN(expr)                                                            \
   do {                                                                       \
      std::cerr << expr;                                                      \
      std::cerr << " at " __FILE__ ":" TOSTRING(__LINE__) "\n" << std::flush; \
   } while (0)

// Macros for outputting JSON as "expressions", but expanding to
// side-effecting code, taking an implicit `out` std::string&
// parameter. (Hoping the compiler optimizes multiple constant string
// outputs into one.)

/// A single atomic value that has an implementation of the `js_print`
/// function.
#define ATOM(expr) js_print(expr, out);

/// Slow fallback for now, XX optimize later.
#define SLOW(expr) js_print_slow(expr, out);

/// A JSON object
#define OBJ(expr)      \
   out.push_back('{'); \
   expr;               \
   out.push_back('}');
/// A single key-value pair; key is implicitly `ATOM`-wrapped, val
/// must be wrapped explicitly.
#define KV(key, val)   \
   ATOM(key);          \
   out.push_back(':'); \
   val;
/// Separator between `KV` in `OBJ`.
#define COMMA out.push_back(',');

#define ARRAY2(val1, val2) \
   out.push_back('[');     \
   val1;                   \
   out.push_back(',');     \
   val2;                   \
   out.push_back(']');

/// Separator between records
#define NEWLINE out.push_back('\n');

namespace {
// Slow fallback to be used via `SLOW`; e.g. for `pthread_t`.
// XX future: provide js_print overloads for various integer types
// if needed for optimization
template <typename T>
void js_print_slow(T val, std::string& out) {
   std::ostringstream tmpout;
   tmpout << val;
   out.append(tmpout.str());
}

[[maybe_unused]] void js_print(uint32_t value, std::string& out) {
   char buffer[11];
   auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value);
   if (ec == std::errc()) {
      out.append(buffer, ptr);
   } else {
      abort();
   }
}

[[maybe_unused]] void js_print(int32_t value, std::string& out) {
   char buffer[12];
   auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value);
   if (ec == std::errc()) {
      out.append(buffer, ptr);
   } else {
      abort();
   }
}

[[maybe_unused]] void js_print(uint64_t value, std::string& out) {
   char buffer[21];
   auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value);
   if (ec == std::errc()) {
      out.append(buffer, ptr);
   } else {
      abort();
   }
}

[[maybe_unused]] void js_print(int64_t value, std::string& out) {
   char buffer[22];
   auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value);
   if (ec == std::errc()) {
      out.append(buffer, ptr);
   } else {
      abort();
   }
}

void js_print(const std::string_view input, std::string& out) {
   out.push_back('"');
   for (char ch : input) {
      switch (ch) {
         case '\"':
            out.append("\\\"");
            break;
         case '\\':
            out.append("\\\\");
            break;
         case '\b':
            out.append("\\b");
            break;
         case '\f':
            out.append("\\f");
            break;
         case '\n':
            out.append("\\n");
            break;
         case '\r':
            out.append("\\r");
            break;
         case '\t':
            out.append("\\t");
            break;
         default:
            if (ch < 32) {
               // XX slow; but rare, so ok?
               std::ostringstream tmpout;
               tmpout << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                      << static_cast<int>(static_cast<unsigned char>(ch));
               out.append(tmpout.str());
            } else {
               // Including characters 0x80+, which are
               // multi-byte UTF-8 code points in the input
               out.push_back(ch);
            }
      }
   }
   out.push_back('"');
}

void js_print(const timespec& t, std::string& out) {
   OBJ(KV("sec", ATOM(static_cast<int64_t>(t.tv_sec)))
          COMMA KV("nsec", ATOM(static_cast<int64_t>(t.tv_nsec))))
}

void js_print(const timeval& t, std::string& out) {
   OBJ(KV("sec", ATOM(static_cast<int64_t>(t.tv_sec))) COMMA KV("usec", ATOM(t.tv_usec)))
}

// Keep in sync with `PointKind` ! XX better solution?
const char* point_kind_name[8] =
   {"TStart", "T", "TS", "TE", "TThreadStart", "TThreadEnd", "TEnd", "TIO"};

#define ERRCHECK(expr)                                              \
   do {                                                             \
      auto result = (expr);                                         \
      if (result < 0) {                                             \
         perror(#expr " at file " __FILE__ ":" TOSTRING(__LINE__)); \
         return;                                                    \
      }                                                             \
   } while (0)

void log_start(std::string& out) {
   OBJ(
      KV("Start",
         OBJ(KV("evobench_log_version", SLOW(evobench::EVOBENCH_LOG_VERSION))
                COMMA KV("evobench_version", ATOM(EVOBENCH_VERSION))))
   );
   NEWLINE;

   char hostname[HOST_NAME_MAX];
   char username[LOGIN_NAME_MAX];
   ERRCHECK(gethostname(hostname, HOST_NAME_MAX));
   ERRCHECK(getlogin_r(username, LOGIN_NAME_MAX));

   struct utsname os_release;
   ERRCHECK(uname(&os_release));

   std::ostringstream compiler;
#if defined(__clang__)
   compiler << "Clang " << __clang_major__ << "." << __clang_minor__ << "." << __clang_patchlevel__;
#elif defined(__GNUC__)
   compiler << "GCC " << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__;
#elif defined(_MSC_VER)
   compiler << "MSVC " << _MSC_VER;
#else
   compiler << "Unknown";
#endif

   // clang-format off
   OBJ(KV("Metadata",
	  OBJ(KV("username", ATOM(username))
	      COMMA
	      KV("hostname", ATOM(hostname))
	      COMMA
	      KV("uname",
		 OBJ(KV("sysname", ATOM(os_release.sysname))
		     COMMA
		     KV("nodename", ATOM(os_release.nodename))
		     COMMA
		     KV("release", ATOM(os_release.release))
		     COMMA
		     KV("version", ATOM(os_release.version))
		     COMMA
		     KV("machine", ATOM(os_release.machine))))
	      COMMA
	      KV("compiler", ATOM(compiler.str())))));
   NEWLINE;
   // clang-format on
}

void log_resource_usage(
   const char* probe_name,
   evobench::PointKind kind,
   std::string& out,
   uint32_t num_calls
) {
   struct timespec t;
   ERRCHECK(clock_gettime(CLOCK_REALTIME, &t));

   struct timeval utime;
   struct timeval stime;
#ifdef __MACH__
   ERRCHECK(mac_get_thread_info(utime, stime));
#else
   struct rusage r;
   ERRCHECK(getrusage(RUSAGE_THREAD, &r));
   utime = r.ru_utime;
   stime = r.ru_stime;
#endif

   // clang-format off
   OBJ(KV(point_kind_name[kind],
	  OBJ(KV("pn", ATOM(probe_name))
	      COMMA
	      KV("pid", ATOM(getpid()))
	      COMMA
	      KV("tid", ATOM(our_get_thread_id()))
	      COMMA
	      KV("n", ATOM(num_calls))
	      COMMA
	      KV("r", ATOM(t))
	      COMMA
	      KV("u", ATOM(utime))
	      COMMA
	      KV("s", ATOM(stime))
#ifndef __MACH__
	      COMMA
	      // Some of these are per process, but when do we
	      // want to know, still per thread action?
	      KV("maxrss", ATOM(r.ru_maxrss))
	      COMMA
	      // KV("ixrss", ATOM(r.ru_ixrss)) -- This field is currently unused on Linux.
	      // COMMA
	      // KV("idrss", ATOM(r.ru_idrss)) -- This field is currently unused on Linux.
	      // COMMA
	      // KV("isrss", ATOM(r.ru_isrss)) -- This field is currently unused on Linux.
	      // COMMA
	      KV("minflt", ATOM(r.ru_minflt))
	      COMMA
	      KV("majflt", ATOM(r.ru_majflt))
	      COMMA
	      // KV("nswap", ATOM(r.ru_nswap)) -- This field is currently unused on Linux.
	      // COMMA
	      KV("inblock", ATOM(r.ru_inblock))
	      COMMA
	      KV("oublock", ATOM(r.ru_oublock))
	      COMMA
	      // KV("msgsnd", ATOM(r.ru_msgsnd)) -- This field is currently unused on Linux.
	      // COMMA
	      // KV("msgrcv", ATOM(r.ru_msgrcv)) -- This field is currently unused on Linux.
	      // COMMA
	      // KV("nsignals", ATOM(r.ru_nsignals)) -- This field is currently unused on Linux.
	      // COMMA
	      KV("nvcsw", ATOM(r.ru_nvcsw))
	      COMMA
	      KV("nivcsw", ATOM(r.ru_nivcsw))
#endif
	      )));
   NEWLINE;
   // clang-format on
}

/// Slow, but we're not using it in a fast path
template <typename T>
void js_print_stream(T val, std::ostream& out) {
   std::string tmpout{};
   js_print(val, tmpout);
   out << tmpout;
}

}  // namespace

namespace evobench {

Output::Output(const char* maybe_output_path) {
   path = maybe_output_path;
   if (!maybe_output_path) {
      is_enabled = false;
      return;
   }

   lock_fd = open(path, O_CREAT, 0666);
   if (lock_fd < 0) {
      auto err = errno;
      std::cerr << "evobench::Output: can't create or open file for locking: ";
      js_print_stream(std::string_view{path}, std::cerr);
      std::cerr << ": " << std::strerror(err) << "\n";
      exit(1);
   }
   if (flock(lock_fd, LOCK_EX | LOCK_NB) < 0) {
      auto err = errno;
      std::cerr << "evobench::Output: can't lock file: ";
      js_print_stream(std::string_view{path}, std::cerr);
      std::cerr << ": " << std::strerror(err) << "\n";
      exit(1);
   }

   fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0666);
   if (fd < 0) {
      auto err = errno;
      std::cerr << "evobench::Output: can't open file for writing: ";
      js_print_stream(std::string_view{path}, std::cerr);
      std::cerr << ": " << std::strerror(err) << "\n";
      exit(1);
   }
   is_enabled = true;
   {
      // Allocate a temporary buffer that doesn't announce the
      // thread, and write the benchmark file header.
      Buffer buffer{true};
      log_start(buffer.string);
      log_resource_usage("-", PointKind::TStart, buffer.string, 1);
   }
}

Output::~Output() {
   // Should only be called once all threads have exited? Or
   // how does this work? XXX otherwise will have to flush
   // all thread-local buffers, or? Simply do not close?
   if (is_enabled) {
      {
         Buffer out{true};
         log_resource_usage("-", PointKind::TEnd, out.string, 1);
      }
      is_enabled = false;
   }
}

void Output::write_all(const std::string_view buffer) {
   // Take lock before checking is_enabled, as a way to avoid
   // being knowledgeable enough about how atomics and
   // mutexes interact.
   std::lock_guard<std::mutex> guard(write_mutex);
   if (is_enabled) {
      ssize_t to_be_written = buffer.length();
      ssize_t written = 0;
      while (to_be_written > 0) {
         auto res = write(fd, buffer.data() + written, to_be_written);
         if (res < 0) {
            auto err = errno;
            std::cerr << "evobench::Output::write_all: ";
            js_print_stream(std::string_view{path}, std::cerr);
            std::cerr << ": " << std::strerror(err) << "\n";
            is_enabled = false;
            return;
         }
         to_be_written -= res;
         written += res;
      }
   }
}

Output output{getenv("EVOBENCH_LOG")};

void Buffer::flush() {
   output.write_all(string);
   string.clear();
}

bool Buffer::possibly_flush() {
   if (string.length() > BUF_MAX_SIZE) {
      flush();
      return true;
   }
   return false;
}

Buffer::Buffer(bool is_manual_)
    : is_manual(is_manual_) {
   if (!is_manual) {
      _log_any("-", PointKind::TThreadStart, 1);
   }
}

Buffer::~Buffer() {
   if (output.is_enabled) {
      if (!is_manual) {
         _log_any("-", PointKind::TThreadEnd, 0);
      }
      flush();
   }
}

thread_local Buffer local_buffer{false};

void _log_key_value(std::string_view key, std::string_view value) {
   std::string& out = local_buffer.string;

   OBJ(KV(
      "KeyValue",
      OBJ(KV("tid", SLOW(our_get_thread_id())) COMMA KV("k", ATOM(key)) COMMA KV("v", ATOM(value)))
   ));
   NEWLINE;
}

void _log_any(const char* probe_name, PointKind kind, uint32_t num_calls) {
   log_resource_usage(probe_name, kind, local_buffer.string, num_calls);
   if (local_buffer.possibly_flush()) {
      log_resource_usage(probe_name, PointKind::TIO, local_buffer.string, 1);
   }
}

}  // namespace evobench
