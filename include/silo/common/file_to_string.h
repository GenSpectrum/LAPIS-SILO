#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include <yaml-cpp/yaml.h>

namespace silo::common {

std::optional<std::string> fileToString(const std::filesystem::path& path);

}
