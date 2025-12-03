#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <nlohmann/json.hpp>

#include "silo/test/query_fixture.test.h"

namespace {
using silo::ReferenceGenomes;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

using boost::uuids::random_generator;

nlohmann::json createDataWithNucleotideSequence(const std::string& nucleotideSequence) {
   static std::atomic_int row_id = 0;
   const auto primary_key = row_id++;

   return nlohmann::json::parse(fmt::format(
      R"(
{{

   "primaryKey": "id_{}",
   "country": "Switzerland",
   "segment1": {{
      "sequence": "{}",
      "insertions": []
   }},
   "unaligned_segment1": null,
   "gene1": null
}}
)",
      primary_key,
      nucleotideSequence
   ));
}

const nlohmann::json DATA_SAME_AS_REFERENCE = createDataWithNucleotideSequence("ATGCN");
const nlohmann::json DATA_SAME_AS_REFERENCE2 = createDataWithNucleotideSequence("ATGCN");
const nlohmann::json DATA_WITH_ALL_N = createDataWithNucleotideSequence("NNNNN");
const nlohmann::json DATA_WITH_ALL_MUTATED = createDataWithNucleotideSequence("CATTT");

const auto DATABASE_CONFIG =
   R"(
defaultNucleotideSequence: "segment1"
schema:
  instanceName: "dummy name"
  metadata:
   - name: "primaryKey"
     type: "string"
   - name: "country"
     type: "string"
  primaryKey: "primaryKey"
)";

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "ATGCN"}},
   {{"gene1", "M*"}},
};

const QueryTestData TEST_DATA{
   .ndjson_input_data =
      {DATA_SAME_AS_REFERENCE, DATA_SAME_AS_REFERENCE2, DATA_WITH_ALL_N, DATA_WITH_ALL_MUTATED},
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES
};

const QueryTestScenario FASTA_ALIGNED = {
   .name = "FASTA_ALIGNED",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "FastaAligned",
    "sequenceNames": [
      "segment1"
    ],
    "orderByFields": [
      "primaryKey"
    ]
  },
  "filterExpression": {
    "type": "True"
  }
})"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"primaryKey":"id_0","segment1":"ATGCN"},
{"primaryKey":"id_1","segment1":"ATGCN"},
{"primaryKey":"id_2","segment1":"NNNNN"},
{"primaryKey":"id_3","segment1":"CATTT"}])"
   )
};

const QueryTestScenario FASTA_ALIGNED_ADDITIONAL_HEADER = {
   .name = "FASTA_ALIGNED_ADDITIONAL_HEADER",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "FastaAligned",
    "sequenceNames": [
      "segment1"
    ],
    "orderByFields": [
      "primaryKey"
    ],
    "additionalFields": [
      "country"
    ]
  },
  "filterExpression": {
    "type": "True"
  }
}
)"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"country":"Switzerland","primaryKey":"id_0","segment1":"ATGCN"},
{"country":"Switzerland","primaryKey":"id_1","segment1":"ATGCN"},
{"country":"Switzerland","primaryKey":"id_2","segment1":"NNNNN"},
{"country":"Switzerland","primaryKey":"id_3","segment1":"CATTT"}])"
   )
};

const QueryTestScenario DUPLICATE_FIELDS = {
   .name = "DUPLICATE_FIELDS",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "FastaAligned",
    "sequenceNames": [
      "segment1",
      "segment1"
    ],
    "orderByFields": [
      "primaryKey"
    ],
    "additionalFields": [
      "country",
      "country"
    ]
  },
  "filterExpression": {
    "type": "True"
  }
}
)"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"country":"Switzerland","primaryKey":"id_0","segment1":"ATGCN"},
{"country":"Switzerland","primaryKey":"id_1","segment1":"ATGCN"},
{"country":"Switzerland","primaryKey":"id_2","segment1":"NNNNN"},
{"country":"Switzerland","primaryKey":"id_3","segment1":"CATTT"}])"
   )
};

const QueryTestScenario FASTA_ALIGNED_EXPLICIT_PRIMARY_KEY = {
   .name = "FASTA_ALIGNED_EXPLICIT_PRIMARY_KEY",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "FastaAligned",
    "sequenceNames": [
      "segment1"
    ],
    "additionalFields": [
      "primaryKey"
    ]
  },
  "filterExpression": {
    "type": "True"
  }
}
)"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"primaryKey":"id_0","segment1":"ATGCN"},
{"primaryKey":"id_1","segment1":"ATGCN"},
{"primaryKey":"id_2","segment1":"NNNNN"},
{"primaryKey":"id_3","segment1":"CATTT"}])"
   )
};

const QueryTestScenario FASTA_ALIGNED_DUPLICATE_HEADER = {
   .name = "FASTA_ALIGNED_DUPLICATE_HEADER",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "FastaAligned",
    "sequenceNames": [
      "segment1"
    ],
    "additionalFields": [
      "country",
      "primaryKey",
      "country"
    ]
  },
  "filterExpression": {
    "type": "True"
  }
}
)"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"country":"Switzerland","primaryKey":"id_0","segment1":"ATGCN"},
{"country":"Switzerland","primaryKey":"id_1","segment1":"ATGCN"},
{"country":"Switzerland","primaryKey":"id_2","segment1":"NNNNN"},
{"country":"Switzerland","primaryKey":"id_3","segment1":"CATTT"}])"
   )
};

