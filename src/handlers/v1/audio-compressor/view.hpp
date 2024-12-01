#pragma once

#include <string>
#include <string_view>

#include <userver/components/component_list.hpp>

namespace audio_compressor {

void AppendAudioCompressor(userver::components::ComponentList& component_list);

}  // namespace compress
