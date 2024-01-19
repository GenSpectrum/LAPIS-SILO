#include "silo/common/format_number.h"

#include <locale>
#include <memory>
#include <sstream>
#include <string>

namespace silo {

struct ThousandSeparator : std::numpunct<char> {
   // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
   [[nodiscard]] char_type do_thousands_sep() const override { return '\''; }
   // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
   [[nodiscard]] string_type do_grouping() const override { return "\3"; }
};

std::string formatNumber(uint64_t number) {
   std::ostringstream oss;
   auto thousands = std::make_unique<ThousandSeparator>();
   oss.imbue(std::locale(oss.getloc(), thousands.release()));
   oss << number;
   return oss.str();
}

}  // namespace silo
