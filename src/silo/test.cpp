
#include <spdlog/spdlog.h>

namespace {
	std::string something(){
	SPDLOG_WARN("{}", "123");
	return "123";
	}
}

