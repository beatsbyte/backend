#include "view.hpp"

#include <fmt/format.h> 
#include <curl/curl.h> // Подключение библиотеки curl

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
  static constexpr std::string_view kShazamApiHost = "shazam-api6.p.rapidapi.com";
  static constexpr std::string_view kShazamApiKey = "265928d64fmsh87a3e9feb2ece3bp163256jsn269b8640dd26";

  // Функция обратного вызова для записи ответа от curl
  size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t totalSize = size * nmemb;
    response->append(static_cast<char*>(contents), totalSize);
    return totalSize;
  }

  // Функция для отправки аудиофайла в Shazam API через curl
  userver::formats::json::Value IdentifyAudio(const std::string& audio_path) {
    CURL* curl;
    CURLcode res;
    std::string response_data;

    curl = curl_easy_init();
    if (!curl) {
      throw std::runtime_error("Failed to initialize curl");
    }

    struct curl_httppost* formpost = nullptr;
    struct curl_httppost* lastptr = nullptr;

    // Создаем multipart/form-data с файлом
    curl_formadd(&formpost, &lastptr,
                 CURLFORM_COPYNAME, "upload_file",
                 CURLFORM_FILE, audio_path.c_str(),
                 CURLFORM_END);

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "x-rapidapi-host: shazam-api6.p.rapidapi.com");
    headers = curl_slist_append(headers, "x-rapidapi-key: 265928d64fmsh87a3e9feb2ece3bp163256jsn269b8640dd26");

    curl_easy_setopt(curl, CURLOPT_URL, "https://shazam-api6.p.rapidapi.com/shazam/recognize/");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

    // Выполняем запрос
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      LOG_ERROR() << "curl_easy_perform() failed: " << curl_easy_strerror(res);
      curl_easy_cleanup(curl);
      curl_formfree(formpost);
      curl_slist_free_all(headers);
      throw std::runtime_error("Failed to identify audio via Shazam API");
    }

    // Очистка ресурсов
    curl_easy_cleanup(curl);
    curl_formfree(formpost);
    curl_slist_free_all(headers);

    return userver::formats::json::FromString(response_data);
  }

  // Проверка входных данных
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
        cache_(100)  // Размер кэша на 100 элементов
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
    auto shazam_result = IdentifyAudio(temp_file_path);
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

    std::cout << "shazam :" << filename_with_metadata;

    // Проверяем, есть ли результат в кэше
    if (cache_.Contains(filename_with_metadata)) {
      LOG_DEBUG() << "Cache hit for file: " << filename_with_metadata;
      compressedData = cache_.Get(filename_with_metadata);
    } else {
      LOG_DEBUG() << "Cache miss for file: " << filename_with_metadata;
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
