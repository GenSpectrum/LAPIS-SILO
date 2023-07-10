#include "silo_api/manual_poco_mocks.test.h"

#include <Poco/Net/HTTPServerResponse.h>
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/SocketAddress.h"

namespace silo_api::test {

void MockResponse::sendContinue() {}

std::ostream& MockResponse::send() {
   return out_stream;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void MockResponse::sendFile(const std::string& path, const std::string& mediaType) {}

void MockResponse::sendBuffer(const void* pBuffer, std::size_t length) {}

void MockResponse::redirect(const std::string& uri, HTTPStatus status) {}

void MockResponse::requireAuthentication(const std::string& realm) {}

bool MockResponse::sent() const {
   return true;
}

MockRequest::MockRequest(MockResponse& mockResponse)
    : mockResponse(mockResponse) {}

std::istream& MockRequest::stream() {
   return in_stream;
}

const Poco::Net::SocketAddress& MockRequest::clientAddress() const {
   return address;
}

const Poco::Net::SocketAddress& MockRequest::serverAddress() const {
   return address;
}

const Poco::Net::HTTPServerParams& MockRequest::serverParams() const {
   return *params;
}

Poco::Net::HTTPServerResponse& MockRequest::response() const {
   return mockResponse;
}

bool MockRequest::secure() const {
   return false;
}

}  // namespace silo_api::test