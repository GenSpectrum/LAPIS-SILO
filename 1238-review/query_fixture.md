# PR #1238 — Review: `query_fixture.test.h` / `query_fixture.test.cpp`

## query_fixture.test.cpp

`L26: 🔴 bug: std::cout << line_object.dump() left in. Dumps every query result line to stdout on every test run. Remove or gate behind SPDLOG_DEBUG.`

## query_fixture.test.h

`L82: 🟡 risk: negateFilter() declared but definition removed from .cpp. Dead declaration → linker error if anyone calls it. Remove declaration.`

`L69: 🟡 risk: QueryTestScenario.query typed as nlohmann::json but every caller assigns a std::string (plain SaneQL). L120 immediately calls .get<std::string>(). Change type to std::string — removes implicit json wrapping, avoids runtime type mismatch if someone passes a json object by mistake, and drops the nlohmann dependency from the struct.`

`L4: 🔵 nit: #include <iostream> unused in header (no std::cout/cin/cerr here). Remove.`

`L8-9: 🔵 nit: #include <simdjson.h> and #include <spdlog/spdlog.h> unused in this header. Nothing from simdjson or spdlog referenced. Remove — reduces transitive include bloat for all test files.`

`L128: 🔵 nit: catch(const std::exception&) is very broad for error-path tests. If planSaneqlQuery or executeQueryToJsonArray throw an unexpected exception type (e.g. std::bad_alloc), test still passes as long as .what() matches. Consider catching the specific silo exception type(s) and letting unexpected exceptions propagate as test failures.`

## has_mutation.test.cpp (bonus — spotted while tracing callers)

`L79-80: 🔴 bug: Variable HAS_NUCLEOTIDE_MUTATION_OUT_OF_RANGE_EDGE_LOW has .name = "HAS_NUCLEOTIDE_MUTATION_OUT_OF_RANGE_EDGE_HIGH" — names swapped with L87-88 (and vice versa). Tests still pass because name is only used for display, but confusing when debugging failures.`

## Summary

Two bugs (debug stdout, swapped test names), one dead declaration, one type mismatch (json→string), three unnecessary includes. No architectural concerns — fixture design is clean and macro approach is reasonable for parameterized query tests.
