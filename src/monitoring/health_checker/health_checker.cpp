#include "health_checker.hpp"

#include <userver/clients/http/client.hpp>
#include <userver/formats/json.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <cstdlib>

namespace healthcheck {

HealthChecker::HealthChecker(const userver::components::ComponentConfig& config,
                             const userver::components::ComponentContext& context)
    : userver::components::LoggableComponentBase(config, context),
      client_(context.FindComponent<userver::components::HttpClient>().GetHttpClient()),
      periodic_task_("SendImAliveTask", userver::utils::PeriodicTask::Settings{
                                             std::chrono::seconds(10),  
                                             {}                         
                                         },
                     [this]() {
                    //   std::cout<<"Hello from periodic_task" <<std::endl;
                       this->SendImAlive("http://192.168.0.60:8080/v1/imalive");
                     }) {

}

HealthChecker::~HealthChecker() {
  periodic_task_.Stop();  
}

void HealthChecker::SendImAlive(const std::string& url) {
  try {
    userver::formats::json::ValueBuilder body;
    body["url"] = "http://192.168.0.61:8080/v1/compress-audio";
    auto bodyStr = userver::formats::json::ToString(body.ExtractValue());

    auto response = client_.CreateRequest()
                        .put(url, bodyStr)
                        .timeout(std::chrono::seconds(5))
                        .perform();

    if (response) {    
        std::cout << "Health check successful"<<response->body()<< std::endl;
      } else {
        std::cerr << "Health check failed"<< std::endl;
      }
    } catch (const std::exception& e) {
      std::cerr << "Exception during health check: " << e.what() << std::endl;
    }
}

void AppendHealthChecker(userver::components::ComponentList& component_list) {
  component_list.Append<HealthChecker>();
}

}  // namespace healthcheck
