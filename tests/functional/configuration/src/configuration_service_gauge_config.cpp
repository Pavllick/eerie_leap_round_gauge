#include <memory>
#include <vector>
#include <map>
#include <string>
#include <cstring>

#include <zephyr/ztest.h>

#include "utilities/cbor/cbor_helpers.hpp"
#include "configuration/ui_config/ui_config.h"
#include "configuration/services/configuration_service.h"

#include "subsys/device_tree/dt_fs.h"
#include "subsys/fs/services/i_fs_service.h"
#include "subsys/fs/services/fs_service.h"

using namespace eerie_leap::utilities::cbor;
using namespace eerie_leap::configuration::services;
using namespace eerie_leap::subsys::fs::services;
using namespace eerie_leap::subsys::device_tree;

ZTEST_SUITE(configuration_service_ui_config, NULL, NULL, NULL, NULL, NULL);

ZTEST(configuration_service_ui_config, test_UiConfig_PropertyValueType_int_Save_successfully_saved_and_loaded) {
    auto ui_config = make_shared_ext<UiConfig>();
    memset(ui_config.get(), 0, sizeof(UiConfig));

    ui_config->active_screen_index = 12;

    ui_config->properties_present = true;
    ui_config->properties.PropertyValueType_m_count = 4;

    std::vector<std::string> keys = {"t_0", "t_1", "t_2", "t_3"};
    std::vector<int32_t> values = {1, 2, 3, 4};

    for(int i = 0; i < 4; i++) {
        PropertiesConfig_PropertyValueType_m property_value;

        property_value.PropertyValueType_m_key = CborHelpers::ToZcborString(&keys[i]);
        property_value.PropertyValueType_m.PropertyValueType_choice = PropertyValueType_r::PropertyValueType_int_c;
        property_value.PropertyValueType_m.value = values[i];
        ui_config->properties.PropertyValueType_m.push_back(property_value);
    }

    DtFs::InitInternalFs();
    auto fs_service = std::make_shared<FsService>(DtFs::GetInternalFsMp().value());

    fs_service->Format();
    auto ui_config_service = std::make_shared<ConfigurationService<UiConfig>>("ui_config", fs_service);

    auto save_res = ui_config_service->Save(ui_config.get());
    zassert_true(save_res);

    auto loaded_config = ui_config_service->Load();

    zassert_true(loaded_config.has_value());
    zassert_equal(loaded_config.value().config->active_screen_index, 12);
    zassert_equal(loaded_config.value().config->properties_present, true);
    zassert_equal(loaded_config.value().config->properties.PropertyValueType_m_count, 4);

    for(int i = 0; i < 4; i++) {
        zassert_equal(CborHelpers::ToStdString(loaded_config.value().config->properties.PropertyValueType_m[i].PropertyValueType_m_key), keys[i]);
        zassert_equal(loaded_config.value().config->properties.PropertyValueType_m[i].PropertyValueType_m.PropertyValueType_choice, PropertyValueType_r::PropertyValueType_int_c);
        zassert_equal(std::get<int32_t>(loaded_config.value().config->properties.PropertyValueType_m[i].PropertyValueType_m.value), values[i]);
    }
}

ZTEST(configuration_service_ui_config, test_UiConfig_PropertyValueType_double_Save_successfully_saved_and_loaded) {
    auto ui_config = make_shared_ext<UiConfig>();
    memset(ui_config.get(), 0, sizeof(UiConfig));

    ui_config->active_screen_index = 12;

    ui_config->properties_present = true;
    ui_config->properties.PropertyValueType_m_count = 4;

    std::vector<std::string> keys = {"t_0", "t_1", "t_2", "t_3"};
    std::vector<double> values = {1.1, 2.2, 3.3, 4.4};

    for(int i = 0; i < 4; i++) {
        PropertiesConfig_PropertyValueType_m property_value;

        property_value.PropertyValueType_m_key = CborHelpers::ToZcborString(&keys[i]);
        property_value.PropertyValueType_m.PropertyValueType_choice = PropertyValueType_r::PropertyValueType_float_c;
        property_value.PropertyValueType_m.value = values[i];
        ui_config->properties.PropertyValueType_m.push_back(property_value);
    }

    DtFs::InitInternalFs();
    auto fs_service = std::make_shared<FsService>(DtFs::GetInternalFsMp().value());

    fs_service->Format();
    auto ui_config_service = std::make_shared<ConfigurationService<UiConfig>>("ui_config", fs_service);

    auto save_res = ui_config_service->Save(ui_config.get());
    zassert_true(save_res);

    auto loaded_config = ui_config_service->Load();

    zassert_true(loaded_config.has_value());
    zassert_equal(loaded_config.value().config->active_screen_index, 12);
    zassert_equal(loaded_config.value().config->properties_present, true);
    zassert_equal(loaded_config.value().config->properties.PropertyValueType_m_count, 4);

    for(int i = 0; i < 4; i++) {
        zassert_equal(CborHelpers::ToStdString(loaded_config.value().config->properties.PropertyValueType_m[i].PropertyValueType_m_key), keys[i]);
        zassert_equal(loaded_config.value().config->properties.PropertyValueType_m[i].PropertyValueType_m.PropertyValueType_choice, PropertyValueType_r::PropertyValueType_float_c);
        zassert_equal(std::get<double>(loaded_config.value().config->properties.PropertyValueType_m[i].PropertyValueType_m.value), values[i]);
    }
}

