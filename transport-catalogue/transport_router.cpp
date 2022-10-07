#include "transport_router.h"

#include <iostream>
#include <unordered_map>

namespace router {

using namespace std::literals;

void Router::SetRoutingSettings(RoutingSettings settings) {
    routing_settings_ = std::move(settings);
}

const RoutingSettings& Router::GetRoutingSettings() const {
    return routing_settings_;
}

void Router::BuildGraph(const TransportCatalogue& db) {

    const std::unordered_map<std::string_view, const Bus*>& all_buses = db.GetAllBuses();
    const std::unordered_map<std::string_view, const Stop*>& all_stops = db.GetAllStops();
    
    graph::DirectedWeightedGraph<double> graph(all_stops.size() * 2);

    std::map<std::string, graph::VertexId> stop_ids;
    graph::VertexId vertex_id = 0;
    
    for (const auto& stop : all_stops) {
        stop_ids[stop.second->name] = vertex_id;
        graph.AddEdge({stop.second->name, 0, vertex_id, ++vertex_id, static_cast<double>(routing_settings_.bus_wait_time)});
        ++vertex_id;
    }
    stop_ids_ = move(stop_ids);

    for (const auto& bus : all_buses) {
        const auto& bus_ptr = bus.second;
        const std::vector<const Stop*>& stops = bus_ptr->stops;
        size_t stops_count = stops.size();
        
        for (size_t i = 0; i < stops_count; ++i) {
            for (size_t j = i + 1; j < stops_count; ++j) {
                const Stop* stop_from = stops[i];
                const Stop* stop_to = stops[j];
                int length = 0;
                for (size_t k = i + 1; k <= j; ++k) {
                    length += db.GetDistanceBetweenStops(stops[k - 1]->name, stops[k]->name);
                }
                graph.AddEdge({bus_ptr->number, j - i, stop_ids_.at(stop_from->name) + 1, stop_ids_.at(stop_to->name),
                                length / (routing_settings_.bus_velocity * (100.0 / 6.0))});
                if (bus_ptr->route_type == RouteType::Pendulum && stop_to == bus_ptr->final_stop && j == stops_count / 2) {
                    break;
                } 
            }
        }
    }
    graph_ = std::move(graph);
    router_ptr_ = new graph::Router<double>(graph_);
}

void Router::SetGraph(graph::DirectedWeightedGraph<double>&& graph) {
    graph_ = std::move(graph);
    router_ptr_ = new graph::Router<double>(graph_);
}

const graph::DirectedWeightedGraph<double>& Router::GetGraph() const {
    return graph_;
}

void Router::SetStopIds(std::map<std::string, graph::VertexId>&& stop_ids) {
    stop_ids_ = std::move(stop_ids);
}

const std::map<std::string, graph::VertexId>& Router::GetStopIds() const {
    return stop_ids_;
}

std::optional<graph::Router<double>::RouteInfo> Router::GetRouteInfo(const Stop* from_stop, const Stop* to_stop) const {
    return router_ptr_->BuildRoute(stop_ids_.at(from_stop->name), stop_ids_.at(to_stop->name));
}

json::Array Router::GetEdgesInfo(const std::vector<graph::EdgeId>& edges) const {
    json::Array items_array;
    items_array.reserve(edges.size());
    for (auto& edge_id : edges) {
        const graph::Edge<double>& edge = graph_.GetEdge(edge_id);
        if (edge.span_count == 0) {
            items_array.emplace_back(json::Node(json::Dict{
                {{"stop_name"s},{static_cast<std::string>(edge.name)}},
                {{"time"s},{edge.weight}},
                {{"type"s},{"Wait"s}}
            }));
        } else {
            items_array.emplace_back(json::Node(json::Dict{
                {{"bus"s},{static_cast<std::string>(edge.name)}},
                {{"span_count"s},{static_cast<int>(edge.span_count)}},
                {{"time"s},{edge.weight}},
                {{"type"s},{"Bus"s}}
            }));
        }
    }
    return items_array;
}

void Router::PrintRoutingSettings() const {
    std::cout << "bus_wait_time = "s << routing_settings_.bus_wait_time << std::endl;
    std::cout << "bus_velocity = "s << routing_settings_.bus_velocity << std::endl;
}

} // namespace router
