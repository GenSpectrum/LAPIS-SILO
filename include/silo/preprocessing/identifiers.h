#pragma once

#include <string>
#include <vector>

#include "silo/preprocessing/identifier.h"

namespace silo::preprocessing {

class Identifiers {
   std::vector<Identifier> identifiers;

  public:
   explicit Identifiers() = default;

   Identifiers(const std::vector<std::string>& raw_identifiers);

   template <typename T>
   void addIdentifier(T&& identifier) {
      identifiers.emplace_back(std::forward<T>(identifier));
   }

   Identifiers prefix(const std::string& prefix) const;

   size_t size() const;

   Identifier getIdentifier(size_t index) const;

   std::vector<std::string> getRawIdentifierStrings() const;

   std::vector<std::string> getEscapedIdentifierStrings() const;
};

}  // namespace silo::preprocessing
