#include "silo/api/request_id_middleware.h"

#include <crow.h>
#include <spdlog/spdlog.h>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

using boost::uuids::random_generator;

namespace silo::api {

void RequestIdMiddleware::before_handle(
   crow::request& request,
   crow::response& response,
   context& context
) {
   const std::string& request_id_from_header_or_empty = request.get_header_value(REQUEST_ID_HEADER);
   if (request_id_from_header_or_empty.empty()) {
      static thread_local random_generator generator;
      const auto request_id = generator();
      response.set_header(REQUEST_ID_HEADER, boost::uuids::to_string(request_id));
   } else {
      response.set_header(REQUEST_ID_HEADER, request_id_from_header_or_empty);
   }
}

void RequestIdMiddleware::after_handle(
   crow::request& request,
   crow::response& response,
   context& context
) {}

}  // namespace silo::api