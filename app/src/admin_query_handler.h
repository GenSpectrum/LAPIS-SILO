#pragma once

#include <memory>

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

#include <rhydb/config/runtime_config.h>

#include "active_database.h"
#include "rest_resource.h"

namespace rhydb_app {

/// Handles `POST /admin/query`: a write-enabled SaneQL endpoint for append queries of the form
/// `<query>.insertInto(<targetTable>)`. Unlike `POST /query`, which only reads, this mutates the
/// active database in place. It is intentionally kept on a separate `/admin` path because in-place
/// mutation is not safe to run concurrently with other queries against the same database. It is
/// only routed to when `api.allowAdminEndpoint` is enabled; otherwise the path 404s.
class AdminQueryHandler : public RestResource {
  private:
   rhydb::config::QueryOptions query_options;
   std::shared_ptr<ActiveDatabase> database_handle;

  public:
   AdminQueryHandler(
      std::shared_ptr<ActiveDatabase> database_handle,
      rhydb::config::QueryOptions query_options
   );

   void post(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
      override;
};

}  // namespace rhydb_app