ZTEST(configuration_service_ui_config, test_UiConfig_PropertyValueType_string_Save_successfully_saved_and_loaded) {
    auto ui_config = make_shared_ext<UiConfig>();
    memset(ui_config.get(), 0, sizeof(UiConfig));

    ui_config->active_screen_index = 12;

    ui_config->properties_present = true;
    ui_config->properties.PropertyValueType_m_count = 4;

    std::vector<std::string> keys = {"t_0", "t_1", "t_2", "t_3"};
    std::vector<std::string> values = {"v_0", "v_1", "v_2", "v_3"};

    for(int i = 0; i < 4; i++) {
        PropertiesConfig_PropertyValueType_m property_value;

        property_value.PropertyValueType_m_key = CborHelpers::ToZcborString(&keys[i]);
        property_value.PropertyValueType_m.PropertyValueType_choice = PropertyValueType_r::PropertyValueType_tstr_c;
        property_value.PropertyValueType_m.value = CborHelpers::ToZcborString(&values[i]);
        ui_config->properties.PropertyValueType_m.push_back(property_value);
    }

    DtFs::InitInternalFs();
    auto fs_service = std::make_shared<FsService>(DtFs::GetInternalFsMp().value());

    fs_service->Format();
    auto ui_config_service = std::make_shared<ConfigurationService<UiConfig>>("ui_config", fs_service);

    auto save_res = ui_config_service->Save(ui_config.get());
    zassert_true(save_res);

    auto loaded_config = ui_config_service->Load();

    zassert_true(loaded_config.has_value());
    zassert_equal(loaded_config.value().config->active_screen_index, 12);
    zassert_equal(loaded_config.value().config->properties_present, true);
    zassert_equal(loaded_config.value().config->properties.PropertyValueType_m_count, 4);

    for(int i = 0; i < 4; i++) {
        zassert_equal(CborHelpers::ToStdString(loaded_config.value().config->properties.PropertyValueType_m[i].PropertyValueType_m_key), keys[i]);
        zassert_equal(loaded_config.value().config->properties.PropertyValueType_m[i].PropertyValueType_m.PropertyValueType_choice, PropertyValueType_r::PropertyValueType_tstr_c);
        zassert_equal(CborHelpers::ToStdString(
            std::get<zcbor_string>(loaded_config.value().config->properties.PropertyValueType_m[i].PropertyValueType_m.value)
        ), values[i]);
    }
}

ZTEST(configuration_service_ui_config, test_UiConfig_PropertyValueType_bool_Save_successfully_saved_and_loaded) {
    auto ui_config = make_shared_ext<UiConfig>();
    memset(ui_config.get(), 0, sizeof(UiConfig));

    ui_config->active_screen_index = 12;

    ui_config->properties_present = true;
    ui_config->properties.PropertyValueType_m_count = 4;

    std::vector<std::string> keys = {"t_0", "t_1", "t_2", "t_3"};
    std::vector<bool> values = {true, false, true, false};

    for(int i = 0; i < 4; i++) {
        PropertiesConfig_PropertyValueType_m property_value;

        property_value.PropertyValueType_m_key = CborHelpers::ToZcborString(&keys[i]);
        property_value.PropertyValueType_m.PropertyValueType_choice = PropertyValueType_r::PropertyValueType_bool_c;
        property_value.PropertyValueType_m.value = values[i];
        ui_config->properties.PropertyValueType_m.push_back(property_value);
    }

    DtFs::InitInternalFs();
    auto fs_service = std::make_shared<FsService>(DtFs::GetInternalFsMp().value());

    fs_service->Format();
    auto ui_config_service = std::make_shared<ConfigurationService<UiConfig>>("ui_config", fs_service);

    auto save_res = ui_config_service->Save(ui_config.get());
    zassert_true(save_res);

    auto loaded_config = ui_config_service->Load();

    zassert_true(loaded_config.has_value());
    zassert_equal(loaded_config.value().config->active_screen_index, 12);
    zassert_equal(loaded_config.value().config->properties_present, true);
    zassert_equal(loaded_config.value().config->properties.PropertyValueType_m_count, 4);

    for(int i = 0; i < 4; i++) {
        zassert_equal(CborHelpers::ToStdString(loaded_config.value().config->properties.PropertyValueType_m[i].PropertyValueType_m_key), keys[i]);
        zassert_equal(loaded_config.value().config->properties.PropertyValueType_m[i].PropertyValueType_m.PropertyValueType_choice, PropertyValueType_r::PropertyValueType_bool_c);
        zassert_equal(std::get<bool>(loaded_config.value().config->properties.PropertyValueType_m[i].PropertyValueType_m.value), values[i]);
    }
}

