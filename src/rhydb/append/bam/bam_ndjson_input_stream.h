#pragma once

#include <istream>
#include <memory>

#include "rhydb/append/bam/bam_ingest_options.h"

namespace rhydb::schema {
class TableSchema;
}
namespace rhydb::storage {
class Table;
}

namespace rhydb::append::bam {

/// An std::istream that renders a BAM byte stream as NDJSON on demand: one JSON
/// object per kept read, keyed by the target table's column names. Because it is
/// a plain istream, the ordinary NDJSON append pipeline (NdjsonLineReader ->
/// TableInserter -> finalize) consumes it without any changes.
///
/// The declared columns are drawn from this canonical BAM field set:
///   read_index (synthetic 0-based counter), qname, flag, rname, pos (1-based),
///   mapq, cigar, mate_rname, mate_pos (1-based), tlen, qual,
/// plus the single NUCLEOTIDE_SEQUENCE column, which receives the read projected
/// onto reference coordinates as {sequence, offset, insertions}.
class BamNdjsonInputStream : public std::istream {
  public:
   BamNdjsonInputStream(
      std::istream& bam_input,
      const schema::TableSchema& schema,
      BamIngestOptions options = {}
   );
   ~BamNdjsonInputStream() override;

   BamNdjsonInputStream(const BamNdjsonInputStream&) = delete;
   BamNdjsonInputStream& operator=(const BamNdjsonInputStream&) = delete;

  private:
   class Streambuf;
   std::unique_ptr<Streambuf> buffer;
};

/// Append every kept read of the BAM byte stream `bam_input` into `table`,
/// reusing the standard NDJSON append + finalize + validate pipeline. Rows are
/// keyed by the table's declared column names (the canonical set above).
void appendBamToTable(
   const std::shared_ptr<storage::Table>& table,
   std::istream& bam_input,
   BamIngestOptions options = {}
);

}  // namespace rhydb::append::bam
