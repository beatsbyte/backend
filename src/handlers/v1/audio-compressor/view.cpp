#include "view.hpp"

#include <fmt/format.h> 

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/utils/assert.hpp>
#include "utils/inplace_converter.hpp"
#include "utils/lru_cache.hpp"

namespace audio_compressor {
namespace {
  static constexpr std::string_view kAllowedContentType = "audio/mpeg";
}

bool isValidInput(const userver::server::http::FormDataArg& form_data) {
  if (form_data.value.empty() ||
      form_data.filename.has_value() == false ||
      form_data.content_type.value() != kAllowedContentType) {
    return false;
  }

  return true;
}

class Compress final : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "handler-v1-audio-compressor";

  Compress(const userver::components::ComponentConfig& config,
           const userver::components::ComponentContext& component_context)
      : HttpHandlerBase(config, component_context),
        pg_cluster_(
            component_context
                .FindComponent<userver::components::Postgres>("postgres-db-1")
                .GetCluster()),
        cache_(100) // Размер кэша на 100 элементов
  {}

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest& request,
      userver::server::request::RequestContext&) const override {
    const auto& form_data = request.GetFormDataArg("file");
    const auto& compress_degree = request.GetFormDataArg("compress_degree"); // 0, 1, 2

    if (!isValidInput(form_data)) {
      auto& response = request.GetHttpResponse();
      response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
      return {};
    }

    const auto& filename = form_data.filename.value();
    const auto bitrate = converter::getBitrate(filename);
    const auto filename_with_bitrate = std::to_string(bitrate) + std::string(filename) +
                                         std::string(compress_degree.value);
    std::string compressedData;

    // Проверяем, есть ли результат в кэше
    if (cache_.Contains(filename_with_bitrate)) {
      LOG_DEBUG() << "Cache hit for file: " << filename;
      compressedData = cache_.Get(filename_with_bitrate);
    } else {
      LOG_DEBUG() << "Cache miss for file: " << filename;
      compressedData = converter::changeBitrateDirectly(
        std::string{form_data.value},
        atoi(std::string{compress_degree.value}.c_str()));

      cache_.Put(filename_with_bitrate, compressedData);
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

} // namespace audio_compressor