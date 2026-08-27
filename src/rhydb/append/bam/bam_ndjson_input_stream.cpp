#include "rhydb/append/bam/bam_ndjson_input_stream.h"

#include <optional>
#include <string>
#include <vector>

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "rhydb/append/bam/bam_exception.h"
#include "rhydb/append/bam/bam_reader.h"
#include "rhydb/append/bam/cigar.h"
#include "rhydb/append/ndjson_line_reader.h"
#include "rhydb/append/table_inserter.h"
#include "rhydb/schema/database_schema.h"
#include "rhydb/storage/table.h"

namespace rhydb::append::bam {

namespace {

constexpr std::string_view CIGAR_OP_CHARS = "MIDNSHP=X";

std::string cigarToString(const std::vector<CigarElement>& cigar) {
   if (cigar.empty()) {
      return "*";
   }
   std::string result;
   for (const auto& element : cigar) {
      result += std::to_string(element.length);
      result += CIGAR_OP_CHARS[static_cast<size_t>(element.op)];
   }
   return result;
}

bool isSupportedMetadataColumn(std::string_view name) {
   return name == "read_index" || name == "qname" || name == "flag" || name == "rname" ||
          name == "pos" || name == "mapq" || name == "cigar" || name == "mate_rname" ||
          name == "mate_pos" || name == "tlen" || name == "qual";
}

/// The reference name for a reference id, or JSON null when the id is -1 (no
/// reference) or out of range.
nlohmann::json referenceName(int32_t reference_id, const std::vector<BamReference>& references) {
   if (reference_id < 0 || static_cast<size_t>(reference_id) >= references.size()) {
      return nullptr;
   }
   return references[static_cast<size_t>(reference_id)].name;
}

/// BAM stores positions 0-based; the metadata `pos`/`mate_pos` columns follow the
/// 1-based SAM convention, with null for an unavailable position.
nlohmann::json toSamPosition(int32_t position) {
   if (position < 0) {
      return nullptr;
   }
   return position + 1;
}

nlohmann::json metadataValue(
   std::string_view name,
   const BamRecord& record,
   const std::vector<BamReference>& references,
   uint64_t read_index
) {
   if (name == "read_index") {
      return read_index;
   }
   if (name == "qname") {
      return record.read_name;
   }
   if (name == "flag") {
      return record.flag;
   }
   if (name == "rname") {
      return referenceName(record.reference_id, references);
   }
   if (name == "pos") {
      return toSamPosition(record.position);
   }
   if (name == "mapq") {
      return record.mapping_quality;
   }
   if (name == "cigar") {
      return cigarToString(record.cigar);
   }
   if (name == "mate_rname") {
      return referenceName(record.next_reference_id, references);
   }
   if (name == "mate_pos") {
      return toSamPosition(record.next_position);
   }
   if (name == "tlen") {
      return record.template_length;
   }
   if (name == "qual") {
      return record.quality;
   }
   throw BamException("BAM ingest reached an unsupported metadata column '{}'", name);
}

}  // namespace

class BamNdjsonInputStream::Streambuf : public std::streambuf {
  public:
   Streambuf(std::istream& bam_input, const schema::TableSchema& schema, BamIngestOptions options)
       : reader(bam_input),
         columns(schema.getColumnIdentifiers()),
         options(options) {
      for (const auto& column : columns) {
         if (column.type == schema::ColumnType::NUCLEOTIDE_SEQUENCE) {
            if (sequence_column.has_value()) {
               throw BamException(
                  "BAM ingest supports a single nucleotide-sequence column, but the schema "
                  "declares both '{}' and '{}'",
                  *sequence_column,
                  column.name
               );
            }
            sequence_column = column.name;
         } else if (column.type == schema::ColumnType::AMINO_ACID_SEQUENCE) {
            throw BamException(
               "BAM ingest cannot populate the amino-acid-sequence column '{}'", column.name
            );
         } else if (!isSupportedMetadataColumn(column.name)) {
            throw BamException(
               "BAM ingest has no data for the column '{}'. Supported metadata columns: "
               "read_index, "
               "qname, flag, rname, pos, mapq, cigar, mate_rname, mate_pos, tlen, qual",
               column.name
            );
         }
      }
   }

  protected:
   int_type underflow() override {
      if (gptr() < egptr()) {
         return traits_type::to_int_type(*gptr());
      }
      if (!renderNextLine()) {
         return traits_type::eof();
      }
      setg(current_line.data(), current_line.data(), current_line.data() + current_line.size());
      return traits_type::to_int_type(*gptr());
   }

  private:
   bool shouldSkip(const BamRecord& record) const {
      if (record.isUnmapped() && !options.include_unmapped) {
         return true;
      }
      if (record.isSecondary() && !options.include_secondary) {
         return true;
      }
      if (record.isSupplementary() && !options.include_supplementary) {
         return true;
      }
      return false;
   }

   bool renderNextLine() {
      while (true) {
         auto record = reader.next();
         if (!record.has_value()) {
            return false;
         }
         if (shouldSkip(*record)) {
            continue;
         }

         std::optional<AlignedRead> aligned;
         if (sequence_column.has_value()) {
            if (record->position < 0) {
               SPDLOG_WARN(
                  "BAM ingest: skipping read '{}' with no mapped position", record->read_name
               );
               continue;
            }
            auto projected = projectReadOntoReference(
               static_cast<uint32_t>(record->position), record->sequence, record->cigar
            );
            if (!projected.has_value()) {
               SPDLOG_WARN(
                  "BAM ingest: skipping read '{}': {}", record->read_name, projected.error()
               );
               continue;
            }
            aligned = std::move(projected).value();
         }

         nlohmann::json object;
         for (const auto& column : columns) {
            if (column.type == schema::ColumnType::NUCLEOTIDE_SEQUENCE) {
               object[column.name] = {
                  {"sequence", aligned->aligned_sequence},
                  {"offset", aligned->offset},
                  {"insertions", aligned->insertions},
               };
            } else {
               object[column.name] =
                  metadataValue(column.name, *record, reader.references(), next_read_index);
            }
         }
         ++next_read_index;

         current_line = object.dump();
         current_line.push_back('\n');
         return true;
      }
   }

   BamReader reader;
   std::vector<schema::ColumnIdentifier> columns;
   std::optional<std::string> sequence_column;
   BamIngestOptions options;
   uint64_t next_read_index = 0;
   std::string current_line;
};

BamNdjsonInputStream::BamNdjsonInputStream(
   std::istream& bam_input,
   const schema::TableSchema& schema,
   BamIngestOptions options
)
    : std::istream(nullptr),
      buffer(std::make_unique<Streambuf>(bam_input, schema, options)) {
   rdbuf(buffer.get());
}

BamNdjsonInputStream::~BamNdjsonInputStream() = default;

void appendBamToTable(
   const std::shared_ptr<storage::Table>& table,
   std::istream& bam_input,
   BamIngestOptions options
) {
   BamNdjsonInputStream ndjson_stream{bam_input, *table->schema, options};
   NdjsonLineReader reader{ndjson_stream};
   appendDataToTable(table, reader);
}

}  // namespace rhydb::append::bam