ZTEST(configuration_service_ui_config, test_UiConfig_PropertyValueType_int_list_Save_config_successfully_saved_and_loaded) {
    auto ui_config = make_shared_ext<UiConfig>();
    memset(ui_config.get(), 0, sizeof(UiConfig));

    ui_config->active_screen_index = 12;

    ui_config->properties_present = true;
    ui_config->properties.PropertyValueType_m_count = 4;

    std::vector<std::string> keys = {"t_0", "t_1", "t_2", "t_3"};
    std::vector<std::vector<int32_t>> values = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {10, 11, 12}};

    for(int i = 0; i < 4; i++) {
        PropertiesConfig_PropertyValueType_m property_value;

        property_value.PropertyValueType_m_key = CborHelpers::ToZcborString(&keys[i]);
        property_value.PropertyValueType_m.PropertyValueType_choice = PropertyValueType_r::PropertyValueType_int_l_c;
        property_value.PropertyValueType_m.value = values[i];
        ui_config->properties.PropertyValueType_m.push_back(property_value);
    }

    DtFs::InitInternalFs();
    auto fs_service = std::make_shared<FsService>(DtFs::GetInternalFsMp().value());

    fs_service->Format();
    auto ui_config_service = std::make_shared<ConfigurationService<UiConfig>>("ui_config", fs_service);

    auto save_res = ui_config_service->Save(ui_config.get());
    zassert_true(save_res);

    auto loaded_config = ui_config_service->Load();

    zassert_true(loaded_config.has_value());
    zassert_equal(loaded_config.value().config->active_screen_index, 12);
    zassert_equal(loaded_config.value().config->properties_present, true);
    zassert_equal(loaded_config.value().config->properties.PropertyValueType_m_count, 4);

    for(int i = 0; i < 4; i++) {
        zassert_equal(CborHelpers::ToStdString(loaded_config.value().config->properties.PropertyValueType_m[i].PropertyValueType_m_key), keys[i]);
        zassert_equal(loaded_config.value().config->properties.PropertyValueType_m[i].PropertyValueType_m.PropertyValueType_choice, PropertyValueType_r::PropertyValueType_int_l_c);
        zassert_equal(std::get<std::vector<int32_t>>(loaded_config.value().config->properties.PropertyValueType_m[i].PropertyValueType_m.value), values[i]);
    }
}

ZTEST(configuration_service_ui_config, test_UiConfig_PropertyValueType_string_list_Save_config_successfully_saved_and_loaded) {
    auto ui_config = make_shared_ext<UiConfig>();
    memset(ui_config.get(), 0, sizeof(UiConfig));

    ui_config->active_screen_index = 12;

    ui_config->properties_present = true;
    ui_config->properties.PropertyValueType_m_count = 4;

    std::vector<std::string> keys = {"t_0", "t_1", "t_2", "t_3"};
    std::vector<std::vector<std::string>> values = {{"v_0", "v_1", "v_2"}, {"v_3", "v_4", "v_5"}, {"v_6", "v_7", "v_8"}, {"v_9", "v_10", "v_11"}};

    std::vector<std::vector<zcbor_string>> values_zcbor;
    values_zcbor.reserve(4);
    for(int i = 0; i < 4; i++) {
        std::vector<zcbor_string> value_zcbor;
        value_zcbor.reserve(3);
        for(int j = 0; j < 3; j++) {
            value_zcbor.push_back(CborHelpers::ToZcborString(&values[i][j]));
        }
        values_zcbor.push_back(value_zcbor);
    }

    for(int i = 0; i < 4; i++) {
        PropertiesConfig_PropertyValueType_m property_value;

        property_value.PropertyValueType_m_key = CborHelpers::ToZcborString(&keys[i]);
        property_value.PropertyValueType_m.PropertyValueType_choice = PropertyValueType_r::PropertyValueType_tstr_l_c;
        property_value.PropertyValueType_m.value = values_zcbor[i];
        ui_config->properties.PropertyValueType_m.push_back(property_value);
    }

    DtFs::InitInternalFs();
    auto fs_service = std::make_shared<FsService>(DtFs::GetInternalFsMp().value());

    fs_service->Format();
    auto ui_config_service = std::make_shared<ConfigurationService<UiConfig>>("ui_config", fs_service);

    auto save_res = ui_config_service->Save(ui_config.get());
    zassert_true(save_res);

    auto loaded_config = ui_config_service->Load();

    zassert_true(loaded_config.has_value());
    zassert_equal(loaded_config.value().config->active_screen_index, 12);
    zassert_equal(loaded_config.value().config->properties_present, true);
    zassert_equal(loaded_config.value().config->properties.PropertyValueType_m_count, 4);

    for(int i = 0; i < 4; i++) {
        zassert_equal(CborHelpers::ToStdString(loaded_config.value().config->properties.PropertyValueType_m[i].PropertyValueType_m_key), keys[i]);
        zassert_equal(loaded_config.value().config->properties.PropertyValueType_m[i].PropertyValueType_m.PropertyValueType_choice, PropertyValueType_r::PropertyValueType_tstr_l_c);

        for(int j = 0; j < 3; j++) {
            zassert_equal(
                CborHelpers::ToStdString(std::get<std::vector<zcbor_string>>(loaded_config.value().config->properties.PropertyValueType_m[i].PropertyValueType_m.value)[j]),
                values[i][j]);
        }
    }
}

