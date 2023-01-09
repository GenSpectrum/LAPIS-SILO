//
// Created by Alexander Taepper on 27.09.22.
//

#ifndef SILO_QUERY_ENGINE_H
#define SILO_QUERY_ENGINE_H

#include "silo/database.h"
#include <string>

namespace silo {

struct QueryParseException : public std::exception {
   private:
   const char* message;

   public:
   explicit QueryParseException(const std::string& msg) : message(msg.c_str()) {}

   [[nodiscard]] const char* what() const noexcept override {
      return message;
   }
};

struct result_s {
   std::string return_message;
   int64_t parse_time;
   int64_t filter_time;
   int64_t action_time;
};

/// The return value of the BoolExpression::evaluate method.
/// May return either a mutable or immutable bitmap.
struct filter_t {
   roaring::Roaring* mutable_res;
   const roaring::Roaring* immutable_res;

   inline const roaring::Roaring* getAsConst() {
      return mutable_res ? mutable_res : immutable_res;
   }

   inline void free() {
      if (mutable_res) delete mutable_res;
   }
};

class mutation_proportion {
   public:
   double proportion;
   unsigned position;
   unsigned count;
   char mut_from;
   char mut_to;

   mutation_proportion(char mut_from, unsigned position, char mut_to, double proportion, unsigned count)
      : mut_from(mut_from), position(position), mut_to(mut_to), proportion(proportion), count(count) {}
};

/// Action
std::vector<mutation_proportion> execute_mutations(const silo::Database&, std::vector<silo::filter_t>&, double proportion_threshold);

uint64_t execute_count(const silo::Database& /*db*/, std::vector<silo::filter_t>& partition_filters);

/// Filter then call action
result_s execute_query(const Database& db, const std::string& query, std::ostream& res_out, std::ostream& perf_out);

} // namespace silo;

#endif //SILO_QUERY_ENGINE_H
