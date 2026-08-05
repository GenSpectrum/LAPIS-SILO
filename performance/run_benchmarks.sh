#!/usr/bin/env bash
# Run every performance benchmark binary in sequence.
#
# The run continues past a benchmark that exits non-zero and reports which ones failed at the end.
# It exits non-zero if any benchmark failed, so CI/automation can detect failures. The benchmarks
# must already be built and `make generateTestData` must have produced their input data; the
# `benchmarks` Makefile target takes care of both before invoking this script.
#
# Usage: performance/run_benchmarks.sh [BENCHMARK_BINARY_DIR]
#   BENCHMARK_BINARY_DIR defaults to build/Release/performance.
set -u

# Run from the repository root so the benchmarks find testBaseData/ and localTestData/ via relative
# paths regardless of where this script was invoked from.
script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${script_dir}/.."

bin_dir="${1:-build/Release/performance}"

benchmarks=(
  mutation_benchmark
  many_string_equals
  many_short_read_filters
  clustered_ingestion_benchmark
  nof_sequence_filter
  sequence_column_insert
  co_occurrence_benchmark
)

failed=()
for bench in "${benchmarks[@]}"; do
  echo "=== Running ${bench} ==="
  if ! "${bin_dir}/${bench}"; then
    failed+=("${bench}")
  fi
done

if [ "${#failed[@]}" -ne 0 ]; then
  echo "Benchmarks that exited non-zero: ${failed[*]}"
else
  echo "All benchmarks completed."
fi
