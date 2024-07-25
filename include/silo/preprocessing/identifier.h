#pragma once

#include <string>
#include <vector>

namespace silo::preprocessing {

class Identifier {
   std::string raw_identifier;

  public:
   explicit Identifier(std::string identifier);

   static std::string escapeIdentifier(const std::string& identifier);

   const std::string& getRawIdentifier() const;

   std::string escape() const;
};

}  // namespace silo::preprocessing
