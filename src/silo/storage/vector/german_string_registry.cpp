#include "silo/storage/vector/german_string_registry.h"

namespace silo::storage::vector {

Idx GermanStringRegistry::insert(const silo::SiloString& silo_string) {
   if (german_string_pages.empty()) {
      german_string_pages.emplace_back();
   } else if (german_string_pages.back().full()) {
      german_string_pages.emplace_back();
   }
   size_t page_id = german_string_pages.size() - 1;
   size_t row_in_page = german_string_pages.back().insert(silo_string);
   return page_id * GermanStringPage::MAX_STRINGS_PER_PAGE + row_in_page;
}

SiloString GermanStringRegistry::get(Idx row_id) const {
   Idx page_id = row_id / GermanStringPage::MAX_STRINGS_PER_PAGE;
   Idx row_in_page = row_id - (page_id * GermanStringPage::MAX_STRINGS_PER_PAGE);
   return german_string_pages.at(page_id).get(row_in_page);
}

}  // namespace silo::storage::vector