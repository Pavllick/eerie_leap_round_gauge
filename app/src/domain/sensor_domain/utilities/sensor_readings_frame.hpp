#pragma once

#include <unordered_map>
#include <stdexcept>
#include <memory>
#include <string>

#include <zephyr/spinlock.h>

#include "utilities/string/string_helpers.h"
#include "domain/sensor_domain/models/sensor_reading_dto.h"

namespace eerie_leap::domain::sensor_domain::utilities {

using namespace eerie_leap::utilities::string;
using namespace eerie_leap::domain::sensor_domain::models;

class SensorReadingsFrame {
private:
    std::unordered_map<size_t, std::shared_ptr<SensorReadingDto>> readings_;
    mutable std::unordered_map<std::string, size_t> sensor_id_hash_map_;

    k_spinlock reading_lock_;

    size_t GetSensorIdHash(const std::string& sensor_id) const {
        if(!sensor_id_hash_map_.contains(sensor_id))
            sensor_id_hash_map_.emplace(sensor_id, StringHelpers::GetHash(sensor_id));

        return sensor_id_hash_map_.at(sensor_id);
    }

    static inline void ErrorSensorIdNotFound() {
        throw std::runtime_error("Sensor ID not found");
    }

public:
    SensorReadingsFrame() = default;

    void AddOrUpdateReading(std::shared_ptr<SensorReadingDto> reading) {
        auto lock_key = k_spin_lock(&reading_lock_);

        readings_[reading->id_hash] = std::move(reading);

        k_spin_unlock(&reading_lock_, lock_key);
    }

    bool HasReading(const size_t sensor_id_hash) const {
        return readings_.contains(sensor_id_hash);
    }

    bool HasReading(const std::string& sensor_id) const {
        return readings_.contains(GetSensorIdHash(sensor_id));
    }

    std::shared_ptr<SensorReadingDto> GetReading(const size_t sensor_id_hash) const {
        if(!HasReading(sensor_id_hash))
            ErrorSensorIdNotFound();

        return readings_.at(sensor_id_hash);
    }

    std::shared_ptr<SensorReadingDto> GetReading(const std::string& sensor_id) const {
        const size_t sensor_id_hash = GetSensorIdHash(sensor_id);

        if(!HasReading(sensor_id_hash))
            ErrorSensorIdNotFound();

        return readings_.at(sensor_id_hash);
    }

    const std::unordered_map<size_t, std::shared_ptr<SensorReadingDto>>& GetReadings() const {
        return readings_;
    }

    void ClearReadings() {
        auto lock_key = k_spin_lock(&reading_lock_);
        readings_.clear();
        k_spin_unlock(&reading_lock_, lock_key);
    }
};

} // eerie_leap::domain::sensor_domain::utilities
