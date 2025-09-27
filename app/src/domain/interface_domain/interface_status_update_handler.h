#pragma once

#include <functional>

#include "domain/interface_domain/types/com_user_status.h"

namespace eerie_leap::domain::interface_domain {

using namespace eerie_leap::domain::interface_domain::types;

using InterfaceStatusUpdateHandler = std::function<int(ComUserStatus status, bool success)>;

} // namespace eerie_leap::domain::interface_domain
