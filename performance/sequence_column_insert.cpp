#include <chrono>
#include <random>
#include <string>
#include <vector>

#include <spdlog/spdlog.h>

#include "sequence_generator.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/storage/column/sequence_column.h"

using silo::Nucleotide;
using silo::storage::column::SequenceColumn;
using silo::storage::column::SequenceColumnMetadata;

namespace {

constexpr size_t SEQUENCE_COUNT = 100'000;

std::vector<std::string> generateSequences(const std::string& reference) {
   SequenceTreeGenerator tree_gen(reference, 42, 0.001, 0.1, 12, 3);
   auto evolved = tree_gen.generateEvolvedSequences();
   SPDLOG_INFO("generated {} distinct evolved sequences", evolved.size());

   std::mt19937 rng(7);
   std::uniform_int_distribution<size_t> pick(0, evolved.size() - 1);
   // realistic-ish N runs at both ends plus a few internal N stretches
   std::uniform_int_distribution<size_t> head_n(0, 300);
   std::uniform_int_distribution<size_t> tail_n(0, 300);
   std::uniform_int_distribution<size_t> internal_runs(0, 5);
   std::uniform_int_distribution<size_t> run_len(1, 100);
   std::uniform_int_distribution<size_t> pos_dist(0, reference.size() - 200);

   std::vector<std::string> sequences;
   sequences.reserve(SEQUENCE_COUNT);
   for (size_t i = 0; i < SEQUENCE_COUNT; ++i) {
      std::string sequence = evolved.at(pick(rng));
      const size_t head = std::min(head_n(rng), sequence.size());
      const size_t tail = std::min(tail_n(rng), sequence.size());
      for (size_t j = 0; j < head; ++j) {
         sequence[j] = 'N';
      }
      for (size_t j = 0; j < tail; ++j) {
         sequence[sequence.size() - 1 - j] = 'N';
      }
      const size_t runs = internal_runs(rng);
      for (size_t r = 0; r < runs; ++r) {
         const size_t start = pos_dist(rng);
         const size_t length = run_len(rng);
         const size_t end = std::min(start + length, sequence.size());
         for (size_t j = start; j < end; ++j) {
            sequence[j] = 'N';
         }
      }
      sequences.push_back(std::move(sequence));
   }
   return sequences;
}

double toMs(std::chrono::steady_clock::time_point start, std::chrono::steady_clock::time_point end) {
   return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.0;
}

}  // namespace

int main() {
   changeCwdToTestFolder();
   const std::string reference = readReferenceFromFile();
   SPDLOG_INFO("reference length {}", reference.size());

   const auto sequences = generateSequences(reference);

   std::vector<Nucleotide::Symbol> reference_symbols;
   for (char character : reference) {
      reference_symbols.push_back(Nucleotide::charToSymbol(character).value());
   }
   SequenceColumnMetadata<Nucleotide> metadata{"main", std::move(reference_symbols)};
   SequenceColumn<Nucleotide> column{&metadata};

   const std::vector<std::string> no_insertions;

   constexpr size_t CHUNK_SIZE = silo::storage::column::COLUMN_CHUNK_SIZE;
   double builder_ms = 0;
   double append_ms = 0;

   const auto total_start = std::chrono::steady_clock::now();
   for (size_t chunk_start = 0; chunk_start < sequences.size(); chunk_start += CHUNK_SIZE) {
      SequenceColumn<Nucleotide>::Builder builder{&metadata, reference};
      const size_t chunk_end = std::min(chunk_start + CHUNK_SIZE, sequences.size());

      const auto build_start = std::chrono::steady_clock::now();
      for (size_t i = chunk_start; i < chunk_end; ++i) {
         builder.insert(sequences[i], 0, no_insertions);
      }
      auto buffer = builder.finalize();
      const auto build_end = std::chrono::steady_clock::now();
      builder_ms += toMs(build_start, build_end);

      const auto append_start = std::chrono::steady_clock::now();
      SILO_ASSERT(column.appendChunk(buffer).has_value());
      const auto append_end = std::chrono::steady_clock::now();
      append_ms += toMs(append_start, append_end);
   }

   const auto finalize_start = std::chrono::steady_clock::now();
   column.finalize();
   const auto finalize_end = std::chrono::steady_clock::now();
   const auto total_end = std::chrono::steady_clock::now();

   size_t adapted_positions = 0;
   for (size_t i = 0; i < reference.size(); ++i) {
      if (column.local_reference_sequence_string[i] != reference[i]) {
         adapted_positions++;
      }
   }

   SPDLOG_INFO("adapted positions: {} of {}", adapted_positions, reference.size());
   SPDLOG_INFO("sequences:        {}", sequences.size());
   SPDLOG_INFO("builder.insert:   {:.1f} ms", builder_ms);
   SPDLOG_INFO("column.appendChunk: {:.1f} ms", append_ms);
   SPDLOG_INFO("column.finalize:  {:.1f} ms", toMs(finalize_start, finalize_end));
   SPDLOG_INFO("total:            {:.1f} ms", toMs(total_start, total_end));
   return 0;
}
