#pragma once

#include <memory>
#include <functional>
#include <unordered_map>
#include <vector>

#include "domain/ui_domain/models/icon_type.h"

#include "views/widgets/basic/icons/i_icon.h"
#include "views/utilitites/frame.h"

namespace eerie_leap::views::widgets::basic::icons {

using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::utilitites;

class IconFactory {
public:
    using IconCreator = std::function<std::unique_ptr<IIcon>(std::shared_ptr<Frame> container)>;

private:
    std::unordered_map<IconType, IconCreator> creators_;

    IconFactory();

    void RegisterTypes();

public:
    static IconFactory& GetInstance();

    template<typename T>
    void Register(const IconType type);
    void Register(const IconType type, IconCreator creator);

    std::unique_ptr<IIcon> Create(const IconType type, const WidgetConfiguration& config, std::shared_ptr<Frame> parent);

    std::vector<IconType> GetAvailableTypes() const;
};

} // namespace eerie_leap::views::widgets::basic::icons
