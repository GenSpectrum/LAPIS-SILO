#include <silo/common/InputStreamWrapper.h>
#include <silo/common/silo_symbols.h>
#include <syncstream>

struct separate_thousands : std::numpunct<char> {
   [[nodiscard]] char_type do_thousands_sep() const override { return '\''; }
   [[nodiscard]] string_type do_grouping() const override { return "\3"; }
};

std::string silo::number_fmt(unsigned long n) {
   std::ostringstream oss;
   auto thousands = std::make_unique<separate_thousands>();
   oss.imbue(std::locale(oss.getloc(), thousands.release()));
   oss << n;
   return oss.str();
}
