#pragma once

#include <string>
#include <vector>

#include "silo/preprocessing/identifier.h"

namespace silo::preprocessing {

class Identifiers {
   std::vector<Identifier> identifiers;

  public:
   Identifiers(const std::vector<std::string>& raw_identifiers);

   Identifiers prefix(const std::string& prefix) const;

   size_t size() const;

   Identifier getIdentifier(size_t index) const;

   std::vector<std::string> getRawIdentifierStrings() const;

   std::vector<std::string> getEscapedIdentifierStrings() const;
};

}  // namespace silo::preprocessing
