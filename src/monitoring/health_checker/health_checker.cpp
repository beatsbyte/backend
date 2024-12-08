#include "health_checker.hpp"

#include <userver/clients/http/client.hpp>
#include <userver/formats/json.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <cstdlib>

namespace healthcheck {

HealthChecker::HealthChecker(const userver::components::ComponentConfig& config,
                             const userver::components::ComponentContext& context
                            )
    : userver::components::LoggableComponentBase(config, context),
      client_(context.FindComponent<userver::components::HttpClient>().GetHttpClient()),
      periodic_task_("SendImAliveTask", userver::utils::PeriodicTask::Settings{
                                             std::chrono::seconds(5),  
                                             {}                         
                                         },
                     [this]() {
                       this->SendImAlive();
                     }) {                 
      balancer_url= std::getenv("BALANCER_URL");
      own_url = std::getenv("OWN_URL");
}

HealthChecker::~HealthChecker() {
  periodic_task_.Stop();  
}

void HealthChecker::SendImAlive() {
  try {
    userver::formats::json::ValueBuilder body;
    body["url"] = own_url;
    auto bodyStr = userver::formats::json::ToString(body.ExtractValue());

    auto response = client_.CreateRequest()
                        .put(balancer_url, bodyStr)
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
