#include "utilities/cbor/cbor_helpers.hpp"
#include "utilities/memory/heap_allocator.h"

#include "ui_configuration_manager.h"

namespace eerie_leap::domain::ui_domain::configuration {

using namespace eerie_leap::utilities::cbor;
using namespace eerie_leap::utilities::memory;

LOG_MODULE_REGISTER(ui_config_ctrl_logger);

UiConfigurationManager::UiConfigurationManager(ext_unique_ptr<ConfigurationService<UiConfig>> ui_configuration_service) :
    ui_configuration_service_(std::move(ui_configuration_service)),
    ui_config_(nullptr),
    ui_configuration_(nullptr) {

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
        property_value.PropertyValueType_m_key = CborHelpers::ToZcborString(key);

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
                prop.value = CborHelpers::ToZcborString(arg);
            }
            else if constexpr (std::is_same_v<T, bool>) {
                prop.PropertyValueType_choice = PropertyValueType_r::PropertyValueType_bool_c;
                prop.value = arg;
            } else if constexpr (std::is_same_v<T, std::vector<int>>) {
                prop.PropertyValueType_choice = PropertyValueType_r::PropertyValueType_int_l_c;
                prop.value = arg;
            } else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
                prop.PropertyValueType_choice = PropertyValueType_r::PropertyValueType_tstr_l_c;

                prop.value = std::vector<zcbor_string>();
                for (auto it = arg.begin(); it != arg.end(); ++it)
                    std::get<std::vector<zcbor_string>>(prop.value).push_back(CborHelpers::ToZcborString(*it));
            } else if constexpr (std::is_same_v<T, std::unordered_map<std::string, std::string>>) {
                prop.PropertyValueType_choice = PropertyValueType_r::PropertyValueType_map_c;

                prop.value = std::vector<map_tstrtstr>();
                for (auto it = arg.begin(); it != arg.end(); ++it) {
                    std::get<std::vector<map_tstrtstr>>(prop.value).push_back({
                        .tstrtstr_key = CborHelpers::ToZcborString(it->first),
                        .tstrtstr = CborHelpers::ToZcborString(it->second)
                    });
                }
            } else {
                throw std::runtime_error("Unsupported property value type");
            }
        }, value);

        properties_config.PropertyValueType_m.push_back(property_value);

        ++i;
    }
}