ZTEST(configuration_service_ui_config, test_UiConfig_PropertyValueType_string_map_Save_config_successfully_saved_and_loaded) {
    auto ui_config = make_shared_ext<UiConfig>();
    memset(ui_config.get(), 0, sizeof(UiConfig));

    ui_config->active_screen_index = 12;

    ui_config->properties_present = true;
    ui_config->properties.PropertyValueType_m_count = 4;

    std::vector<std::string> keys = {"t_0", "t_1", "t_2", "t_3"};

    std::vector<std::map<std::string, std::string>> values = {
        {{"k_0", "v_0"}, {"k_1", "v_1"}, {"k_2", "v_2"}},
        {{"k_3", "v_3"}, {"k_4", "v_4"}, {"k_5", "v_5"}},
        {{"k_6", "v_6"}, {"k_7", "v_7"}, {"k_8", "v_8"}},
        {{"k_9", "v_9"}, {"k_10", "v_10"}, {"k_11", "v_11"}}
    };

    std::vector<std::vector<map_tstrtstr>> values_zcbor;
    values_zcbor.reserve(4);
    for(int i = 0; i < 4; i++) {
        std::vector<map_tstrtstr> value_zcbor;
        value_zcbor.reserve(3);

        for(auto &item : values[i])
            value_zcbor.push_back({
                .tstrtstr_key = CborHelpers::ToZcborString(&item.first),
                .tstrtstr = CborHelpers::ToZcborString(&item.second)
            });

        values_zcbor.push_back(value_zcbor);
    }

    for(int i = 0; i < 4; i++) {
        PropertiesConfig_PropertyValueType_m property_value;

        property_value.PropertyValueType_m_key = CborHelpers::ToZcborString(&keys[i]);
        property_value.PropertyValueType_m.PropertyValueType_choice = PropertyValueType_r::PropertyValueType_map_c;
        property_value.PropertyValueType_m.value = values_zcbor[i];
        ui_config->properties.PropertyValueType_m.push_back(property_value);
    }

    DtFs::InitInternalFs();
    auto fs_service = std::make_shared<FsService>(DtFs::GetInternalFsMp().value());

    fs_service->Format();
    auto ui_config_service = std::make_shared<ConfigurationService<UiConfig>>("ui_config", fs_service);

    auto save_res = ui_config_service->Save(ui_config.get());
    zassert_true(save_res);

    auto loaded_config = ui_config_service->Load();

    zassert_true(loaded_config.has_value());
    zassert_equal(loaded_config.value().config->active_screen_index, 12);
    zassert_equal(loaded_config.value().config->properties_present, true);
    zassert_equal(loaded_config.value().config->properties.PropertyValueType_m_count, 4);

    for(int i = 0; i < 4; i++) {
        zassert_equal(CborHelpers::ToStdString(loaded_config.value().config->properties.PropertyValueType_m[i].PropertyValueType_m_key), keys[i]);
        zassert_equal(loaded_config.value().config->properties.PropertyValueType_m[i].PropertyValueType_m.PropertyValueType_choice, PropertyValueType_r::PropertyValueType_map_c);

        for(int j = 0; j < 3; j++) {
            auto key = CborHelpers::ToStdString(
                std::get<std::vector<map_tstrtstr>>(
                    loaded_config.value().config->properties.PropertyValueType_m[i].PropertyValueType_m.value)[j].tstrtstr_key);

            auto value = CborHelpers::ToStdString(
                std::get<std::vector<map_tstrtstr>>(
                    loaded_config.value().config->properties.PropertyValueType_m[i].PropertyValueType_m.value)[j].tstrtstr);

            zassert_true(values[i].contains(key));
            zassert_equal(value, values[i][key]);
        }
    }
}

