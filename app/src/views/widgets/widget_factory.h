#pragma once

#include <memory>
#include <functional>
#include <unordered_map>
#include <vector>

#include "domain/ui_domain/models/widget_type.h"

#include "views/widgets/i_widget.h"
#include "views/utilitites/frame.h"

namespace eerie_leap::views::widgets {

using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::utilitites;
using namespace eerie_leap::views::widgets;

class WidgetFactory {
public:
    using WidgetCreator = std::function<std::unique_ptr<IWidget>(const uint32_t id, std::shared_ptr<Frame> container)>;

private:
    std::unordered_map<WidgetType, WidgetCreator> creators_;

    WidgetFactory();

    void RegisterTypes();

public:
    static WidgetFactory& GetInstance();

    template<typename T>
    void RegisterWidget(const WidgetType type);
    void RegisterWidget(const WidgetType type, WidgetCreator creator);

    std::unique_ptr<IWidget> CreateWidget(const WidgetType type, const uint32_t id, std::shared_ptr<Frame> parent);
    std::unique_ptr<IWidget> CreateWidget(const WidgetConfiguration& config, std::shared_ptr<Frame> parent);

    std::vector<WidgetType> GetAvailableTypes() const;
};

} // namespace eerie_leap::views::widgets
