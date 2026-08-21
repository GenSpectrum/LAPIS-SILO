#pragma once

#include <memory>

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

#include "active_database.h"
#include "rest_resource.h"

namespace rhydb_app {

/// Handles `POST /admin/query`: a write-enabled SaneQL endpoint for append queries of the form
/// `<query>.insertInto(<targetTable>)`. Unlike `POST /query`, which only reads, this mutates the
/// active database in place. It is intentionally kept on a separate `/admin` path because in-place
/// mutation is not safe to run concurrently with other queries against the same database.
class AdminQueryHandler : public RestResource {
  private:
   std::shared_ptr<ActiveDatabase> database_handle;

  public:
   explicit AdminQueryHandler(std::shared_ptr<ActiveDatabase> database_handle);

   void post(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
      override;
};

}  // namespace rhydb_app
