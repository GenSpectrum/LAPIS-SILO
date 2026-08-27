#pragma once

#include <istream>
#include <memory>

namespace rhydb::schema {
class TableSchema;
}
namespace rhydb::storage {
class Table;
}

namespace rhydb::append::fasta {

/// An std::istream that renders a FASTA file as NDJSON on demand: one JSON object
/// per record, keyed by the target table's column names. Because it is a plain
/// istream, the ordinary NDJSON append pipeline consumes it unchanged.
///
/// FASTA carries no alignment or metadata, so the table schema must be just a
/// primary-key column (which receives the record identifier) and a single
/// sequence column (nucleotide or amino acid), which receives the sequence as
/// {sequence, offset: 0, insertions: []}. The sequence is taken to be already
/// aligned to that column's reference (length <= reference length, gaps as '-').
class FastaNdjsonInputStream : public std::istream {
  public:
   FastaNdjsonInputStream(std::istream& fasta_input, const schema::TableSchema& schema);
   ~FastaNdjsonInputStream() override;

   FastaNdjsonInputStream(const FastaNdjsonInputStream&) = delete;
   FastaNdjsonInputStream& operator=(const FastaNdjsonInputStream&) = delete;

  private:
   class Streambuf;
   std::unique_ptr<Streambuf> buffer;
};

/// Append every record of the FASTA byte stream `fasta_input` into `table`,
/// reusing the standard NDJSON append + finalize + validate pipeline.
void appendFastaToTable(const std::shared_ptr<storage::Table>& table, std::istream& fasta_input);

}  // namespace rhydb::append::fasta
