#pragma once

#include "domain/interface_domain/types/user_status.h"
#include "i_interface_processor.h"

namespace eerie_leap::domain::interface_domain::processors {

using namespace eerie_leap::domain::interface_domain::types;

class StatusProcessor : public IInterfaceProcessor<UserStatus> {
private:
    static void SubmitToEventBus(const UserStatus& status);

public:
    StatusProcessor();

    int Process(UserStatus status) override;
};

} // namespace eerie_leap::domain::interface_domain::services
