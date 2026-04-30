# Review: `saneql.examples`

## 🔴 Critical: Wrong file — TPC-H queries, not SILO SaneQL

**Entire file (L1-250) contains TPC-H benchmark queries that cannot parse or execute with the SILO SaneQL parser.** These are standard SQL-style analytical queries (TPC-H Q1–Q22) written in a SaneQL dialect from the original academic SaneQL paper, NOT the SILO implementation.

None of these 250 lines will parse with the SILO parser. The file is misleading as documentation.

### Functions used in file but NOT in SILO parser:

| Function | Lines used | Status |
|----------|-----------|--------|
| `join` | L22-24, L27-30, L39-40, L49, L56-60, L74-78, L88-94, L103-107, L117-119, L126-127, L138, L145, L155, L167, L176-177, L187-188, L193-196, L203, L219, L222-224, L231-235, L248 | ❌ Not registered |
| `aggregate` | L24, L69, L130, L156, L168, L184, L208, L244 | ❌ Not registered |
| `map` | L80, L95, L108, L246 | ❌ Not registered |
| `let` | L19, L114, L125, L135, L152, L160-161, L183, L212-213, L217, L241 | ❌ Not a keyword/construct |
| `case` | L96, L139, L156 | ❌ Not in parser |
| `as` | L77-78, L92-93, L231, L234-235 | ❌ Not registered |
| `extract` | L80, L95 | ❌ Not registered |
| `substr` | L243, L246 | ❌ Not registered |
| `sum` | L6-9, L61, L81, L96, L109, L120, L129-130, L139, L156, L164, L196, L208, L249 | ❌ Only `count` supported |
| `avg` | L10-12, L184, L244 | ❌ Only `count` supported |
| `min` | L24 | ❌ Only `count` supported |
| `max` | L168 | ❌ Only `count` supported |
| `count(distinct:=true)` | L178 | ❌ `count` takes no args |
| `::interval` | L4, L48, L56, L68, L116, L137, L154, L163, L215 | ❌ Only `::date` supported |
| `orderby` with `limit:=` | L32, L42, L121, L197, L237 | ❌ `orderBy` has no `limit` param |
| `.desc()` on fields | L32, L42, L62, L110, L121, L131, L148, L197, L237 | ⚠️ Works differently — SILO uses `desc(field)` inside `orderBy({...})` |

### SILO-specific functions NOT demonstrated anywhere:

**Pipeline functions (0 of 12 covered):**
- `filter` — used but only with TPC-H predicates
- `groupBy` — used but with unsupported aggregates (sum/avg)
- `project` — used but in TPC-H context
- `mutations` ❌
- `aminoAcidMutations` ❌
- `insertions` ❌
- `aminoAcidInsertions` ❌
- `randomize` ❌
- `limit` — not as separate function (only as `limit:=` param on orderby)
- `offset` ❌
- `orderBy` — used but with wrong syntax
- `mostRecentCommonAncestor` ❌
- `phyloSubtree` ❌

**Filter functions (0 of 15 covered in SILO context):**
- `between` — used but only for TPC-H
- `in` — used but only for TPC-H
- `isNull` ❌
- `isNotNull` ❌
- `lineage` ❌
- `phyloDescendantOf` ❌
- `like` — used but only for TPC-H
- `nucleotideEquals` ❌
- `aminoAcidEquals` ❌
- `hasMutation` ❌
- `hasAAMutation` ❌
- `insertionContains` ❌
- `aminoAcidInsertionContains` ❌
- `exact` ❌
- `maybe` ❌
- `nOf` ❌

### Syntax features NOT demonstrated:

- `'2021-01-01'::date` — used but only `::interval` (unsupported) context
- Set literal `{...}` — used but only for TPC-H
- Record literal `{name:=value}` — used but only for TPC-H aggregates
- `true`/`false` boolean literals ❌
- `null` literal ❌
- `!` (NOT) operator ❌
- `<>` (not-equals) operator — used in TPC-H only

## Summary

`L1-250: 🔴 bug: entire file is TPC-H benchmark queries from academic SaneQL paper. None parse with SILO's SaneQL implementation. Zero SILO-specific functions demonstrated. File misleads users about supported syntax.`

**Recommendation:** Replace with actual SILO genomic query examples. Good examples already exist in `endToEndTests/test/queries/*.json` — extract the `"query"` fields from those. Example correct queries from the e2e tests:

```
-- Filter + aggregate
default.filter(country = 'Switzerland').groupBy({count:=count()})

-- Lineage with sublineages
default.filter(pango_lineage.lineage('B.1.1.7', includeSublineages:=true)).groupBy({count:=count()})

-- Date between
default.filter(date.between('2021-01-01'::date, '2021-12-31'::date)).groupBy({count:=count()})

-- Mutations
default.filter(false).mutations(minProportion:=0.5)

-- Insertions
default.insertions().orderBy({insertion})

-- Details with limit/offset
default.filter(country = 'Switzerland').orderBy({primary_key}).offset(9).limit(2).project({age, country})

-- nOf
default.filter(nOf(2, {nucleotideEquals(position:=241, symbol:='T'), nucleotideEquals(position:=29734, symbol:='T')})).groupBy({count:=count()})

-- Phylo
default.filter((primary_key = 'key_11') || (primary_key = 'key_22')).mostRecentCommonAncestor('usherTree').orderBy({mrcaNode})
```
