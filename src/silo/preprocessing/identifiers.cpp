#include "silo/preprocessing/identifiers.h"

namespace silo::preprocessing {

Identifiers::Identifiers(const std::vector<std::string>& raw_identifiers) {
   for (const std::string& identifier : raw_identifiers) {
      identifiers.emplace_back(identifier);
   }
}

Identifiers Identifiers::prefix(const std::string& prefix) const {
   std::vector<std::string> prefixed_identifiers;
   prefixed_identifiers.reserve(identifiers.size());
   for (const auto& identifier : identifiers) {
      prefixed_identifiers.emplace_back(prefix + identifier.getRawIdentifier());
   }
   return {prefixed_identifiers};
}

size_t Identifiers::size() const {
   return identifiers.size();
}

Identifier Identifiers::getIdentifier(size_t index) const {
   return identifiers[index];
}

std::vector<std::string> Identifiers::getRawIdentifierStrings() const {
   std::vector<std::string> output;
   output.reserve(identifiers.size());
   for (const auto& identifier : identifiers) {
      output.emplace_back(identifier.getRawIdentifier());
   }
   return output;
}

std::vector<std::string> Identifiers::getEscapedIdentifierStrings() const {
   std::vector<std::string> output;
   output.reserve(identifiers.size());
   for (const auto& identifier : identifiers) {
      output.emplace_back(identifier.escape());
   }
   return output;
}

}  // namespace silo::preprocessing
