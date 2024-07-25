#include "silo/preprocessing/identifier.h"

namespace silo::preprocessing {

Identifier::Identifier(std::string identifier)
    : raw_identifier(std::move(identifier)) {}

// See https://duckdb.org/docs/sql/dialect/keywords_and_identifiers.html#identifiers
std::string Identifier::escapeIdentifier(const std::string& identifier) {
   std::string output;
   for (const char character : identifier) {
      if (character == '"') {
         output += "\"\"";
      } else {
         output += character;
      }
   }
   return "\"" + output + "\"";
}

const std::string& Identifier::getRawIdentifier() const {
   return raw_identifier;
}

std::string Identifier::escape() const {
   return escapeIdentifier(raw_identifier);
}

}  // namespace silo::preprocessing