bool UiConfigurationManager::Update(std::shared_ptr<UiConfiguration> ui_configuration) {
    auto ui_config = make_unique_ext<UiConfig>();

    ui_config->active_screen_index = ui_configuration->active_screen_index;

    ui_config->properties_present = ui_configuration->properties.size() > 0;
    if(ui_configuration->properties.size() > 0) {
        ui_config->properties.PropertyValueType_m_count = ui_configuration->properties.size();
        ValueTypeToPropertyValueType(ui_config->properties, ui_configuration->properties);
    }

    ui_config->ScreenConfig_m_count = ui_configuration->screen_configurations.size();
    ui_config->ScreenConfig_m.clear();
    for(int i = 0; i < ui_configuration->screen_configurations.size(); i++) {
        ScreenConfig screen_config;

        screen_config.id = ui_configuration->screen_configurations[i].id;
        screen_config.type = static_cast<uint32_t>(ui_configuration->screen_configurations[i].type);
        screen_config.grid.snap_enabled = ui_configuration->screen_configurations[i].grid.snap_enabled;
        screen_config.grid.width = ui_configuration->screen_configurations[i].grid.width;
        screen_config.grid.height = ui_configuration->screen_configurations[i].grid.height;
        screen_config.grid.spacing_px = ui_configuration->screen_configurations[i].grid.spacing_px;

        screen_config.WidgetConfig_m_count = ui_configuration->screen_configurations[i].widget_configurations.size();
        screen_config.WidgetConfig_m.clear();
        for(int j = 0; j < ui_configuration->screen_configurations[i].widget_configurations.size(); j++) {
            WidgetConfig widget_config;

            widget_config.id = ui_configuration->screen_configurations[i].widget_configurations[j].id;
            widget_config.type = static_cast<uint32_t>(ui_configuration->screen_configurations[i].widget_configurations[j].type);
            widget_config.position.x = ui_configuration->screen_configurations[i].widget_configurations[j].position_grid.x;
            widget_config.position.y = ui_configuration->screen_configurations[i].widget_configurations[j].position_grid.y;
            widget_config.size.width = ui_configuration->screen_configurations[i].widget_configurations[j].size_grid.width;
            widget_config.size.height = ui_configuration->screen_configurations[i].widget_configurations[j].size_grid.height;

            widget_config.properties_present = ui_configuration->screen_configurations[i].widget_configurations[j].properties.size() > 0;
            if(ui_configuration->screen_configurations[i].widget_configurations[j].properties.size() > 0) {
                widget_config.properties.PropertyValueType_m_count = ui_configuration->screen_configurations[i].widget_configurations[j].properties.size();
                ValueTypeToPropertyValueType(widget_config.properties, ui_configuration->screen_configurations[i].widget_configurations[j].properties);
            }

            screen_config.WidgetConfig_m.push_back(widget_config);
        }

        ui_config->ScreenConfig_m.push_back(screen_config);
    }

    if(!ui_configuration_service_->Save(ui_config.get()))
        return false;

    ui_config_ = std::move(ui_config);
    ui_configuration_ = ui_configuration;

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
                value = std::vector<int>();
                for (const auto& it : arg)
                    std::get<std::vector<int>>(value).push_back(it);
            } else if constexpr (std::is_same_v<T, std::vector<zcbor_string>>) {
                value = std::vector<std::string>();
                for (const auto& it : arg)
                    std::get<std::vector<std::string>>(value).push_back(CborHelpers::ToStdString(it));
            } else if constexpr (std::is_same_v<T, std::unordered_map<zcbor_string, zcbor_string>>) {
                value = std::unordered_map<std::string, std::string>();
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

std::shared_ptr<UiConfiguration> UiConfigurationManager::Get(bool force_load) {
    if (ui_configuration_ != nullptr && !force_load) {
        return ui_configuration_;
    }

    auto ui_config = ui_configuration_service_->Load();
    if(!ui_config.has_value())
        return nullptr;

    ui_config_raw_ = std::move(ui_config.value().config_raw);
    ui_config_ = std::move(ui_config.value().config);

    UiConfiguration ui_configuration;
    ui_configuration.active_screen_index = ui_config_->active_screen_index;

    if(ui_config_->properties_present)
        PropertyValueTypeToValueType(ui_configuration.properties, ui_config_->properties);

    for(int i = 0; i < ui_config_->ScreenConfig_m_count; i++) {
        ScreenConfiguration screen_configuration;
        screen_configuration.id = ui_config_->ScreenConfig_m[i].id;
        screen_configuration.type = static_cast<ScreenType>(ui_config_->ScreenConfig_m[i].type);

        screen_configuration.grid.snap_enabled = ui_config_->ScreenConfig_m[i].grid.snap_enabled;
        screen_configuration.grid.width = ui_config_->ScreenConfig_m[i].grid.width;
        screen_configuration.grid.height = ui_config_->ScreenConfig_m[i].grid.height;
        screen_configuration.grid.spacing_px = ui_config_->ScreenConfig_m[i].grid.spacing_px;

        for(int j = 0; j < ui_config_->ScreenConfig_m[i].WidgetConfig_m_count; j++) {
            WidgetConfiguration widget_configuration;
            widget_configuration.type = static_cast<WidgetType>(ui_config_->ScreenConfig_m[i].WidgetConfig_m[j].type);
            widget_configuration.id = ui_config_->ScreenConfig_m[i].WidgetConfig_m[j].id;
            widget_configuration.position_grid.x = ui_config_->ScreenConfig_m[i].WidgetConfig_m[j].position.x;
            widget_configuration.position_grid.y = ui_config_->ScreenConfig_m[i].WidgetConfig_m[j].position.y;
            widget_configuration.size_grid.width = ui_config_->ScreenConfig_m[i].WidgetConfig_m[j].size.width;
            widget_configuration.size_grid.height = ui_config_->ScreenConfig_m[i].WidgetConfig_m[j].size.height;

            if(ui_config_->ScreenConfig_m[i].WidgetConfig_m[j].properties_present)
                PropertyValueTypeToValueType(widget_configuration.properties, ui_config_->ScreenConfig_m[i].WidgetConfig_m[j].properties);

            screen_configuration.widget_configurations.push_back(widget_configuration);
        }

        ui_configuration.screen_configurations.push_back(screen_configuration);
    }

    ui_configuration_ = make_shared_ext<UiConfiguration>(ui_configuration);

    return ui_configuration_;
}

} // namespace eerie_leap::domain::ui_domain::configuration
