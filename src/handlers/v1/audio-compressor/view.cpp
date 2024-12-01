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
namespace {
  static constexpr std::string_view kAllowedContentType = "audio/mpeg";
}

bool isValidInput(const userver::server::http::FormDataArg& form_data) {
  LOG_DEBUG() << "form_data.value: " << form_data.value;
  LOG_DEBUG() << "form_data.filename: " << form_data.filename.value_or("None");
  LOG_DEBUG() << "form_data.content_type: " << form_data.content_type.value_or("None");

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
    const auto& form_data = request.GetFormDataArg("file");

    if (!isValidInput(form_data)) {
      auto& response = request.GetHttpResponse();
      response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
      return {};
    }

    auto& response = request.GetHttpResponse();
    response.SetContentType("audio/mpeg");
    response.SetData(std::string{form_data.value});
    response.SetStatusOk();
    
    return std::string{form_data.value};
  }

  userver::storages::postgres::ClusterPtr pg_cluster_;
};


void AppendAudioCompressor(userver::components::ComponentList& component_list) {
  component_list.Append<Compress>();
}

} // namespace audio_compressor