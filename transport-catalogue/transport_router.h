#pragma once

#include "domain.h"
#include "json.h"
#include "router.h"
#include "transport_catalogue.h"

#include <map>

namespace router {

using namespace transport_catalogue;

struct RoutingSettings {
    int bus_wait_time = 0;
    double bus_velocity = 0.0;
};

class Router {
public:
    Router(const TransportCatalogue& db)
        : db_(db) {
    }

    void SetRoutingSettings(RoutingSettings settings);

    const RoutingSettings& GetRoutingSettings() const;

    void BuildGraph(const TransportCatalogue& db);
    
    void SetGraph(graph::DirectedWeightedGraph<double>&& graph);

    const graph::DirectedWeightedGraph<double>& GetGraph() const;

    void SetStopIds(std::map<std::string, graph::VertexId>&& stop_ids);

    const std::map<std::string, graph::VertexId>& GetStopIds() const;

    std::optional<graph::Router<double>::RouteInfo> GetRouteInfo(const Stop* from_stop, const Stop* to_stop) const;

    json::Array GetEdgesInfo(const std::vector<graph::EdgeId>& edges) const;
    
    void PrintRoutingSettings() const;

    ~Router() {
        delete router_ptr_;
    }

private:
    const TransportCatalogue& db_;
    RoutingSettings routing_settings_;
    graph::DirectedWeightedGraph<double> graph_;
    graph::Router<double>* router_ptr_ = nullptr;
    std::map<std::string, graph::VertexId> stop_ids_;
};

} // namespace router
