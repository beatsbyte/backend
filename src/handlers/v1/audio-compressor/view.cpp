#include "view.hpp"

#include <fmt/format.h> 
#include <curl/curl.h> 

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/utils/assert.hpp>

#include "utils/inplace_converter.hpp"
#include "utils/lru_cache.hpp"
#include "utils/shazam_api.hpp"

namespace audio_compressor {
namespace {
  static constexpr std::string_view kAllowedContentType = "audio/mpeg";

  bool isValidInput(const userver::server::http::FormDataArg& form_data) {
    if (form_data.value.empty() ||
        !form_data.filename.has_value() ||
        form_data.content_type.value() != kAllowedContentType) {
      return false;
    }
    return true;
  }
}

class Compress final : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "handler-v1-audio-compressor";

  Compress(const userver::components::ComponentConfig& config,
           const userver::components::ComponentContext& component_context)
      : HttpHandlerBase(config, component_context),
        pg_cluster_(component_context
                        .FindComponent<userver::components::Postgres>("postgres-db-1")
                        .GetCluster()),
        cache_(100)  // cache size
  {}

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest& request,
      userver::server::request::RequestContext&) const override {
    const auto& form_data = request.GetFormDataArg("file");
    const auto& compress_degree = request.GetFormDataArg("compress_degree");  // 0, 1, 2

    if (!isValidInput(form_data)) {
      auto& response = request.GetHttpResponse();
      response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
      return {};
    }

    const auto& filename = form_data.filename.value();

    // Сохраняем файл временно для передачи в Shazam API
    const std::string temp_file_path = "/tmp/" + filename;
    std::ofstream temp_file(temp_file_path, std::ios::binary);
    temp_file.write(form_data.value.data(), form_data.value.size());
    temp_file.close();

    // Анализ аудиофайла через Shazam API
    auto shazam_result = shazam_api::IdentifyAudio(temp_file_path);
    auto song_title = shazam_result["track"]["title"].As<std::string>("");
    auto song_artist = shazam_result["track"]["subtitle"].As<std::string>("");

    LOG_INFO() << "Identified audio: " << song_title << " by " << song_artist;

    // Удаляем временный файл
    std::remove(temp_file_path.c_str());

    const auto bitrate = converter::getBitrate(std::string{form_data.value});
    const auto filename_with_metadata = fmt::format("{}_{}_{}kbps_{}",
                                                    song_title,
                                                    song_artist,
                                                    bitrate,
                                                    compress_degree.value);

    std::string compressedData;

    if (cache_.Contains(filename_with_metadata)) {
      LOG_DEBUG() << "Cache hit for file: " << filename_with_metadata << " filename: " << filename;
      compressedData = cache_.Get(filename_with_metadata);
    } else {
      LOG_DEBUG() << "Cache miss for file: " << filename_with_metadata << " filename: " << filename;
      compressedData = converter::changeBitrateDirectly(
          std::string{form_data.value},
          atoi(std::string{compress_degree.value}.c_str()));

      cache_.Put(filename_with_metadata, compressedData);
    }

    auto& response = request.GetHttpResponse();
    response.SetContentType("audio/mpeg");
    response.SetData(compressedData);
    response.SetStatusOk();

    return compressedData;
  }

 private:
  userver::storages::postgres::ClusterPtr pg_cluster_;
  mutable cache::LRUCache<std::string, std::string> cache_;
};

void AppendAudioCompressor(userver::components::ComponentList& component_list) {
  component_list.Append<Compress>();
}

}  // namespace audio_compressor
