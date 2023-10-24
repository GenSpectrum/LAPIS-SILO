#include <gtest/gtest.h>
#include <optional>
#include <string>

#include <duckdb.hpp>

#include "silo/common/fasta_reader.h"
#include "silo/zstdfasta/zstd_decompressor.h"
#include "silo/zstdfasta/zstdfasta_reader.h"
#include "silo/zstdfasta/zstdfasta_table.h"
#include "silo/zstdfasta/zstdfasta_table_reader.h"

TEST(ZstdFastaTableReader, correctlyReadsZstdFastaTableFromFastaFile) {
   const std::filesystem::path input_directory{"testBaseData/fastaFiles/"};
   const std::string sequence_filename{"test.fasta"};

   std::filesystem::path file_path = input_directory / sequence_filename;

   silo::FastaReader file_reader(file_path);

   duckdb::DuckDB duckDB(nullptr);
   duckdb::Connection connection(duckDB);

   silo::ZstdFastaTable::generate(connection, "test", file_reader, "ACGT");

   silo::ZstdFastaTableReader under_test(connection, "test", "ACGT", "true", "");

   std::optional<std::string> key;
   std::string genome;

   EXPECT_TRUE(key = under_test.next(genome));
   EXPECT_EQ(key, "Key1");
   EXPECT_EQ(genome, "ACGT");

   EXPECT_TRUE(key = under_test.next(genome));
   EXPECT_EQ(key, "Key2");
   EXPECT_EQ(genome, "CGTA");

   EXPECT_FALSE(key = under_test.next(genome));
}

TEST(ZstdFastaTableReader, correctlyReadsZstdFastaTableFromZstdFastaFile) {
   const std::filesystem::path input_directory{"testBaseData/fastaFiles/"};
   const std::string sequence_filename{"test.zstdfasta"};

   std::filesystem::path file_path = input_directory / sequence_filename;

   silo::ZstdFastaReader file_reader(file_path, "ACGT");

   duckdb::DuckDB duckDB(nullptr);
   duckdb::Connection connection(duckDB);

   silo::ZstdFastaTable::generate(connection, "test", file_reader);

   silo::ZstdFastaTableReader under_test(connection, "test", "ACGT", "true", "");

   std::optional<std::string> key;
   std::string genome;

   EXPECT_TRUE(key = under_test.next(genome));
   EXPECT_EQ(key, "Key1");
   EXPECT_EQ(genome, "ACGT");

   EXPECT_TRUE(key = under_test.next(genome));
   EXPECT_EQ(key, "Key2");
   EXPECT_EQ(genome, "CGTA");

   EXPECT_FALSE(key = under_test.next(genome));
}

TEST(ZstdFastaTableReader, correctlySortsZstdFastaTableFromFastaFile) {
   const std::filesystem::path input_directory{"testBaseData/fastaFiles/"};
   const std::string sequence_filename{"test.fasta"};

   std::filesystem::path file_path = input_directory / sequence_filename;

   silo::FastaReader file_reader(file_path);

   duckdb::DuckDB duckDB(nullptr);
   duckdb::Connection connection(duckDB);

   silo::ZstdFastaTable::generate(connection, "test", file_reader, "ACGT");

   silo::ZstdFastaTableReader under_test(connection, "test", "ACGT", "true", "ORDER BY key desc");

   std::optional<std::string> key;
   std::string genome;

   EXPECT_TRUE(key = under_test.next(genome));
   EXPECT_EQ(key, "Key2");
   EXPECT_EQ(genome, "CGTA");

   EXPECT_TRUE(key = under_test.next(genome));
   EXPECT_EQ(key, "Key1");
   EXPECT_EQ(genome, "ACGT");

   EXPECT_FALSE(key = under_test.next(genome));
}

TEST(ZstdFastaTableReader, correctlySortsZstdFastaTableFromZstdFastaFile) {
   const std::filesystem::path input_directory{"testBaseData/fastaFiles/"};
   const std::string sequence_filename{"test.zstdfasta"};

   std::filesystem::path file_path = input_directory / sequence_filename;

   silo::ZstdFastaReader file_reader(file_path, "ACGT");

   duckdb::DuckDB duckDB(nullptr);
   duckdb::Connection connection(duckDB);

   silo::ZstdFastaTable::generate(connection, "test", file_reader);

   silo::ZstdFastaTableReader under_test(connection, "test", "ACGT", "true", "ORDER BY key desc");

   std::optional<std::string> key;
   std::string genome;

   EXPECT_TRUE(key = under_test.next(genome));
   EXPECT_EQ(key, "Key2");
   EXPECT_EQ(genome, "CGTA");

   EXPECT_TRUE(key = under_test.next(genome));
   EXPECT_EQ(key, "Key1");
   EXPECT_EQ(genome, "ACGT");

   EXPECT_FALSE(key = under_test.next(genome));
}