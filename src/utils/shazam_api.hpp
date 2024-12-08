#pragma once

#include <string>
#include <userver/formats/json.hpp>

namespace shazam_api {

/// Отправляет аудиофайл в Shazam API и возвращает результат в формате JSON
/// @param audio_path Путь к аудиофайлу
/// @return JSON-ответ от Shazam API
userver::formats::json::Value IdentifyAudio(const std::string& audio_path);

}  // namespace shazam_api