ZTEST(configuration_service_ui_config, test_UiConfig_PropertyValueType_string_all_mixed_Save_config_successfully_saved_and_loaded) {
    auto ui_config = make_shared_ext<UiConfig>();
    memset(ui_config.get(), 0, sizeof(UiConfig));

    ui_config->active_screen_index = 12;

    ui_config->properties_present = true;
    ui_config->properties.PropertyValueType_m_count = 7;

    std::vector<std::string> keys = {"t_0", "t_1", "t_2", "t_3", "t_4", "t_5", "t_6"};
    int32_t uint32_t_value = 46;
    double double_value = 1.1;
    std::string string_value = "string";
    bool bool_value = true;

    std::vector<int32_t> int_vector_value = {1, 2, 3, 4, 5, 6, 7};

    std::vector<std::string> string_vector_value = {
        "string_0", "string_1", "string_2", "string_3", "string_4", "string_5", "string_6"
    };

    std::map<std::string, std::string> string_map_value = {
        {{"k_0", "v_0"}, {"k_1", "v_1"}, {"k_2", "v_2"}}
    };

    // int32_t
    ui_config->properties.PropertyValueType_m.push_back({
        .PropertyValueType_m_key = CborHelpers::ToZcborString(&keys[0]),
        .PropertyValueType_m = {
            .value = uint32_t_value,
            .PropertyValueType_choice = PropertyValueType_r::PropertyValueType_int_c
        }
    });

    // double
    ui_config->properties.PropertyValueType_m.push_back({
        .PropertyValueType_m_key = CborHelpers::ToZcborString(&keys[1]),
        .PropertyValueType_m = {
            .value = double_value,
            .PropertyValueType_choice = PropertyValueType_r::PropertyValueType_float_c
        }
    });

    // string
    ui_config->properties.PropertyValueType_m.push_back({
        .PropertyValueType_m_key = CborHelpers::ToZcborString(&keys[2]),
        .PropertyValueType_m = {
            .value = CborHelpers::ToZcborString(&string_value),
            .PropertyValueType_choice = PropertyValueType_r::PropertyValueType_tstr_c
        }
    });

        ui_config->properties.PropertyValueType_m.push_back({
        .PropertyValueType_m_key = CborHelpers::ToZcborString(&keys[3]),
        .PropertyValueType_m = {
            .value = bool_value,
            .PropertyValueType_choice = PropertyValueType_r::PropertyValueType_bool_c
        }
    });

    // int vector
    ui_config->properties.PropertyValueType_m.push_back({
        .PropertyValueType_m_key = CborHelpers::ToZcborString(&keys[4]),
        .PropertyValueType_m = {
            .value = int_vector_value,
            .PropertyValueType_choice = PropertyValueType_r::PropertyValueType_int_l_c
        }
    });

        std::vector<zcbor_string> string_vector_value_zcbor;
    string_vector_value_zcbor.reserve(string_vector_value.size());
    for(int i = 0; i < string_vector_value.size(); i++)
        string_vector_value_zcbor.push_back(CborHelpers::ToZcborString(&string_vector_value[i]));

    ui_config->properties.PropertyValueType_m.push_back({
        .PropertyValueType_m_key = CborHelpers::ToZcborString(&keys[5]),
        .PropertyValueType_m = {
            .value = string_vector_value_zcbor,
            .PropertyValueType_choice = PropertyValueType_r::PropertyValueType_tstr_l_c
        }
    });

    // string map
    std::vector<map_tstrtstr> string_map_value_zcbor;
    string_map_value_zcbor.reserve(string_map_value.size());
    for(auto &item : string_map_value) {
        string_map_value_zcbor.push_back({
            .tstrtstr_key = CborHelpers::ToZcborString(&item.first),
            .tstrtstr = CborHelpers::ToZcborString(&item.second)
        });
    }

    ui_config->properties.PropertyValueType_m.push_back({
        .PropertyValueType_m_key = CborHelpers::ToZcborString(&keys[6]),
        .PropertyValueType_m = {
            .value = string_map_value_zcbor,
            .PropertyValueType_choice = PropertyValueType_r::PropertyValueType_map_c
        }
    });

    DtFs::InitInternalFs();
    auto fs_service = std::make_shared<FsService>(DtFs::GetInternalFsMp().value());

    fs_service->Format();
    auto ui_config_service = std::make_shared<ConfigurationService<UiConfig>>("ui_config", fs_service);

    auto save_res = ui_config_service->Save(ui_config.get());
    zassert_true(save_res);

    auto loaded_config = ui_config_service->Load();

    zassert_true(loaded_config.has_value());
    zassert_equal(loaded_config.value().config->active_screen_index, 12);
    zassert_equal(loaded_config.value().config->properties_present, true);
    zassert_equal(loaded_config.value().config->properties.PropertyValueType_m_count, 7);

    // int32_t
    zassert_equal(CborHelpers::ToStdString(loaded_config.value().config->properties.PropertyValueType_m[0].PropertyValueType_m_key), keys[0]);
    zassert_equal(loaded_config.value().config->properties.PropertyValueType_m[0].PropertyValueType_m.PropertyValueType_choice, PropertyValueType_r::PropertyValueType_int_c);
    zassert_equal(std::get<int32_t>(loaded_config.value().config->properties.PropertyValueType_m[0].PropertyValueType_m.value), uint32_t_value);

    // double
    zassert_equal(CborHelpers::ToStdString(loaded_config.value().config->properties.PropertyValueType_m[1].PropertyValueType_m_key), keys[1]);
    zassert_equal(loaded_config.value().config->properties.PropertyValueType_m[1].PropertyValueType_m.PropertyValueType_choice, PropertyValueType_r::PropertyValueType_float_c);
    zassert_equal(std::get<double>(loaded_config.value().config->properties.PropertyValueType_m[1].PropertyValueType_m.value), double_value);

    // string
    zassert_equal(CborHelpers::ToStdString(loaded_config.value().config->properties.PropertyValueType_m[2].PropertyValueType_m_key), keys[2]);
    zassert_equal(loaded_config.value().config->properties.PropertyValueType_m[2].PropertyValueType_m.PropertyValueType_choice, PropertyValueType_r::PropertyValueType_tstr_c);
    zassert_equal(CborHelpers::ToStdString(std::get<zcbor_string>(loaded_config.value().config->properties.PropertyValueType_m[2].PropertyValueType_m.value)), string_value);

    // bool
    zassert_equal(CborHelpers::ToStdString(loaded_config.value().config->properties.PropertyValueType_m[3].PropertyValueType_m_key), keys[3]);
    zassert_equal(loaded_config.value().config->properties.PropertyValueType_m[3].PropertyValueType_m.PropertyValueType_choice, PropertyValueType_r::PropertyValueType_bool_c);
    zassert_equal(std::get<bool>(loaded_config.value().config->properties.PropertyValueType_m[3].PropertyValueType_m.value), bool_value);

    // int vector
    zassert_equal(CborHelpers::ToStdString(loaded_config.value().config->properties.PropertyValueType_m[4].PropertyValueType_m_key), keys[4]);
    zassert_equal(loaded_config.value().config->properties.PropertyValueType_m[4].PropertyValueType_m.PropertyValueType_choice, PropertyValueType_r::PropertyValueType_int_l_c);
    zassert_equal(std::get<std::vector<int32_t>>(loaded_config.value().config->properties.PropertyValueType_m[4].PropertyValueType_m.value), int_vector_value);

    // string vector
    zassert_equal(CborHelpers::ToStdString(loaded_config.value().config->properties.PropertyValueType_m[5].PropertyValueType_m_key), keys[5]);
    zassert_equal(loaded_config.value().config->properties.PropertyValueType_m[5].PropertyValueType_m.PropertyValueType_choice, PropertyValueType_r::PropertyValueType_tstr_l_c);
    for(int i = 0; i < string_vector_value.size(); i++)
        zassert_equal(
            CborHelpers::ToStdString(
                std::get<std::vector<zcbor_string>>(loaded_config.value().config->properties.PropertyValueType_m[5].PropertyValueType_m.value)[i]),
            string_vector_value[i]);

    // string map
    zassert_equal(CborHelpers::ToStdString(loaded_config.value().config->properties.PropertyValueType_m[6].PropertyValueType_m_key), keys[6]);
    zassert_equal(loaded_config.value().config->properties.PropertyValueType_m[6].PropertyValueType_m.PropertyValueType_choice, PropertyValueType_r::PropertyValueType_map_c);
    for(int j = 0; j < 3; j++) {
        auto key = CborHelpers::ToStdString(
            std::get<std::vector<map_tstrtstr>>(
                loaded_config.value().config->properties.PropertyValueType_m[6].PropertyValueType_m.value)[j].tstrtstr_key);

        auto value = CborHelpers::ToStdString(
            std::get<std::vector<map_tstrtstr>>(
                loaded_config.value().config->properties.PropertyValueType_m[6].PropertyValueType_m.value)[j].tstrtstr);

        zassert_true(string_map_value.contains(key));
        zassert_equal(value, string_map_value[key]);
    }
}

