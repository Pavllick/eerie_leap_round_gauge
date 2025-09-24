#include "utilities/cbor/cbor_helpers.hpp"
#include "utilities/memory/heap_allocator.h"

#include "gauge_configuration_controller.h"

namespace eerie_leap::controllers::configuration {

using namespace eerie_leap::utilities::cbor;
using namespace eerie_leap::utilities::memory;

LOG_MODULE_REGISTER(gauge_config_ctrl_logger);

GaugeConfigurationController::GaugeConfigurationController(std::shared_ptr<ConfigurationService<GaugeConfig>> gauge_configuration_service) :
    gauge_configuration_service_(std::move(gauge_configuration_service)),
    gauge_config_(nullptr),
    gauge_configuration_(nullptr) {

    if(Get(true) == nullptr)
        LOG_ERR("Failed to load gauge configuration.");
    else
        LOG_INF("Gauge Configuration Controller initialized successfully.");
}

void ValueTypeToPropertyValueType(PropertiesConfig& properties_config, std::unordered_map<std::string, ConfigValue>& properties) {
    properties_config.PropertyValueType_m_count = properties.size();

    size_t i = 0;
    for(auto& [key, value] : properties) {
        PropertiesConfig_PropertyValueType_m property_value;
        property_value.PropertyValueType_m_key = CborHelpers::ToZcborString(&key);

        auto& prop = property_value.PropertyValueType_m;
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, int>) {
                prop.PropertyValueType_choice = PropertyValueType_r::PropertyValueType_int_c;
                prop.value = static_cast<int32_t>(arg);
            } else if constexpr (std::is_same_v<T, double>) {
                prop.PropertyValueType_choice = PropertyValueType_r::PropertyValueType_float_c;
                prop.value = static_cast<double>(arg);
            } else if constexpr (std::is_same_v<T, std::string>) {
                prop.PropertyValueType_choice = PropertyValueType_r::PropertyValueType_tstr_c;
                prop.value = CborHelpers::ToZcborString(&arg);
            }
            else if constexpr (std::is_same_v<T, bool>) {
                prop.PropertyValueType_choice = PropertyValueType_r::PropertyValueType_bool_c;
                prop.value = arg;
            } else if constexpr (std::is_same_v<T, std::vector<int>>) {
                prop.PropertyValueType_choice = PropertyValueType_r::PropertyValueType_int_l_c;
                prop.value = arg;
            } else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
                prop.PropertyValueType_choice = PropertyValueType_r::PropertyValueType_tstr_l_c;

                for (auto it = arg.begin(); it != arg.end(); ++it)
                    std::get<std::vector<zcbor_string>>(prop.value).push_back(CborHelpers::ToZcborString(&*it));
            } else if constexpr (std::is_same_v<T, std::unordered_map<std::string, std::string>>) {
                prop.PropertyValueType_choice = PropertyValueType_r::PropertyValueType_map_c;

                for (auto it = arg.begin(); it != arg.end(); ++it) {
                    std::get<std::vector<map_tstrtstr>>(prop.value).push_back({
                        .tstrtstr_key = CborHelpers::ToZcborString(&it->first),
                        .tstrtstr = CborHelpers::ToZcborString(&it->second)
                    });
                }
            }
        }, value);

        properties_config.PropertyValueType_m.push_back(property_value);

        ++i;
    }
}

bool GaugeConfigurationController::Update(std::shared_ptr<GaugeConfiguration> gauge_configuration) {
    auto gauge_config = make_shared_ext<GaugeConfig>();
    memset(gauge_config.get(), 0, sizeof(GaugeConfig));

    gauge_config->active_screen_index = gauge_configuration->active_screen_index;

    gauge_config->properties_present = gauge_configuration->properties.size() > 0;
    if(gauge_configuration->properties.size() > 0) {
        gauge_config->properties.PropertyValueType_m_count = gauge_configuration->properties.size();
        ValueTypeToPropertyValueType(gauge_config->properties, gauge_configuration->properties);
    }

    gauge_config->ScreenConfig_m_count = gauge_configuration->screen_configurations.size();
    gauge_config->ScreenConfig_m.clear();
    for(int i = 0; i < gauge_configuration->screen_configurations.size(); i++) {
        ScreenConfig screen_config;

        screen_config.id = gauge_configuration->screen_configurations[i].id;
        screen_config.grid.snap_enabled = gauge_configuration->screen_configurations[i].grid.snap_enabled;
        screen_config.grid.width = gauge_configuration->screen_configurations[i].grid.width;
        screen_config.grid.height = gauge_configuration->screen_configurations[i].grid.height;
        screen_config.grid.spacing_px = gauge_configuration->screen_configurations[i].grid.spacing_px;

        screen_config.WidgetConfig_m_count = gauge_configuration->screen_configurations[i].widget_configurations.size();
        screen_config.WidgetConfig_m.clear();
        for(int j = 0; j < gauge_configuration->screen_configurations[i].widget_configurations.size(); j++) {
            WidgetConfig widget_config;

            widget_config.id = gauge_configuration->screen_configurations[i].widget_configurations[j].id;
            widget_config.type = static_cast<uint32_t>(gauge_configuration->screen_configurations[i].widget_configurations[j].type);
            widget_config.position.x = gauge_configuration->screen_configurations[i].widget_configurations[j].position_grid.x;
            widget_config.position.y = gauge_configuration->screen_configurations[i].widget_configurations[j].position_grid.y;
            widget_config.size.width = gauge_configuration->screen_configurations[i].widget_configurations[j].size_grid.width;
            widget_config.size.height = gauge_configuration->screen_configurations[i].widget_configurations[j].size_grid.height;
            widget_config.is_animation_enabled = gauge_configuration->screen_configurations[i].widget_configurations[j].is_animation_enabled;

            widget_config.properties_present = gauge_configuration->screen_configurations[i].widget_configurations[j].properties.size() > 0;
            if(gauge_configuration->screen_configurations[i].widget_configurations[j].properties.size() > 0) {
                widget_config.properties.PropertyValueType_m_count = gauge_configuration->screen_configurations[i].widget_configurations[j].properties.size();
                ValueTypeToPropertyValueType(widget_config.properties, gauge_configuration->screen_configurations[i].widget_configurations[j].properties);
            }

            screen_config.WidgetConfig_m.push_back(widget_config);
        }

        gauge_config->ScreenConfig_m.push_back(screen_config);
    }

    if(!gauge_configuration_service_->Save(gauge_config.get()))
        return false;

    gauge_config_ = gauge_config;
    gauge_configuration_ = gauge_configuration;

    return true;
}

