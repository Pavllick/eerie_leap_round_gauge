#pragma once

namespace eerie_leap::event_bus {

// Constructs both buses and fills the channel registry. Channels are inert until a bus
// registers them, so this has to run before anything publishes.
void InitializeEventChannels();

} // namespace eerie_leap::event_bus
