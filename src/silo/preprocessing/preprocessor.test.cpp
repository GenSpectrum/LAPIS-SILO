#include "silo/preprocessing/preprocessor.h"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "silo/config/config_repository.h"
#include "silo/database.h"
#include "silo/database_info.h"
#include "silo/preprocessing/preprocessing_config_reader.h"
#include "silo/query_engine/query_engine.h"

TEST(PreprocessorTest, shouldProcessDataSetWithSequencesThatAreNull) {
   const silo::preprocessing::InputDirectory input_directory{
      "./testBaseData/ndjsonWithNullSequences/"
   };

   auto config = silo::preprocessing::PreprocessingConfigReader()
                    .readConfig(input_directory.directory + "preprocessing_config.yaml")
                    .mergeValuesFromOrDefault(silo::preprocessing::OptionalPreprocessingConfig());

   const auto database_config = silo::config::ConfigRepository().getValidatedConfig(
      input_directory.directory + "database_config.yaml"
   );

   silo::preprocessing::Preprocessor preprocessor(config, database_config);
   auto database = preprocessor.preprocess();

   const auto database_info = database.getDatabaseInfo();

   EXPECT_GT(database_info.total_size, 0);
   EXPECT_EQ(database_info.sequence_count, 5);

   const silo::query_engine::QueryEngine query_engine(database);
   const auto result = query_engine.executeQuery(R"(
      {
         "action": {
           "type": "FastaAligned",
           "sequenceName": ["someShortGene", "secondSegment"],
           "orderByFields": ["accessionVersion"]
         },
         "filterExpression": {
            "type": "True"
         }
      }
   )");

   const auto actual = nlohmann::json(result.query_result);
   const nlohmann::json expected = {
      {{"accessionVersion", "1.1"}, {"someShortGene", "MADS"}, {"secondSegment", "NNNNNNNNNNNNNNNN"}
      },
      {{"accessionVersion", "1.2"}, {"someShortGene", "MADS"}, {"secondSegment", "NNNNNNNNNNNNNNNN"}
      },
      {{"accessionVersion", "1.3"}, {"someShortGene", "XXXX"}, {"secondSegment", "NNNNNNNNNNNNNNNN"}
      },
      {{"accessionVersion", "1.4"}, {"someShortGene", "MADS"}, {"secondSegment", "NNNNNNNNNNNNNNNN"}
      },
      {{"accessionVersion", "1.5"}, {"someShortGene", "MADS"}, {"secondSegment", "NNNNNNNNNNNNNNNN"}
      }
   };
   ASSERT_EQ(actual, expected);
}
