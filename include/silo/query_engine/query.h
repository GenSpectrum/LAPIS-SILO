#ifndef SILO_QUERY_H
#define SILO_QUERY_H

#include <memory>

namespace silo::query_engine {

namespace filter_expressions {
struct Expression;
}
namespace actions {
struct Action;
}

struct Query {
   std::unique_ptr<filter_expressions::Expression> filter;
   std::unique_ptr<actions::Action> action;

   explicit Query(const std::string& query_string);
};

}  // namespace silo::query_engine

#endif  // SILO_QUERY_H
