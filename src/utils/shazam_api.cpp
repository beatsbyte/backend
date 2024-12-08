#include "shazam_api.hpp"
#include <curl/curl.h>
#include <fmt/format.h>
#include <stdexcept>

namespace shazam_api {
namespace {
  namespace config {
    static constexpr std::string_view kShazamApiHost = "shazam-api6.p.rapidapi.com";
    static constexpr std::string_view kShazamApiKey = "265928d64fmsh87a3e9feb2ece3bp163256jsn269b8640dd26";
    static constexpr std::string_view kShazamApiUrl = "https://shazam-api6.p.rapidapi.com/shazam/recognize/";
  }

  size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t totalSize = size * nmemb;
    response->append(static_cast<char*>(contents), totalSize);
    return totalSize;
  }
}  // namespace

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

  headers = curl_slist_append(headers, fmt::format("x-rapidapi-host: {}", config::kShazamApiHost).c_str());
  headers = curl_slist_append(headers, fmt::format("x-rapidapi-key: {}", config::kShazamApiKey).c_str());

  curl_easy_setopt(curl, CURLOPT_URL, config::kShazamApiUrl.data());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

  res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    curl_easy_cleanup(curl);
    curl_formfree(formpost);
    curl_slist_free_all(headers);
    throw std::runtime_error(fmt::format("Failed to identify audio via Shazam API: {}", curl_easy_strerror(res)));
  }

  curl_easy_cleanup(curl);
  curl_formfree(formpost);
  curl_slist_free_all(headers);

  return userver::formats::json::FromString(response_data);
}

}  // namespace shazam_api
