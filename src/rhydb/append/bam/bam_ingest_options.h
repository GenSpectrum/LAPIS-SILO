#pragma once

namespace rhydb::append::bam {

/// Controls which BAM alignment records are ingested. By default only primary
/// mapped reads are kept; the flags opt the excluded categories back in.
struct BamIngestOptions {
   bool include_unmapped = false;
   bool include_secondary = false;
   bool include_supplementary = false;
};

}  // namespace rhydb::append::bam
