#include "silo/preprocessing/lineage_definition_file.h"

#include <gtest/gtest.h>
#include <yaml-cpp/exceptions.h>

#include "silo/preprocessing/preprocessing_exception.h"

using silo::preprocessing::LineageDefinitionFile;

TEST(LineageDefinitionFile, correctlyParsesFromYAML) {
   auto lineage_definition_file = LineageDefinitionFile::fromYAML(R"(
BASE:
  parents: []
CHILD:
  parents:
    - BASE
)");
   ASSERT_EQ(lineage_definition_file.lineages.size(), 2);
   ASSERT_EQ(lineage_definition_file.lineages.at(0).lineage_name.string, "BASE");
   ASSERT_TRUE(lineage_definition_file.lineages.at(0).parents.empty());
   ASSERT_EQ(lineage_definition_file.lineages.at(1).lineage_name.string, "CHILD");
   ASSERT_EQ(lineage_definition_file.lineages.at(1).parents.size(), 1);
   ASSERT_EQ(lineage_definition_file.lineages.at(1).parents.at(0).string, "BASE");
}

TEST(LineageDefinitionFile, unparsableOnBadFormat) {
   ASSERT_THROW(
      LineageDefinitionFile::fromYAML(R"(
X
SOME_lineage: "
-:
)"),
      YAML::Exception
   );
}

TEST(LineageDefinitionFile, errorOnDuplicateKey) {
   auto throwing_lambda = []() {
      LineageDefinitionFile::fromYAML(R"(
some_duplicate_lineage:
  parents:
  - anything
some_duplicate_lineage:
  parents:
  - also_anything)");
   };

   EXPECT_THROW(
      {
         try {
            throwing_lambda();
         } catch (const silo::preprocessing::PreprocessingException& e) {
            ASSERT_EQ(
               std::string(e.what()),
               "The lineage definitions contain the duplicate lineage 'some_duplicate_lineage'"
            );
            throw;
         }
      },
      silo::preprocessing::PreprocessingException
   );
}

TEST(LineageDefinitionFile, errorOnMisspelledParents) {
   auto throwing_lambda = []() {
      LineageDefinitionFile::fromYAML(R"(
some_lineage:
  parent:
  - anything
some_other_lineage:
  parent:
  - also_anything)");
   };

   EXPECT_THROW(
      {
         try {
            throwing_lambda();
         } catch (const silo::preprocessing::PreprocessingException& e) {
            ASSERT_EQ(
               std::string(e.what()),
               "The lineage 'some_lineage' does not contain the field 'parents'"
            );
            throw;
         }
      },
      silo::preprocessing::PreprocessingException
   );
}

TEST(LineageDefinitionFile, errorOnEmptyMap) {
   auto throwing_lambda = []() {
      LineageDefinitionFile::fromYAML(R"(
some_lineage: {}
some_other_lineage:
  parents:
  - also_anything)");
   };

   EXPECT_THROW(
      {
         try {
            throwing_lambda();
         } catch (const silo::preprocessing::PreprocessingException& e) {
            ASSERT_EQ(
               std::string(e.what()),
               "The lineage 'some_lineage' does not contain the field 'parents'"
            );
            throw;
         }
      },
      silo::preprocessing::PreprocessingException
   );
}

TEST(LineageDefinitionFile, errorOnExtraFields) {
   auto throwing_lambda = []() {
      LineageDefinitionFile::fromYAML(R"(
some_lineage:
  parents: []
  some_extra_field: "some_value"
some_other_lineage:
  parents:
  - also_anything)");
   };

   EXPECT_THROW(
      {
         try {
            throwing_lambda();
         } catch (const silo::preprocessing::PreprocessingException& e) {
            ASSERT_EQ(
               std::string(e.what()),
               "The definition of lineage 'some_lineage' contains the invalid fields (only "
               "'parents' is allowed): parents: []\nsome_extra_field: some_value"
            );
            throw;
         }
      },
      silo::preprocessing::PreprocessingException
   );
}

TEST(LineageDefinitionFile, errorOnNullLineageMap) {
   auto throwing_lambda = []() {
      LineageDefinitionFile::fromYAML(R"(
some_lineage:
some_other_lineage:
  parents:
  - some_lineage
)");
   };

   EXPECT_THROW(
      {
         try {
            throwing_lambda();
         } catch (const silo::preprocessing::PreprocessingException& e) {
            ASSERT_EQ(
               std::string(e.what()),
               "The lineage 'some_lineage' is not defined as a valid YAML Map in its definition: ~"
            );
            throw;
         }
      },
      silo::preprocessing::PreprocessingException
   );
}
