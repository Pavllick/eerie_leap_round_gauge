#pragma once

#include "domain/system_domain/models/system_configuration.h"

namespace eerie_leap::domain::system_domain::configuration::parsers {

using namespace eerie_leap::domain::system_domain::models;

class SystemConfigurationValidator {
public:
    static void Validate(const SystemConfiguration& configuration);
};

} // namespace eerie_leap::domain::system_domain::configuration::parsers
