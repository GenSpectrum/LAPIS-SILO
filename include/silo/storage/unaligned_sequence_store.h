#pragma once

#include <cstdint>
#include <deque>
#include <filesystem>
#include <string>

namespace boost::serialization {
class access;
}  // namespace boost::serialization

namespace silo {
class ZstdFastaTableReader;

/// Holds information where to read unaligned sequences for a
/// segment (= the sequence of a particular name) in one partition.
class UnalignedSequenceStorePartition {
   friend class boost::serialization::access;

   std::string sql_for_reading_file;

  public:
   const std::string& compression_dictionary;

   explicit UnalignedSequenceStorePartition(
      std::string sql_for_reading_file,
      const std::string& compression_dictionary
   );

   std::string getReadSQL() const;
};

class UnalignedSequenceStore {
  public:
   std::deque<UnalignedSequenceStorePartition> partitions;
   std::filesystem::path folder_path;
   std::string compression_dictionary;

  private:
   std::filesystem::path partitionFilename(size_t partition_id) const;

  public:
   void saveFolder(const std::filesystem::path& save_location) const;

   explicit UnalignedSequenceStore(
      std::filesystem::path folder_path,
      std::string&& compression_dictionary
   );

   UnalignedSequenceStorePartition& createPartition();
};

}  // namespace silo
