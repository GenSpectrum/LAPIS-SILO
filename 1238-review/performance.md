# PR #1238 — Performance Benchmark Files Review

## many_short_read_filters.cpp

`L286: 🔵 nit:` Unused variable `input_data_stream`. `NdjsonLineReader` constructed but never used — `appendData` called directly on `input_buffer` at L287. Remove dead code.

`L114-136: 🟡 risk:` `current_generation` and `next_generation` are `vector<string_view>` pointing into `all_generated_sequences` (a `vector<string>`). When `push_back` at L125 triggers reallocation, moved `string` objects preserve heap pointers for non-SSO strings — so this works for 200-char reads. But it's **fragile**: if `DEFAULT_READ_LENGTH` is ever reduced below SSO threshold (~22 chars), all `string_view`s silently dangle → UB. Pre-existing, not introduced by this PR, but worth a `reserve()` or switching to `vector<size_t>` indices.

`L369-376: ❓ q:` `while(true)` infinite loop — intentional for profiling (attach perf/vtune and Ctrl-C)? If so, a comment explaining purpose would help. Otherwise looks like missing termination condition.

`L311-323: ✅` `{{count:=count()}}` in `fmt::format` correctly escapes to `{count:=count()}` — matches SaneQL `groupBy` syntax. Verified against parser tests.

`L311-323: ✅` SaneQL query syntax correct: `nucleotideEquals(position:=N, symbol:='X')` matches registered function signature `{named("position"), named("symbol"), named("sequenceName", false)}`.

`L313-320: 🔵 nit:` Duplicate `samplingDate.between(...)` clause at L314 and L320 — same filter applied twice in AND. Functionally harmless (optimizer likely deduplicates) but reads like copy-paste artifact. Pre-existing from JSON version.

`L37: 🔵 nit:` `std::filesystem::current_path().string()` → unnecessary round-trip through string. `std::filesystem::path` can be assigned directly. Pre-existing.

## mutation_benchmark.cpp

`L105: ✅` `"default.mutations(minProportion:=0.05, sequenceNames:={main})"` — correct SaneQL. `{main}` is a set literal, matches parser. `minProportion` is named arg matching `ast_to_query.cpp` L888.

`L118: ✅` `"default.filter(!(key = '3')).mutations(minProportion:=0.05, sequenceNames:={main})"` — negation syntax `!(expr)` correct. Clean migration from `Negation(StringEquals(...))`.

`L37: 🟡 risk:` `current_id` is a file-scope mutable global in anonymous namespace. Not thread-safe, and state persists across calls. Fine for single-threaded benchmark, but fragile if someone adds parallelism. Pre-existing.

`L143: 🔵 nit:` Line exceeds 100-char column limit (project style). Pre-existing.

## many_string_equals.cpp

`L131-150: ❓ q:` This file constructs operator trees directly (`ScanNode` → `FilterNode` → `AggregateNode`) via `Planner::planQuery()` instead of using `Planner::planSaneqlQuery()`. Looking at the diff, this is **intentional** — the benchmark specifically tests `StringEquals` vs `StringInSet` filter expression performance at the operator level, bypassing the parser. The `Expression` objects are constructed programmatically. This is valid and correct — `planQuery()` still exists and accepts operator trees.

`L146: ✅` `Planner::planQuery(std::move(root), ...)` — correct API usage per `planner.h` L19-24.

`L236: 🔵 nit:` Line exceeds 100-char column limit. Same for L293.

## Summary

Migration looks correct. SaneQL syntax verified against parser tests and `ast_to_query.cpp` function registrations. Key findings:

| Severity | Count | Notes |
|----------|-------|-------|
| 🔴 Critical | 0 | — |
| 🟡 Risk | 2 | string_view fragility (pre-existing), global mutable state (pre-existing) |
| 🔵 Nit | 4 | dead code, line length, duplicate filter, unnecessary conversion |
| ❓ Question | 1 | `while(true)` loop intent |
| ✅ Good | 4 | SaneQL syntax correct, operator tree approach intentional |

No blockers for merge from these files.