const QueryTestScenario FASTA_ALIGNED_DESCENDING = {
   .name = "FASTA_ALIGNED_DESCENDING",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "FastaAligned",
    "sequenceNames": [
      "segment1"
    ],
    "orderByFields": [
      {
        "field": "primaryKey",
        "order": "descending"
      }
    ],
    "additionalFields": [
      "country"
    ]
  },
  "filterExpression": {
    "type": "True"
  }
}
)"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"country":"Switzerland","primaryKey":"id_3","segment1":"CATTT"},
{"country":"Switzerland","primaryKey":"id_2","segment1":"NNNNN"},
{"country":"Switzerland","primaryKey":"id_1","segment1":"ATGCN"},
{"country":"Switzerland","primaryKey":"id_0","segment1":"ATGCN"}])"
   )
};

const QueryTestScenario FASTA_ALIGNED_SUBSET = {
   .name = "FASTA_ALIGNED_SUBSET",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "FastaAligned",
    "sequenceNames": [
      "segment1"
    ]
  },
  "filterExpression": {
    "type": "Or",
    "children": [
      {
        "type": "StringEquals",
        "column": "primaryKey",
        "value": "id_0"
      },
      {
        "type": "StringEquals",
        "column": "primaryKey",
        "value": "id_2"
      },
      {
        "type": "StringEquals",
        "column": "primaryKey",
        "value": "id_3"
      }
    ]
  }
}
)"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"primaryKey":"id_0","segment1":"ATGCN"},
{"primaryKey":"id_2","segment1":"NNNNN"},
{"primaryKey":"id_3","segment1":"CATTT"}])"
   )
};

const QueryTestScenario FASTA_ALIGNED_SMALL_BATCHES = {
   .name = "FASTA_ALIGNED_SMALL_BATCHES",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "FastaAligned",
    "sequenceNames": [
      "segment1"
    ],
    "additionalFields": [
      "country"
    ],
    "orderByFields": [
      "country","primaryKey"
    ]
  },
  "filterExpression": {
    "type": "True"
  }
}
)"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"country":"Switzerland","primaryKey":"id_0","segment1":"ATGCN"},
{"country":"Switzerland","primaryKey":"id_1","segment1":"ATGCN"},
{"country":"Switzerland","primaryKey":"id_2","segment1":"NNNNN"},
{"country":"Switzerland","primaryKey":"id_3","segment1":"CATTT"}])"
   ),
   .query_options = silo::config::QueryOptions{.materialization_cutoff = 0}
};

const QueryTestScenario FASTA_ALIGNED_WITH_OFFSET = {
   .name = "FASTA_ALIGNED_WITH_OFFSET",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "FastaAligned",
    "sequenceNames": [
      "segment1"
    ],
    "orderByFields": [
      "primaryKey"
    ],
    "offset": 2
  },
  "filterExpression": {
    "type": "True"
  }
}
)"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"primaryKey":"id_2","segment1":"NNNNN"},
{"primaryKey":"id_3","segment1":"CATTT"}])"
   ),
   .query_options = silo::config::QueryOptions{.materialization_cutoff = 1}
};

const QueryTestScenario FASTA_ALIGNED_WITH_LIMIT = {
   .name = "FASTA_ALIGNED_WITH_LIMIT",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "FastaAligned",
    "sequenceNames": [
      "segment1"
    ],
    "orderByFields": [
      {
        "field": "primaryKey",
        "order": "descending"
      }
    ],
    "limit": 3
  },
  "filterExpression": {
    "type": "True"
  }
}
)"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"primaryKey":"id_3","segment1":"CATTT"},
{"primaryKey":"id_2","segment1":"NNNNN"},
{"primaryKey":"id_1","segment1":"ATGCN"}])"
   ),
   .query_options = silo::config::QueryOptions{.materialization_cutoff = 1}
};

const QueryTestScenario FASTA_ALIGNED_WITH_LIMIT_UNSORTED = {
   .name = "FASTA_ALIGNED_WITH_LIMIT_UNSORTED",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "FastaAligned",
    "sequenceNames": [
      "segment1"
    ],
    "limit": 3
  },
  "filterExpression": {
    "type": "True"
  }
}
)"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"primaryKey":"id_0","segment1":"ATGCN"},
{"primaryKey":"id_1","segment1":"ATGCN"},
{"primaryKey":"id_2","segment1":"NNNNN"}])"
   ),
   .query_options = silo::config::QueryOptions{.materialization_cutoff = 1}
};

const QueryTestScenario FASTA_ALIGNED_WITH_OFFSET_AND_LIMIT = {
   .name = "FASTA_ALIGNED_WITH_OFFSET_AND_LIMIT",
   .query = nlohmann::json::parse(
      R"(
{
  "action": {
    "type": "FastaAligned",
    "sequenceNames": [
      "segment1"
    ],
    "orderByFields": [
      "primaryKey"
    ],
    "offset": 2,
    "limit": 1
  },
  "filterExpression": {
    "type": "True"
  }
}
)"
   ),
   .expected_query_result = nlohmann::json::parse(
      R"(
[{"primaryKey":"id_2","segment1":"NNNNN"}])"
   ),
   .query_options = silo::config::QueryOptions{.materialization_cutoff = 1}
};

}  // namespace

QUERY_TEST(
   FastaAligned,
   TEST_DATA,
   ::testing::Values(
      FASTA_ALIGNED,
      FASTA_ALIGNED_ADDITIONAL_HEADER,
      FASTA_ALIGNED_DUPLICATE_HEADER,
      DUPLICATE_FIELDS,
      FASTA_ALIGNED_EXPLICIT_PRIMARY_KEY,
      FASTA_ALIGNED_DESCENDING,
      FASTA_ALIGNED_SUBSET,
      FASTA_ALIGNED_SMALL_BATCHES,
      FASTA_ALIGNED_WITH_LIMIT,
      FASTA_ALIGNED_WITH_LIMIT_UNSORTED,
      FASTA_ALIGNED_WITH_OFFSET,
      FASTA_ALIGNED_WITH_OFFSET_AND_LIMIT
   )
);
