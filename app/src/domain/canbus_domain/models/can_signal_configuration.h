#pragma once

#include <memory_resource>
#include <cstdint>
#include <string>

namespace eerie_leap::domain::canbus_domain::models {

struct CanSignalConfiguration {
    using allocator_type = std::pmr::polymorphic_allocator<>;

    uint32_t start_bit = 0;
    uint32_t size_bits = 0;
    float factor = 1.0F;
    float offset = 0.0F;

    std::pmr::string name;
    std::pmr::string unit;

    CanSignalConfiguration(std::allocator_arg_t, allocator_type alloc)
        : name(alloc), unit(alloc) {}

    CanSignalConfiguration(const CanSignalConfiguration&) = delete;
	CanSignalConfiguration& operator=(const CanSignalConfiguration&) noexcept = default;
	CanSignalConfiguration& operator=(CanSignalConfiguration&&) noexcept = default;
	CanSignalConfiguration(CanSignalConfiguration&&) noexcept = default;
	~CanSignalConfiguration() = default;

    CanSignalConfiguration(CanSignalConfiguration&& other, allocator_type alloc)
        : start_bit(other.start_bit),
        size_bits(other.size_bits),
        factor(other.factor),
        offset(other.offset),
        name(std::move(other.name), alloc),
        unit(std::move(other.unit), alloc) {}
};

} // namespace eerie_leap::domain::canbus_domain::models
