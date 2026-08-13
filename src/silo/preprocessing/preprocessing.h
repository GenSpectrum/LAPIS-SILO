#pragma once

#include "silo/config/preprocessing_config.h"

#include "silo/database.h"

namespace rhydb::preprocessing {

Database preprocessing(const config::PreprocessingConfig& preprocessing_config);

}
