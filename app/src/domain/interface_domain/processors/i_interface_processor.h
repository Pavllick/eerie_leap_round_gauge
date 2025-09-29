#pragma once

namespace eerie_leap::domain::interface_domain::processors {

template<typename T>
class IInterfaceProcessor {
public:
    virtual ~IInterfaceProcessor() = default;

    virtual int Process(T value) = 0;
};

} // namespace eerie_leap::domain::interface_domain::services