void PropertyValueTypeToValueType(std::unordered_map<std::string, ConfigValue>& properties, PropertiesConfig& properties_config) {
    if(properties_config.PropertyValueType_m_count == 0)
        return;

    for(auto& property : properties_config.PropertyValueType_m) {
        ConfigValue value;
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, int32_t>) {
                value = arg;
            } else if constexpr (std::is_same_v<T, double>) {
                value = static_cast<double>(arg);
            } else if constexpr (std::is_same_v<T, zcbor_string>) {
                value = CborHelpers::ToStdString(arg);
            } else if constexpr (std::is_same_v<T, bool>) {
                value = arg;
            } else if constexpr (std::is_same_v<T, std::vector<int32_t>>) {
                for (const auto& it : arg)
                    std::get<std::vector<int>>(value).push_back(it);
            } else if constexpr (std::is_same_v<T, std::vector<zcbor_string>>) {
                for (const auto& it : arg)
                    std::get<std::vector<std::string>>(value).push_back(CborHelpers::ToStdString(it));
            } else if constexpr (std::is_same_v<T, std::unordered_map<zcbor_string, zcbor_string>>) {
                for (const auto& it : arg) {
                    std::get<std::unordered_map<std::string, std::string>>(value).insert({
                        CborHelpers::ToStdString(it.tstrtstr_key),
                        CborHelpers::ToStdString(it.tstrtstr)
                    });
                }
            } else {
                throw std::runtime_error("Unsupported property value type");
            }
        }, property.PropertyValueType_m.value);

        properties.insert({CborHelpers::ToStdString(property.PropertyValueType_m_key), value});
    }
}

std::shared_ptr<GaugeConfiguration> GaugeConfigurationController::Get(bool force_load) {
    if (gauge_configuration_ != nullptr && !force_load) {
        return gauge_configuration_;
    }

    auto gauge_config = gauge_configuration_service_->Load();
    if(!gauge_config.has_value())
        return nullptr;

    gauge_config_raw_ = gauge_config.value().config_raw;
    gauge_config_ = gauge_config.value().config;

    GaugeConfiguration gauge_configuration;
    gauge_configuration.active_screen_index = gauge_config_->active_screen_index;

    if(gauge_config_->properties_present)
        PropertyValueTypeToValueType(gauge_configuration.properties, gauge_config_->properties);

    for(int i = 0; i < gauge_config_->ScreenConfig_m_count; i++) {
        ScreenConfiguration screen_configuration;
        screen_configuration.id = gauge_config_->ScreenConfig_m[i].id;

        screen_configuration.grid.snap_enabled = gauge_config_->ScreenConfig_m[i].grid.snap_enabled;
        screen_configuration.grid.width = gauge_config_->ScreenConfig_m[i].grid.width;
        screen_configuration.grid.height = gauge_config_->ScreenConfig_m[i].grid.height;
        screen_configuration.grid.spacing_px = gauge_config_->ScreenConfig_m[i].grid.spacing_px;

        for(int j = 0; j < gauge_config_->ScreenConfig_m[i].WidgetConfig_m_count; j++) {
            WidgetConfiguration widget_configuration;
            widget_configuration.type = static_cast<WidgetType>(gauge_config_->ScreenConfig_m[i].WidgetConfig_m[j].type);
            widget_configuration.id = gauge_config_->ScreenConfig_m[i].WidgetConfig_m[j].id;
            widget_configuration.position_grid.x = gauge_config_->ScreenConfig_m[i].WidgetConfig_m[j].position.x;
            widget_configuration.position_grid.y = gauge_config_->ScreenConfig_m[i].WidgetConfig_m[j].position.y;
            widget_configuration.size_grid.width = gauge_config_->ScreenConfig_m[i].WidgetConfig_m[j].size.width;
            widget_configuration.size_grid.height = gauge_config_->ScreenConfig_m[i].WidgetConfig_m[j].size.height;
            widget_configuration.is_animation_enabled = gauge_config_->ScreenConfig_m[i].WidgetConfig_m[j].is_animation_enabled;

            if(gauge_config_->ScreenConfig_m[i].WidgetConfig_m[j].properties_present)
                PropertyValueTypeToValueType(widget_configuration.properties, gauge_config_->ScreenConfig_m[i].WidgetConfig_m[j].properties);

            screen_configuration.widget_configurations.push_back(widget_configuration);
        }

        gauge_configuration.screen_configurations.push_back(screen_configuration);
    }

    gauge_configuration_ = make_shared_ext<GaugeConfiguration>(gauge_configuration);

    return gauge_configuration_;
}

} // namespace eerie_leap::controllers::configuration
