# Bitmap-aggregation benchmark baseline

Reference numbers for the bitmap-aggregation work **before it is split into separate mergeable
PRs**. Re-run these on each PR branch and fill in the [per-PR table](#per-pr-comparison-to-fill-in)
so we can confirm each split is perf-neutral where expected, and that the coverage-scan win is
attributed to the right PR.

## Environment (where these numbers were taken)

| | |
|---|---|
| CPU | AMD Ryzen 7 PRO 8700GE (Zen 4, 16 threads), **AVX-512 present** |
| RAM | 124 GiB |
| Compiler | clang 18.1.3, CMake **Release** build |
| Branch | `cowb-container-rep` (all changes combined) |

> Numbers are machine-specific — only compare runs taken on the **same machine, same Release build**.
> The AVX-512 coverage scan has a scalar fallback, so on a non-AVX-512 host the real-data numbers will
> differ (that path should be measured separately if we care about it).

## Benchmarks

| Binary | What it exercises | Data |
|---|---|---|
| `co_occurrence_many_positions_benchmark` | Co-occurrence groupBy over **128 sequence positions** (10 variable at rate 0.15); stresses the per-chunk grouping + coverage scan + output materialization. Self-contained. | 2,000,000 synthetic seqs, ref length 128 |
| `co_occurrence_benchmark` | Smaller co-occurrence groupBy; quick smoke number. Self-contained. | synthetic |
| `real_data_mutations_benchmark` | Co-occurrence groupBy over the **141 real SARS-CoV-2 positions** in `performance/mutations.csv`, on real short-read data. Clustered ingestion. | full `~/sorted.ndjson.zst` (~296M reads) |

Only `co_occurrence_many_positions_benchmark` and `real_data_mutations_benchmark` are load-bearing
for this work; `co_occurrence_benchmark` is a cheap sanity number.

## How to reproduce

```shell
# Build (Release)
make build/Release/build.ninja
cmake --build build/Release --target \
  co_occurrence_many_positions_benchmark co_occurrence_benchmark real_data_mutations_benchmark

# Synthetic (self-contained, deterministic)
./build/Release/performance/co_occurrence_many_positions_benchmark
./build/Release/performance/co_occurrence_benchmark

# Real data: FIRST run ingests ~296M reads from the .zst (~9 min, ~24 GB RAM) and saves a ~21 GB DB.
# Subsequent runs LOAD that saved DB (~16 s) and just re-run the query — use these for timing.
REAL_DB_DIR=/home/alexander/realdb_clustered \
REAL_NDJSON=/home/alexander/sorted.ndjson.zst \
CLUSTER=1 \
./build/Release/performance/real_data_mutations_benchmark      # ingest + save + query

REAL_DB_DIR=/home/alexander/realdb_clustered \
./build/Release/performance/real_data_mutations_benchmark      # load saved DB + query (repeat 2-3x)
```

### Measurement rules

- **Discard the first real-data run of a freshly-ingested DB.** Immediately after the 554 s
  ingest + 21 GB save the machine is under I/O/memory pressure and the DB pages are cold; that run
  reads high (observed 1167 ms vs a settled ~923 ms). Time only the **loaded-DB** runs.
- Take the **min** as the comparable figure (least noise); the benchmark also prints avg over 5
  iterations.
- Keep `CLUSTER=1` for the real-data DB (matches how the baseline DB was built). Re-using the same
  saved `REAL_DB_DIR` across PRs keeps the on-disk layout identical, so only code changes move the
  number. The serialized DB format is unchanged by this work (the SoA / envelope fields are derived
  on load, not serialized), so the same saved DB loads across all these branches.

## Results — all changes combined (`cowb-container-rep`)

| Benchmark | avg | min |
|---|---|---|
| `co_occurrence_many_positions_benchmark` | 509 ms | **495 ms** |
| `co_occurrence_benchmark` | 15.6 ms | **15.0 ms** |
| `real_data_mutations_benchmark` (loaded DB, warm) | 923 ms | **891 ms** |

Real-data query returns 15,548 result rows.

## Development-time progression (context)

Approximate figures recorded while the work was developed, same real-data workload, for attribution
(the real-data win is essentially all from the coverage-scan change):

| Stage | Real-data query (avg) | Synthetic many-positions (avg) |
|---|---|---|
| Before this work | ~1553 ms | ~552 ms |
| + SoA + AVX-512 coverage scan | ~946 ms | ~508 ms |
| + `unordered_map` for counts | ~926 ms | — |
| + FieldRef / general-expression grouping (this branch) | ~923 ms | ~509 ms |

## Candidate PR split & expected perf impact

Measure each PR **cumulatively** (stacked in this order) or against `main`, whichever the split uses.
Expected impact on the co-occurrence path:

| # | PR (logical change) | Expected impact |
|---|---|---|
| 1 | Bitmap-aggregation node restructure (per-chunk group building, container-level algebra in `roaring_util`, `DimensionGroupsInChunk` variant + whole-chunk fast paths) | neutral to slightly better |
| 2 | `HorizontalCoverageIndex`: cached batch envelopes + SoA layout + **AVX-512 coverage scan** | **the real-data win** (~1553 → ~946 ms) |
| 3 | `counts`: `std::map` → `std::unordered_map` | small win (~946 → ~926 ms) |
| 4 | FieldRef-in-map grouping (`FieldColumnGrouper`, indexed-column reuse) | **neutral** — new path, not on the co-occurrence benchmark |
| 5 | General scalar-expression grouping (`isoWeek`; typed output via `arrow::compute::Take`; shared `scalarToArrowExpression`) | **neutral** — new path; the shared `Take` materialization was confirmed neutral at 141 dims × 15.5k rows |
| 6 | Generic group-by fields (a key the `map` does not produce resolves as a bare read of the same-named scan column, so plain metadata fields group through the bitmap engine too) | **neutral** — it only widens which queries take the path |

The two co-occurrence benchmarks exercise only the sequence-position path, so PRs 4 to 6 should
leave both numbers flat; watch PRs 2 and 3 for the movement.

## Per-PR comparison (to fill in)

| PR | synthetic min (ms) | real-data min (ms) | Δ vs previous | Notes |
|---|---|---|---|---|
| baseline (`main`) | | | — | |
| 1. node restructure | | | | |
| 2. coverage scan (SoA + AVX-512) | | | | |
| 3. unordered_map counts | | | | |
| 4. FieldRef grouping | | | | |
| 5. scalar-expression grouping | | | | |
| 6. generic group-by fields | | | | |
| **combined (`cowb-container-rep`)** | **495** | **891** | — | reference, this doc |
