#pragma once

#include <expected>
#include <string>
#include <string_view>

namespace silo {

/// Decodes a standard base64-encoded string to binary data.
/// Returns a user-legible error string if the input cannot be decoded
std::expected<std::string, std::string> decodeBase64(std::string_view encoded);

}  // namespace silo
