#include "silo/storage/dictionary.h"

#include <gtest/gtest.h>
#include <fstream>

#include "silo/storage/pango_lineage_alias.h"

using silo::Dictionary;

TEST(Dictionary, shouldBuildWithoutErrors) {
   ASSERT_NO_THROW(Dictionary());
}

TEST(Dictionary, shouldHavePredefinedCoulmns) {
   const Dictionary dictionary;

   ASSERT_NO_THROW(dictionary.lookupValueId("country", "someCountry"));
   ASSERT_THROW(dictionary.lookupValueId("noValidColumn", "someValue"), std::exception);
}

class DictionaryTestFixture : public ::testing::Test {
  protected:
   Dictionary dictionary;

   void SetUp() override {
      const std::string directory = "testBaseData/";

      auto alias = silo::PangoLineageAliasLookup::readFromFile(directory);
      dictionary.updateDictionary(directory + "small_metadata_set.tsv", alias);
   }
};

TEST_F(DictionaryTestFixture, shouldUpdateDictionary) {
   ASSERT_TRUE(dictionary.lookupValueId("country", "Switzerland").has_value());
   ASSERT_TRUE(dictionary.lookupValueId("division", "Bern").has_value());
}

TEST_F(DictionaryTestFixture, lookupValueIdshouldReturnEmptyOptionalIfNotExists) {
   ASSERT_FALSE(dictionary.lookupValueId("country", "NotACountry").has_value());
}

TEST_F(DictionaryTestFixture, shouldLookupValueId) {
   ASSERT_EQ(dictionary.lookupValueId("country", "Switzerland"), 0);
   ASSERT_EQ(dictionary.lookupValueId("division", "Bern"), 1);

   ASSERT_EQ(
      dictionary.lookupValueId("division", "Bern"), dictionary.lookupValueId("division", "Bern")
   );

   ASSERT_NE(
      dictionary.lookupValueId("division", "Aargau"), dictionary.lookupValueId("division", "Bern")
   );
}

TEST_F(DictionaryTestFixture, shouldSaveAndLoadDictionary) {
   const std::string test_file = "testBaseData/dictionary_test";
   auto output_stream = std::ofstream(test_file);
   dictionary.saveDictionary(output_stream);

   auto input_stream = std::ifstream(test_file);
   const Dictionary loaded_dictionary = Dictionary::loadDictionary(input_stream);

   ASSERT_EQ(
      dictionary.lookupValueId("division", "Bern"),
      loaded_dictionary.lookupValueId("division", "Bern")
   );

   ASSERT_NO_THROW(std::remove(test_file.c_str()));
}

TEST_F(DictionaryTestFixture, shouldLookupStringValue) {
   ASSERT_EQ(dictionary.lookupStringValue("country", 0), "Switzerland");
   ASSERT_EQ(dictionary.lookupStringValue("division", 1), "Bern");
}

TEST_F(DictionaryTestFixture, shouldLookupPangoLineageValue) {
   const std::string pango_lineage = "B.1.1.7";
   ASSERT_EQ(dictionary.lookupPangoLineageValue("pango_lineage", 0), pango_lineage);

   const std::string pango_lineage2 = "B.1.258.17";
   ASSERT_EQ(dictionary.lookupPangoLineageValue("pango_lineage", 1), pango_lineage2);
}
