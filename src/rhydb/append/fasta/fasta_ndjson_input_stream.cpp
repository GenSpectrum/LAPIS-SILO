#include "rhydb/append/fasta/fasta_ndjson_input_stream.h"

#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "rhydb/append/fasta/fasta_exception.h"
#include "rhydb/append/fasta/fasta_reader.h"
#include "rhydb/append/ndjson_line_reader.h"
#include "rhydb/append/table_inserter.h"
#include "rhydb/schema/database_schema.h"
#include "rhydb/storage/table.h"

namespace rhydb::append::fasta {

class FastaNdjsonInputStream::Streambuf : public std::streambuf {
  public:
   Streambuf(std::istream& fasta_input, const schema::TableSchema& schema)
       : reader(fasta_input),
         columns(schema.getColumnIdentifiers()),
         primary_key_name(schema.primary_key.name) {
      for (const auto& column : columns) {
         if (schema::isSequenceColumn(column.type)) {
            if (sequence_column.has_value()) {
               throw FastaException(
                  "FASTA ingest supports a single sequence column, but the schema declares both "
                  "'{}' and '{}'",
                  *sequence_column,
                  column.name
               );
            }
            sequence_column = column.name;
         } else if (column.name != primary_key_name) {
            throw FastaException(
               "FASTA ingest has no data for the column '{}'. Declare only the primary-key column "
               "and a single sequence column.",
               column.name
            );
         }
      }
      if (!sequence_column.has_value()) {
         throw FastaException("FASTA ingest requires one sequence column in the table schema");
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
   bool renderNextLine() {
      auto record = reader.next();
      if (!record.has_value()) {
         return false;
      }

      nlohmann::json object;
      for (const auto& column : columns) {
         if (schema::isSequenceColumn(column.type)) {
            object[column.name] = {
               {"sequence", record->sequence},
               {"offset", 0},
               {"insertions", nlohmann::json::array()},
            };
         } else {
            // The only non-sequence column is the primary key (enforced in the ctor).
            object[column.name] = record->identifier;
         }
      }

      current_line = object.dump();
      current_line.push_back('\n');
      return true;
   }

   FastaReader reader;
   std::vector<schema::ColumnIdentifier> columns;
   std::string primary_key_name;
   std::optional<std::string> sequence_column;
   std::string current_line;
};

FastaNdjsonInputStream::FastaNdjsonInputStream(
   std::istream& fasta_input,
   const schema::TableSchema& schema
)
    : std::istream(nullptr),
      buffer(std::make_unique<Streambuf>(fasta_input, schema)) {
   rdbuf(buffer.get());
}

FastaNdjsonInputStream::~FastaNdjsonInputStream() = default;

void appendFastaToTable(const std::shared_ptr<storage::Table>& table, std::istream& fasta_input) {
   FastaNdjsonInputStream ndjson_stream{fasta_input, *table->schema};
   NdjsonLineReader reader{ndjson_stream};
   appendDataToTable(table, reader);
}

}  // namespace rhydb::append::fasta
