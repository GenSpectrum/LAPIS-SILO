#include <gtest/gtest.h>
#include <string>
#include <unordered_map>

#include "silo/config/preprocessing_config.h"
#include "silo/sequence_file_reader/sam_format_exception.h"
#include "silo/sequence_file_reader/sam_reader.h"

using silo::sequence_file_reader::SamFormatException;
using silo::sequence_file_reader::SamReader;

TEST(SamReader, shouldReadSamFile) {
   const std::string file_content{
      R"(
1.1	99	NC_045512.2	10	60	5S246M	=	201	404	TATACCTTCCCAGGTAACAAACCAACCAACTTTCGATCT	CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC5CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC5CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC	NM:i:1	MD:Z:197C48	MC:Z:247M	AS:i:241	XS:i:0
1.3	99	NC_045512.2	5	60	47S204M	=	209	400	TATACCTTCCCAGGTAACAAACCAACCAACTTTTTTTTT	CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC5CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC5CCCCCCCCCCCCCCCCC*CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC	NM:i:1	MD:Z:196C7	MC:Z:236M	AS:i:199	XS:i:0	SA:Z:NC_045512.2,7769,+,30M221S,60,0;
)"
   };

   SamReader under_test(file_content);

   auto entry = under_test.nextEntry();
   EXPECT_TRUE(entry.has_value());
   auto [key, offset, genome] = entry.value();
   EXPECT_EQ(key, "1.1");
   EXPECT_EQ(offset, 10);
   EXPECT_EQ(genome, "TATACCTTCCCAGGTAACAAACCAACCAACTTTCGATCT");

   entry = under_test.nextEntry();
   EXPECT_TRUE(entry.has_value());
   key = entry.value().key;
   offset = entry.value().offset;
   genome = entry.value().sequence;
   EXPECT_EQ(key, "1.3");
   EXPECT_EQ(offset, 5);
   EXPECT_EQ(genome, "TATACCTTCCCAGGTAACAAACCAACCAACTTTTTTTTT");

   entry = under_test.nextEntry();
   EXPECT_FALSE(entry.has_value());
}

TEST(SamReader, shouldReadSamFileWithBlankLines) {
   const std::string file_content{
      R"(

1.1	99	NC_045512.2	10	60	5S246M	=	201	404	TATACCTTCCCAGGTAACAAACCAACCAACTTTCGATCT	CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC5CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC5CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC	NM:i:1	MD:Z:197C48	MC:Z:247M	AS:i:241	XS:i:0


1.3	99	NC_045512.2	5	60	47S204M	=	209	400	TATACCTTCCCAGGTAACAAACCAACCAACTTTTTTTTT	CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC5CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC5CCCCCCCCCCCCCCCCC*CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC	NM:i:1	MD:Z:196C7	MC:Z:236M	AS:i:199	XS:i:0	SA:Z:NC_045512.2,7769,+,30M221S,60,0;
)"
   };

   SamReader under_test(file_content);

   auto entry = under_test.nextEntry();
   EXPECT_TRUE(entry.has_value());
   auto [key, offset, genome] = entry.value();
   EXPECT_EQ(key, "1.1");
   EXPECT_EQ(offset, 10);
   EXPECT_EQ(genome, "TATACCTTCCCAGGTAACAAACCAACCAACTTTCGATCT");

   entry = under_test.nextEntry();
   EXPECT_TRUE(entry.has_value());
   key = entry.value().key;
   offset = entry.value().offset;
   genome = entry.value().sequence;
   EXPECT_EQ(key, "1.3");
   EXPECT_EQ(offset, 5);
   EXPECT_EQ(genome, "TATACCTTCCCAGGTAACAAACCAACCAACTTTTTTTTT");

   entry = under_test.nextEntry();
   EXPECT_FALSE(entry.has_value());
}

TEST(SamReader, givenDataInWrongFormatThenShouldThrowAnException) {
   const std::string file_content{"Wrong format"};

   SamReader under_test(file_content);

   EXPECT_THROW(under_test.nextEntry(), SamFormatException);
}

TEST(SamReader, givenDataInWithMissingGenomeThenShouldThrowAnException) {
   const std::string file_content{
      R"(
1.1	99	NC_045512.2	10	60	5S246M	=	201	404		CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC5CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC5CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC	NM:i:1	MD:Z:197C48	MC:Z:247M	AS:i:241	XS:i:0
)"
   };

   SamReader under_test(file_content);

   EXPECT_THROW(under_test.nextEntry(), SamFormatException);
}