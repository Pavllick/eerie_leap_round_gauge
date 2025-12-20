#pragma once

namespace eerie_leap::domain::canbus_domain::processors {

template<typename T>
class IReadingProcessor {
public:
    virtual ~IReadingProcessor() = default;

    virtual int Process(const T& value) = 0;
};

} // namespace eerie_leap::domain::canbus_domain::processors
