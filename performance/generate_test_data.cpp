#include <cstdlib>
#include <exception>
#include <fstream>
#include <string>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "sequence_generator.h"

// Standalone generator for the datasets consumed by the performance benchmarks. Generating this
// data (tree-evolution models, millions of rows) is the expensive part of each benchmark, so it is
// produced once here with fixed parameters, written to disk under localTestData/performance/, and
// read back by the benchmarks instead of being regenerated on every run. Run with
// `make generateTestData`.

namespace {

void writeDataset(std::string_view path, const auto& generate) {
   auto out = openTestDataOutput(path);
   generate(out);
   out.flush();
   if (!out) {
      throw std::runtime_error(fmt::format("Failed while writing {}", path));
   }
   SPDLOG_INFO("Wrote {}", path);
}

void run() {
   changeCwdToTestFolder();
   const std::string reference = readReferenceFromFile();
   SPDLOG_INFO("Read reference sequence of length {}", reference.size());

   // Short reads: 100k for nof_sequence_filter, 5M shared by many_short_read_filters and the large
   // nof_sequence_filter case.
   writeDataset(SHORT_READ_SMALL_NDJSON_PATH, [&](std::ostream& out) {
      writeShortReadNdjson(out, reference, DEFAULT_FULL_SEQ_COUNT);
   });
   writeDataset(SHORT_READ_LARGE_NDJSON_PATH, [&](std::ostream& out) {
      writeShortReadNdjson(out, reference, DEFAULT_READ_COUNT);
   });

   // Amplicon-coverage short reads for many_short_read_filters, emitted both amplicon-sorted and
   // randomly shuffled. The two files hold the same reads, so ingesting either builds the same
   // database; only their on-disk order (and hence the coverage layout ingestion sees) differs.
   writeDataset(SHORT_READ_AMPLICON_SORTED_NDJSON_PATH, [&](std::ostream& out) {
      writeAmpliconShortReadNdjson(out, reference, /*shuffle=*/false);
   });
   writeDataset(SHORT_READ_AMPLICON_SHUFFLED_NDJSON_PATH, [&](std::ostream& out) {
      writeAmpliconShortReadNdjson(out, reference, /*shuffle=*/true);
   });

   // Full-length sequences for nof_sequence_filter.
   writeDataset(FULL_SEQUENCE_NDJSON_PATH, [&](std::ostream& out) {
      writeFullSequenceNdjson(out, reference);
   });

   // Full-length sequences with N runs for sequence_column_insert.
   writeDataset(SEQUENCE_COLUMN_NDJSON_PATH, [&](std::ostream& out) {
      writeNRunSequenceNdjson(out, reference);
   });

   // Synthetic short reads for mutation_benchmark (uses its own repeated ACGT reference).
   writeDataset(MUTATION_READS_NDJSON_PATH, [](std::ostream& out) {
      writeMutationBenchmarkNdjson(out);
   });

   // Accession/country records for many_string_equals.
   writeDataset(STRING_EQUALS_NDJSON_PATH, [](std::ostream& out) {
      writeStringEqualsNdjson(out);
   });

   // Random sequences for co_occurrence_benchmark (uses its own short random reference).
   const std::string co_occurrence_reference = makeCoOccurrenceReference();
   writeDataset(CO_OCCURRENCE_NDJSON_PATH, [&](std::ostream& out) {
      writeCoOccurrenceNdjson(out, co_occurrence_reference);
   });

   SPDLOG_INFO("All benchmark datasets written.");
}

}  // namespace

int main() {
   try {
      run();
   } catch (const std::exception& e) {
      SPDLOG_ERROR(e.what());
      return EXIT_FAILURE;
   }
}