ZTEST(configuration_service_ui_config, test_UiConfig_Save_successfully_saved_and_loaded) {
    auto ui_config = make_shared_ext<UiConfig>();
    memset(ui_config.get(), 0, sizeof(UiConfig));

    ui_config->active_screen_index = 12;

    ui_config->properties_present = true;
    ui_config->properties.PropertyValueType_m_count = 4;

    std::vector<std::string> keys = {"t_0", "t_1", "t_2", "t_3"};
    std::vector<int32_t> values = {1, 2, 3, 4};

    for(int i = 0; i < 4; i++) {
        PropertiesConfig_PropertyValueType_m property_value;

        property_value.PropertyValueType_m_key = CborHelpers::ToZcborString(&keys[i]);
        property_value.PropertyValueType_m.PropertyValueType_choice = PropertyValueType_r::PropertyValueType_int_c;
        property_value.PropertyValueType_m.value = values[i];
        ui_config->properties.PropertyValueType_m.push_back(property_value);
    }

    ui_config->ScreenConfig_m_count = 1;
    ui_config->ScreenConfig_m.push_back({
        .id = 1,
        .type = 2,
        .grid = {
            .snap_enabled = true,
            .width = 100,
            .height = 100,
            .spacing_px = 10
        },
        .WidgetConfig_m = {{
            .type = 1,
            .id = 1,
            .position = {
                .x = 0,
                .y = 0
            },
            .size = {
                .width = 100,
                .height = 100
            },
            .properties = {
                .PropertyValueType_m = {{
                    .PropertyValueType_m_key = CborHelpers::ToZcborString(&keys[0]),
                    .PropertyValueType_m = {
                        .value = values[0],
                        .PropertyValueType_choice = PropertyValueType_r::PropertyValueType_int_c
                    }
                }},
                .PropertyValueType_m_count = 1
            },
            .properties_present = true,
        },
        {
            .type = 1,
            .id = 2,
            .position = {
                .x = 0,
                .y = 0
            },
            .size = {
                .width = 100,
                .height = 100
            },
            .properties = {
                .PropertyValueType_m = {{
                    .PropertyValueType_m_key = CborHelpers::ToZcborString(&keys[0]),
                    .PropertyValueType_m = {
                        .value = values[0],
                        .PropertyValueType_choice = PropertyValueType_r::PropertyValueType_int_c
                    }
                },
                {
                    .PropertyValueType_m_key = CborHelpers::ToZcborString(&keys[1]),
                    .PropertyValueType_m = {
                        .value = values[1],
                        .PropertyValueType_choice = PropertyValueType_r::PropertyValueType_int_c
                    }
                },
                {
                    .PropertyValueType_m_key = CborHelpers::ToZcborString(&keys[2]),
                    .PropertyValueType_m = {
                        .value = values[2],
                        .PropertyValueType_choice = PropertyValueType_r::PropertyValueType_int_c
                    }
                },
                {
                    .PropertyValueType_m_key = CborHelpers::ToZcborString(&keys[3]),
                    .PropertyValueType_m = {
                        .value = values[3],
                        .PropertyValueType_choice = PropertyValueType_r::PropertyValueType_int_c
                    }
                }},
                .PropertyValueType_m_count = 4
            },
            .properties_present = true,
        }},
        .WidgetConfig_m_count = 2
    });

    DtFs::InitInternalFs();
    auto fs_service = std::make_shared<FsService>(DtFs::GetInternalFsMp().value());

    fs_service->Format();
    auto ui_config_service = std::make_shared<ConfigurationService<UiConfig>>("ui_config", fs_service);

    auto save_res = ui_config_service->Save(ui_config.get());
    zassert_true(save_res);

    auto loaded_config = ui_config_service->Load();

    zassert_true(loaded_config.has_value());
    zassert_equal(loaded_config.value().config->active_screen_index, 12);
    zassert_equal(loaded_config.value().config->properties_present, true);
    zassert_equal(loaded_config.value().config->properties.PropertyValueType_m_count, 4);

    for(int i = 0; i < 4; i++) {
        zassert_equal(CborHelpers::ToStdString(loaded_config.value().config->properties.PropertyValueType_m[i].PropertyValueType_m_key), keys[i]);
        zassert_equal(loaded_config.value().config->properties.PropertyValueType_m[i].PropertyValueType_m.PropertyValueType_choice, PropertyValueType_r::PropertyValueType_int_c);
        zassert_equal(std::get<int32_t>(loaded_config.value().config->properties.PropertyValueType_m[i].PropertyValueType_m.value), values[i]);
    }

    zassert_equal(loaded_config.value().config->ScreenConfig_m_count, 1);
    zassert_equal(loaded_config.value().config->ScreenConfig_m[0].id, 1);
    zassert_equal(loaded_config.value().config->ScreenConfig_m[0].type, 2);
    zassert_equal(loaded_config.value().config->ScreenConfig_m[0].grid.snap_enabled, true);
    zassert_equal(loaded_config.value().config->ScreenConfig_m[0].grid.width, 100);
    zassert_equal(loaded_config.value().config->ScreenConfig_m[0].grid.height, 100);
    zassert_equal(loaded_config.value().config->ScreenConfig_m[0].grid.spacing_px, 10);

    zassert_equal(loaded_config.value().config->ScreenConfig_m[0].WidgetConfig_m_count, 2);
    zassert_equal(loaded_config.value().config->ScreenConfig_m[0].WidgetConfig_m[0].type, 1);
    zassert_equal(loaded_config.value().config->ScreenConfig_m[0].WidgetConfig_m[0].id, 1);
    zassert_equal(loaded_config.value().config->ScreenConfig_m[0].WidgetConfig_m[0].position.x, 0);
    zassert_equal(loaded_config.value().config->ScreenConfig_m[0].WidgetConfig_m[0].position.y, 0);
    zassert_equal(loaded_config.value().config->ScreenConfig_m[0].WidgetConfig_m[0].size.width, 100);
    zassert_equal(loaded_config.value().config->ScreenConfig_m[0].WidgetConfig_m[0].size.height, 100);
    zassert_equal(loaded_config.value().config->ScreenConfig_m[0].WidgetConfig_m[0].properties_present, true);
    zassert_equal(loaded_config.value().config->ScreenConfig_m[0].WidgetConfig_m[0].properties.PropertyValueType_m_count, 1);
    zassert_equal(CborHelpers::ToStdString(loaded_config.value().config->ScreenConfig_m[0].WidgetConfig_m[0].properties.PropertyValueType_m[0].PropertyValueType_m_key), keys[0]);
    zassert_equal(loaded_config.value().config->ScreenConfig_m[0].WidgetConfig_m[0].properties.PropertyValueType_m[0].PropertyValueType_m.PropertyValueType_choice, PropertyValueType_r::PropertyValueType_int_c);
    zassert_equal(std::get<int32_t>(loaded_config.value().config->ScreenConfig_m[0].WidgetConfig_m[0].properties.PropertyValueType_m[0].PropertyValueType_m.value), values[0]);

    zassert_equal(loaded_config.value().config->ScreenConfig_m[0].WidgetConfig_m[1].type, 1);
    zassert_equal(loaded_config.value().config->ScreenConfig_m[0].WidgetConfig_m[1].id, 2);
    zassert_equal(loaded_config.value().config->ScreenConfig_m[0].WidgetConfig_m[1].position.x, 0);
    zassert_equal(loaded_config.value().config->ScreenConfig_m[0].WidgetConfig_m[1].position.y, 0);
    zassert_equal(loaded_config.value().config->ScreenConfig_m[0].WidgetConfig_m[1].size.width, 100);
    zassert_equal(loaded_config.value().config->ScreenConfig_m[0].WidgetConfig_m[1].size.height, 100);
    zassert_equal(loaded_config.value().config->ScreenConfig_m[0].WidgetConfig_m[1].properties_present, true);
    zassert_equal(loaded_config.value().config->ScreenConfig_m[0].WidgetConfig_m[1].properties.PropertyValueType_m_count, 4);
    zassert_equal(CborHelpers::ToStdString(loaded_config.value().config->ScreenConfig_m[0].WidgetConfig_m[1].properties.PropertyValueType_m[0].PropertyValueType_m_key), keys[0]);
    zassert_equal(loaded_config.value().config->ScreenConfig_m[0].WidgetConfig_m[1].properties.PropertyValueType_m[0].PropertyValueType_m.PropertyValueType_choice, PropertyValueType_r::PropertyValueType_int_c);
    zassert_equal(std::get<int32_t>(loaded_config.value().config->ScreenConfig_m[0].WidgetConfig_m[1].properties.PropertyValueType_m[0].PropertyValueType_m.value), values[0]);
    zassert_equal(CborHelpers::ToStdString(loaded_config.value().config->ScreenConfig_m[0].WidgetConfig_m[1].properties.PropertyValueType_m[1].PropertyValueType_m_key), keys[1]);
    zassert_equal(loaded_config.value().config->ScreenConfig_m[0].WidgetConfig_m[1].properties.PropertyValueType_m[1].PropertyValueType_m.PropertyValueType_choice, PropertyValueType_r::PropertyValueType_int_c);
    zassert_equal(std::get<int32_t>(loaded_config.value().config->ScreenConfig_m[0].WidgetConfig_m[1].properties.PropertyValueType_m[1].PropertyValueType_m.value), values[1]);
    zassert_equal(CborHelpers::ToStdString(loaded_config.value().config->ScreenConfig_m[0].WidgetConfig_m[1].properties.PropertyValueType_m[2].PropertyValueType_m_key), keys[2]);
    zassert_equal(loaded_config.value().config->ScreenConfig_m[0].WidgetConfig_m[1].properties.PropertyValueType_m[2].PropertyValueType_m.PropertyValueType_choice, PropertyValueType_r::PropertyValueType_int_c);
    zassert_equal(std::get<int32_t>(loaded_config.value().config->ScreenConfig_m[0].WidgetConfig_m[1].properties.PropertyValueType_m[2].PropertyValueType_m.value), values[2]);
    zassert_equal(CborHelpers::ToStdString(loaded_config.value().config->ScreenConfig_m[0].WidgetConfig_m[1].properties.PropertyValueType_m[3].PropertyValueType_m_key), keys[3]);
    zassert_equal(loaded_config.value().config->ScreenConfig_m[0].WidgetConfig_m[1].properties.PropertyValueType_m[3].PropertyValueType_m.PropertyValueType_choice, PropertyValueType_r::PropertyValueType_int_c);
    zassert_equal(std::get<int32_t>(loaded_config.value().config->ScreenConfig_m[0].WidgetConfig_m[1].properties.PropertyValueType_m[3].PropertyValueType_m.value), values[3]);
}
