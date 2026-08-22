#pragma once

#include <memory>

#include "utilities/guid/guid_generator.h"

#include "subsys/fs/services/i_fs_service.h"
#include "subsys/gpio/i_gpio.h"
#include "subsys/threading/work_queue_thread.h"
#include "subsys/time/i_time_service.h"

#include "domain/configuration_domain/services/configuration_service.h"

#include "domain/canbus_domain/services/canbus_service.h"

#include "domain/sensor_domain/configuration/sensors_configuration_manager.h"
#include "domain/sensor_domain/isr_sensor_readers/isr_sensor_reader_factory.h"
#include "domain/sensor_domain/services/sensors_processing_service.h"
#include "domain/sensor_domain/utilities/sensor_readings_frame.hpp"

namespace eerie_leap::controllers {

using eerie_leap::utilities::guid::GuidGenerator;

using eerie_leap::subsys::fs::services::IFsService;
using eerie_leap::subsys::gpio::IGpio;
using eerie_leap::subsys::threading::WorkQueueThread;
using eerie_leap::subsys::time::ITimeService;

using eerie_leap::domain::configuration_domain::services::ConfigurationService;
using eerie_leap::domain::canbus_domain::services::CanbusService;

using eerie_leap::domain::sensor_domain::configuration::SensorsConfigurationManager;
using eerie_leap::domain::sensor_domain::isr_sensor_readers::IsrSensorReaderFactory;
using eerie_leap::domain::sensor_domain::services::SensorsProcessingService;
using eerie_leap::domain::sensor_domain::utilities::SensorReadingsFrame;

class SensorsController {
private:
    static constexpr const char* SENSORS_CONFIGURATION_NAME = "sensors_config";

    std::shared_ptr<IFsService> fs_service_;
    std::shared_ptr<WorkQueueThread> config_work_queue_thread_;
    std::shared_ptr<ConfigurationService> configuration_service_;
    std::shared_ptr<ITimeService> time_service_;
    std::shared_ptr<GuidGenerator> guid_generator_;
    std::shared_ptr<SensorReadingsFrame> sensor_readings_frame_;
    std::shared_ptr<CanbusService> canbus_service_;
    std::shared_ptr<IGpio> gpio_;

    std::shared_ptr<SensorsConfigurationManager> sensors_configuration_manager_;
    std::shared_ptr<IsrSensorReaderFactory> isr_sensor_reader_factory_;
    std::shared_ptr<SensorsProcessingService> sensors_processing_service_;

    // TODO: For test purposes only
    void SetupTestConfiguration();

public:
    SensorsController(
        std::shared_ptr<IFsService> fs_service,
        std::shared_ptr<WorkQueueThread> config_work_queue_thread,
        std::shared_ptr<ConfigurationService> configuration_service,
        std::shared_ptr<ITimeService> time_service,
        std::shared_ptr<GuidGenerator> guid_generator,
        std::shared_ptr<SensorReadingsFrame> sensor_readings_frame,
        std::shared_ptr<CanbusService> canbus_service,
        std::shared_ptr<IGpio> gpio);

    int Initialize();
    int Start();

    std::shared_ptr<SensorsProcessingService> GetProcessingService() const { return sensors_processing_service_; }

    // TODO: For test purposes only
    void EmulateReadings();
};

} // namespace eerie_leap::controllers
