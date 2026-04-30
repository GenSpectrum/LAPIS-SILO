# Review: function_registry.h / function_registry.cpp

**Files:** `src/silo/query_engine/saneql/function_registry.{h,cpp}`
**PR:** #1238 — SaneQL function/filter registries + argument binding

---

## Findings

### function_registry.cpp

`L49-55: 🟡 risk: getOptionalUint32 checks value >= 0 but not <= UINT32_MAX. int64_t up to 2^63 silently truncated to uint32_t. Add upper-bound check: value <= std::numeric_limits<uint32_t>::max().`

`L53: 🔵 nit: error message "If the action contains an {}" is copy-pasted from old JSON API style. Doesn't match SaneQL context. Use "{}(): '{}' must be a non-negative integer" with function_name_ and name.`

`L60-122: bindArguments — overall logic correct. Positional-then-named matching, skip non-positional params, detect duplicates, check required. Clean.`

`L95-108: 🟡 risk: duplicate named args from parser not caught. Parser's parseArgList() doesn't reject f(x := 1, x := 2). bindArguments checks bound.contains() but only against positional bindings — second named arg with same name silently overwrites first via bound[named_arg.name]. Should CHECK_SILO_QUERY(!bound.contains(named_arg.name)) before insert, with error message covering both positional-duplicate AND named-duplicate cases. Current message says "already bound positionally" which is wrong when both are named.`

`L102-106: 🔵 nit: error message says "already bound positionally" but could also be duplicate named arg. Change to "already bound" or detect which case.`

### function_registry.h

`L30-51: BoundArguments — good API. at() for required, get() for optional, has() for existence. Clear contract.`

`L46: 🔵 nit: getOptionalUint32 exists but no getRequiredUint32. Callers in ast_to_query.cpp do raw static_cast<uint32_t>(extractIntLiteral(args.at(...))) without range checks (L74, L184, L284, L287, L376, L401, L411, L433, L737). Consider adding getRequiredUint32() to centralize the range validation. Not blocking but would eliminate ~10 unsafe casts.`

`L87,112: instance() — static local in .cpp, C++11 guarantees thread-safe init. Construction calls registerFunction() which only touches own entries_ map. Safe. Good.`

`L74-91, L99-116: 🔵 nit: FunctionRegistry and FilterFunctionRegistry are structurally identical except handler type. Could be a template<typename Handler> class GenericRegistry. Not blocking — two classes is fine for now, but worth noting if more registry types appear.`

`L16: using Tables = std::map<...> — declared in header but only used by FunctionHandler typedef. Fine, but couples header to storage/table.h. Minor.`

### Cross-file (ast_to_query.cpp callers)

`ast_to_query.cpp:L74,L284,L287,L376,L401,L411: 🟡 risk: same truncation bug as getOptionalUint32 — raw static_cast<uint32_t>(extractIntLiteral(...)) with no upper-bound check. Negative values also unchecked in some (L74, L284, L287). Systematic issue. getOptionalUint32 at least checks >= 0 but misses upper bound; these callers check neither.`

`ast_to_query.cpp:L433: static_cast<int>(extractIntLiteral(...)) for nOf count — same truncation risk for int (though practically unlikely with small counts).`

---

## Summary

| Severity | Count | Description |
|----------|-------|-------------|
| 🟡 risk | 3 | uint32 truncation (getOptionalUint32 + callers), duplicate named arg silent overwrite |
| 🔵 nit | 4 | error message style, missing getRequiredUint32, registry dedup, error text accuracy |

**Overall:** Solid design. bindArguments logic correct and well-structured. Main concern is int64→uint32 truncation without upper-bound check — values > 4B silently wrap. Duplicate named arg edge case unlikely from normal usage but parser doesn't prevent it. No blockers, but the truncation risk should be fixed.
