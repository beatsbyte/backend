#pragma once

#include <string>
#include <userver/clients/http/client.hpp>
#include <userver/clients/http/component.hpp>
#include <userver/components/component.hpp>
#include <userver/components/component_list.hpp>
#include <userver/utils/periodic_task.hpp>

namespace healthcheck {

class HealthChecker : public userver::components::LoggableComponentBase {
 public:
  std::string balancer_url;  
  std::string own_url;

  static constexpr std::string_view kName = "health-checker";

  HealthChecker(const userver::components::ComponentConfig& config,
                const userver::components::ComponentContext& context);
  ~HealthChecker();

  void SendImAlive();

 private:
  userver::clients::http::Client& client_;
  userver::utils::PeriodicTask periodic_task_;  
};

void AppendHealthChecker(userver::components::ComponentList& component_list);
}  // namespace healthcheck
