#pragma once

#include <memory>
#include <functional>
#include <unordered_map>
#include <vector>

#include "domain/ui_domain/models/widget_type.h"

#include "views/widgets/i_widget.h"
#include "views/widgets/widget_context.h"
#include "views/utilitites/frame.h"

namespace eerie_leap::views::widgets {

using eerie_leap::domain::ui_domain::models::WidgetType;
using eerie_leap::views::utilitites::Frame;
using eerie_leap::views::widgets::IWidget;

class WidgetFactory {
public:
    using WidgetCreator = std::function<std::unique_ptr<IWidget>(const uint32_t id, std::shared_ptr<Frame> container, const WidgetContext& context)>;

private:
    std::unordered_map<WidgetType, WidgetCreator> creators_;

    WidgetFactory();

    void RegisterTypes();

public:
    static WidgetFactory& GetInstance();

    template<typename T>
    void RegisterWidget(const WidgetType type);
    void RegisterWidget(const WidgetType type, WidgetCreator creator);

    std::unique_ptr<IWidget> CreateWidget(const WidgetType type, const uint32_t id, std::shared_ptr<Frame> parent, const WidgetContext& context);
    std::unique_ptr<IWidget> CreateWidget(std::shared_ptr<WidgetConfiguration> configuration, std::shared_ptr<Frame> parent, const WidgetContext& context);

    std::vector<WidgetType> GetAvailableTypes() const;
};

} // namespace eerie_leap::views::widgets
