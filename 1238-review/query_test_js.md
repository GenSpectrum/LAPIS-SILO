# Review: `endToEndTests/test/query.test.js` (PR #1238)

Overall: solid rewrite. Data-driven test structure good. Few issues below.

---

## Findings

`L43: 🟡 risk: Number(bigint) silently loses precision for values > 2^53. Genomic counts unlikely to hit this, but no guard. Add assertion or comment documenting safe range assumption.`

`L24: 🔵 nit: readFilesRecursively filters .json by extension but invalidQueries at L85 uses readdirSync without filter — non-.json files in that dir would cause parse crash. Inconsistent.`

`L85: 🟡 risk: readdirSync(invalidQueriesPath) returns ALL files (no .json filter, no recursive). Stray file (e.g. .DS_Store) → JSON.parse crash with unhelpful error. Filter to .json like readFilesRecursively does.`

`L125-134: 🟡 risk: invalid queries only tested with NDJSON format. Valid queries test both NDJSON and Arrow IPC. If server returns different error shape for Arrow Accept header, that path is untested. Should loop over formats like valid queries do, or add explicit Arrow IPC error test.`

`L152-158: 🟡 risk: tests invalid SaneQL returns 400 + correct Content-Type, but does NOT verify response body. Malformed error message or empty body would pass. Add .expect(body => ...) asserting error structure matches {error, message} shape.`

`L161-167: 🟡 risk: same as L152 — empty query test checks status+content-type but not body. Should verify error payload.`

`L14-30: 🔵 nit: readFilesRecursively only used once (L83). queries/ has one subdir (symbolEquals/). Could simplify with glob or flat list. Not blocking, but adds complexity for minimal gain.`

`L56-59: 🔵 nit: split(/\n/) on NDJSON — if server sends \r\n line endings, empty strings between lines survive the filter but parsed JSON would still work. Fine in practice, just noting.`

`L65: ❓ q: Content-Type 'text/plain' for SaneQL — is this the agreed-upon MIME type? No custom media type like 'application/x-saneql'? If server validates Content-Type strictly, this is correct. If server accepts anything, test doesn't verify rejection of wrong Content-Type.`

`L97-111: 👍 good: parameterized test over formats with shared test fixtures. Clean pattern, easy to extend.`

`L115-120: 👍 good: uniqueness check on test case names prevents silent shadowing.`

---

## Summary

| Severity | Count |
|----------|-------|
| 🔴 bug | 0 |
| 🟡 risk | 4 |
| 🔵 nit | 3 |
| ❓ question | 1 |

**Key action items:**
1. Add .json filter to `invalidQueries` file reading (L85) — crash waiting to happen
2. Invalid query tests should verify error body content, not just status code (L152, L161)
3. Consider testing invalid queries with Arrow IPC Accept header too (L125)
