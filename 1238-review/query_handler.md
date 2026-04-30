# PR #1238 — `query_handler.cpp` / `query_handler.h` Review

## Summary

Endpoint reads SaneQL body, plans query, streams results as NDJSON or Arrow IPC based on Accept header. Error handling delegates to `ErrorRequestHandler` wrapper. Overall structure solid — findings below.

---

## Findings

### query_handler.cpp

`L3: 🔵 nit: #include <cxxabi.h> unused. Only used in error_request_handler.cpp. Remove.`

`L11: 🔵 nit: #include <nlohmann/json.hpp> unused. No JSON usage in this file. Remove.`

`L12: 🔵 nit: #include <utility> misplaced — system include in external-includes group. Move to group 2 (after <string>). Per AGENTS.md include order: system → external → internal.`

`L35: 🔵 nit: DEFAULT_TIMEOUT_TWO_MINUTES is uint64_t but name says "two minutes" — magic constant. Consider making configurable via QueryOptions or at least add unit suffix: DEFAULT_TIMEOUT_SECONDS.`

`L48: 🟡 risk: response.get("X-Request-Id") throws Poco::NotFoundException if header missing. Other handlers (logging_request_handler.cpp:L16) do same pattern, so presumably guaranteed by RequestIdHandler middleware — but no defensive check. If middleware chain changes, this crashes. Consider request.has() guard or document the invariant.`

`L52: 🟡 risk: Poco::StreamCopier::copyToString reads entire body into memory with no size limit. Malicious client can send multi-GB body → OOM. Add Content-Length check or cap read size. Even a 10MB limit would prevent trivial DoS.`

`L54: 🟡 risk: Full query string logged at INFO level. If queries contain sensitive data (patient IDs, etc.), this leaks to logs. Consider SPDLOG_DEBUG or truncating.`

`L64-65: 🟡 risk: Accept header parsing via string::find is naive. "application/vnd.apache.arrow.stream" could match inside quality params or comments. RFC 7231 Accept headers can be "application/vnd.apache.arrow.stream;q=0.5, application/x-ndjson;q=1.0" — find() would match arrow even though ndjson has higher quality. For now probably fine since clients are controlled, but worth a TODO or comment acknowledging the limitation.`

`L69,81: 🟡 risk: response.send() commits HTTP 200 status + headers to wire. If executeAndWrite() then throws (L143 in query_plan.cpp throws std::runtime_error on non-IO errors), ErrorRequestHandler catches it but CANNOT change status code — headers already sent. Client gets HTTP 200 with truncated/corrupt body. This is inherent to streaming and may be acceptable, but:`
- `For NDJSON: consider writing an error object as final line so clients can detect it.`
- `For Arrow IPC: stream will be truncated — Arrow readers should detect incomplete stream.`
- `At minimum: document this behavior so API consumers know to check for complete responses.`

`L72-74: 🟡 risk: ArrowIpcSink::make failure throws std::runtime_error AFTER response.send() already committed 200. Same problem as above — client gets 200 + empty body. Move result check before response.send(), or restructure to validate sink creation first.`

`L80: ❓ q: Default fallback is NDJSON for any Accept header (including "application/json", "text/html", "*/*"). Is this intentional? Might want to return 406 Not Acceptable for explicitly unsupported types, or at least match "*/*" and "application/x-ndjson" specifically.`

`L87-91: Minor — catch blocks only handle ParseException and IllegalQueryException. Other exceptions (std::runtime_error from L73, Arrow errors) propagate to ErrorRequestHandler which handles them as 500. This is correct given the middleware design. No issue here.`

### query_handler.h

`L3-4: 🔵 nit: HTTPServerRequest.h and HTTPServerResponse.h included but only used as reference params in post() declaration. Forward declarations would suffice, reducing header coupling. Though this matches existing codebase style (rest_resource.h does same), so low priority.`

---

## Architecture Notes

**Error handling chain is sound**: QueryHandler throws BadRequest for user errors → ErrorRequestHandler catches and returns 400. Unexpected exceptions → 500. The `cxxabi.h` usage in error_request_handler.cpp for catch(...) type introspection is clever.

**Streaming-after-commit is the main design tension**: Once `response.send()` is called, HTTP status is locked. Mid-query failures produce corrupt 200 responses. This is a known tradeoff in streaming APIs. Recommend documenting this in API docs and considering error sentinel values in NDJSON output.

---

## Severity Summary

| Severity | Count |
|----------|-------|
| 🔴 bug | 0 |
| 🟡 risk | 6 |
| 🔵 nit | 4 |
| ❓ question | 1 |

**Most impactful**: L52 (no body size limit) and L69/81 (error-after-commit).
