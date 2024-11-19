#include "config/toplevel_interface.h"

void ToplevelConfig::overwriteFrom(const VerifiedConfigSource& config_source) {
   overwriteFromParents(ConsList<std::string>(), config_source);
}
