#pragma once

#include <memory>
#include <functional>
#include <unordered_map>
#include <vector>

#include "views/widgets/i_widget.h"
#include "views/widgets/configuration/widget_type.h"

namespace eerie_leap::views::widgets {

using namespace eerie_leap::views::widgets;
using namespace eerie_leap::views::widgets::configuration;

class WidgetFactory {
public:
    using WidgetCreator = std::function<std::unique_ptr<IWidget>(const uint32_t id)>;

private:
    std::unordered_map<WidgetType, WidgetCreator> creators_;
    static std::shared_ptr<WidgetFactory> instance_;

    WidgetFactory();

    void RegisterBuiltinWidgets();

public:
    static std::shared_ptr<WidgetFactory> GetInstance();

    void RegisterWidget(const WidgetType type, WidgetCreator creator);

    template<typename T>
    void RegisterWidget(const WidgetType type);

    std::unique_ptr<IWidget> CreateWidget(const WidgetType type, const uint32_t id);
    std::unique_ptr<IWidget> CreateWidget(const WidgetConfiguration& config);

    std::vector<WidgetType> GetAvailableTypes() const;
    bool IsTypeRegistered(const WidgetType type) const;
};

} // namespace eerie_leap::views::widgets
