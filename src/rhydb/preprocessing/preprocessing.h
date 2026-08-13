#pragma once

#include "rhydb/config/preprocessing_config.h"

#include "rhydb/database.h"

namespace rhydb::preprocessing {

Database preprocessing(const config::PreprocessingConfig& preprocessing_config);

}
