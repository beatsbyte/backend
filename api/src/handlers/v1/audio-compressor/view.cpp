#include "view.hpp"

#include <fmt/format.h>

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/utils/assert.hpp>

namespace audio_compressor {

bool isValidInput(const userver::server::http::FormDataArg& form_data) {
  static constexpr std::string_view kAllowedContentType = "audio/mpeg";

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
                .GetCluster()) {}

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest& request,
      userver::server::request::RequestContext&) const override {
    auto form_data = request.GetFormDataArg("file");

    if (!isValidInput(form_data)) {
      auto& response = request.GetHttpResponse();
      response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
      return {};
    }

    LOG_DEBUG() << form_data.filename.value() << " form_data_audio filename\n";
    LOG_DEBUG() << form_data.content_type.value() << " form_data_audio content_type\n";
    

    userver::formats::json::ValueBuilder response;
    response["filename"] = fmt::format("{}", form_data.filename.value());

    return userver::formats::json::ToString(response.ExtractValue());
  }

  userver::storages::postgres::ClusterPtr pg_cluster_;
};


void AppendAudioCompressor(userver::components::ComponentList& component_list) {
  component_list.Append<Compress>();
}

} // namespace audio_compressor