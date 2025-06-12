#pragma once

#include <cinttypes>
#include <map>
#include <vector>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include "silo/common/format_number.h"
#include "silo/common/nucleotide_symbols.h"

namespace silo {

struct DatabaseInfo {
   std::string_view version;
   uint32_t sequence_count;
   uint64_t vertical_bitmaps_size;
   uint64_t horizontal_bitmaps_size;
   uint64_t number_of_partitions;
};

// NOLINTNEXTLINE(readability-identifier-naming,misc-use-internal-linkage)
void to_json(nlohmann::json& json, const silo::DatabaseInfo& databaseInfo);

}  // namespace silo

template <>
class [[maybe_unused]] fmt::formatter<silo::DatabaseInfo> {
  public:
   constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
   [[maybe_unused]] static auto format(silo::DatabaseInfo database_info, format_context& ctx)
      -> decltype(ctx.out()) {
      return fmt::format_to(ctx.out(), "{}", nlohmann::json{database_info}.dump());
   }
};
