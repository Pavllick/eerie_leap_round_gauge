#include <string>
#include <vector>

#include <zephyr/logging/log.h>
#include <eerie_memory.hpp>

#include "utilities/memory/memory_resource_manager.h"

#include "configuration/cbor/cbor_sensors_config/cbor_sensors_config.h"
#include "configuration/services/cbor_configuration_service.h"

#include "subsys/random/rng.h"

#include "domain/sensor_domain/models/sensor.h"
#include "domain/sensor_domain/models/sensor_type.h"
#include "domain/sensor_domain/models/sensor_reading.h"
#include "domain/sensor_domain/models/reading_source.h"
#include "domain/sensor_domain/models/reading_status.h"
#include "domain/sensor_domain/models/sources/canbus_source.h"

#include "sensors_controller.h"

namespace eerie_leap::controllers {

using namespace eerie_memory;
using namespace eerie_leap::utilities::memory;
using namespace eerie_leap::subsys::random;
using namespace eerie_leap::domain::sensor_domain::models;
using namespace eerie_leap::domain::sensor_domain::models::sources;

namespace config_services = eerie_leap::configuration::services;

LOG_MODULE_REGISTER(sensors_controller_logger);

SensorsController::SensorsController(
    std::shared_ptr<IFsService> fs_service,
    std::shared_ptr<WorkQueueThread> config_work_queue_thread,
    std::shared_ptr<ConfigurationService> configuration_service,
    std::shared_ptr<ITimeService> time_service,
    std::shared_ptr<GuidGenerator> guid_generator,
    std::shared_ptr<SensorReadingsFrame> sensor_readings_frame,
    std::shared_ptr<CanbusService> canbus_service,
    std::shared_ptr<IGpio> gpio)
    : fs_service_(std::move(fs_service)),
      config_work_queue_thread_(std::move(config_work_queue_thread)),
      configuration_service_(std::move(configuration_service)),
      time_service_(std::move(time_service)),
      guid_generator_(std::move(guid_generator)),
      sensor_readings_frame_(std::move(sensor_readings_frame)),
      canbus_service_(std::move(canbus_service)),
      gpio_(std::move(gpio)) {}

int SensorsController::Initialize() {
    auto cbor_sensors_config_service = std::make_unique<config_services::CborConfigurationService<CborSensorsConfig>>(
        SENSORS_CONFIGURATION_NAME, fs_service_, config_work_queue_thread_);
    sensors_configuration_manager_ = std::make_shared<SensorsConfigurationManager>(
        std::move(cbor_sensors_config_service),
        nullptr,
        gpio_ != nullptr ? gpio_->GetChannelCount() : 0, 0);

    if(configuration_service_ != nullptr)
        configuration_service_->RegisterCborConfigurationManager(
            ConfigurationService::Type::Sensors, sensors_configuration_manager_);

    isr_sensor_reader_factory_ = std::make_shared<IsrSensorReaderFactory>(
        time_service_,
        guid_generator_,
        sensor_readings_frame_,
        canbus_service_,
        gpio_);

    sensors_processing_service_ = std::make_shared<SensorsProcessingService>(
        sensors_configuration_manager_,
        sensor_readings_frame_,
        isr_sensor_reader_factory_,
        nullptr);
    if(!sensors_processing_service_->Initialize()) {
        LOG_ERR("Failed to initialize the sensors processing service.");
        return -1;
    }

    // TODO: For test purposes only
    SetupTestConfiguration();

    return 0;
}

int SensorsController::Start() {
    return sensors_processing_service_->Start() ? 0 : -1;
}

void SensorsController::SetupTestConfiguration() {
    // Test Sensors

    // auto sensor_1 = make_shared_pmr<Sensor>(Mrm::GetExtPmr(), "sensor_1");

    // sensor_1->metadata.name = "Sensor 1";
    // sensor_1->metadata.unit = "";
    // sensor_1->metadata.description = "Test Sensor 1";

    // sensor_1->configuration.type = SensorType::CANBUS_ANALOG;
    // sensor_1->configuration.canbus_source = make_unique_pmr<CanbusSource>(Mrm::GetExtPmr(), 0, 790, "RPM");

    // std::vector<std::shared_ptr<Sensor>> sensors = {
    //     sensor_1
    // };

    std::vector<std::shared_ptr<Sensor>> sensors;

    for(int i = 0; i < 10; i++) {
        auto sensor = make_shared_pmr<Sensor>(Mrm::GetExtPmr(), "sensor_" + std::to_string(i));

        sensor->metadata.name = "Sensor 1";
        sensor->metadata.unit = "";
        sensor->metadata.description = "Test Sensor 1";

        sensor->configuration.type = SensorType::CANBUS_ANALOG;
        sensor->configuration.canbus_source = make_unique_pmr<CanbusSource>(Mrm::GetExtPmr(), 0, 100 + i, "sensor_" + std::to_string(i));

        sensors.push_back(sensor);
    }

    sensors_configuration_manager_->Update(sensors);
}

void SensorsController::EmulateReadings() {
    for(auto sensor : *sensors_configuration_manager_->Get()) {
        SensorReading reading(guid_generator_->Generate(), sensor);
        reading.source = ReadingSource::PROCESSING;
        reading.status = ReadingStatus::PROCESSED;
        reading.value = (Rng::Get<uint32_t>() / static_cast<float>(UINT32_MAX)) * 100.0F;

        sensor_readings_frame_->AddOrUpdateReading(reading);
    }
}

} // namespace eerie_leap::controllers
