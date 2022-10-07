#include "serialization.h"

#include <fstream>
#include <unordered_set>

namespace serialization {

using namespace std::string_literals;

Serialization::Serialization(TransportCatalogue& db, renderer::MapRenderer& map_renderer, router::Router& router, const Path& path)
    : db_(db), map_renderer_(map_renderer), router_(router), path_(path) {
}

void Serialization::SerializeDataBase() {
    std::ofstream out_file(path_, std::ios::binary);

    SerializeTransportCatalogue();
    SerializeMapRenderer();
    SerializeRouter();

    data_base_.SerializeToOstream(&out_file);
}

void Serialization::DeserializeDataBase() {
    std::ifstream in_file(path_, std::ios::binary);

    data_base_.ParseFromIstream(&in_file);

    DeserializeTransportCatalogue();
    DeserializeMapRenderer();
    DeserializeRouter();
}

transport_catalogue_serialize::Stop Serialization::SerializeStop(const domain::Stop& stop) {
    transport_catalogue_serialize::Stop result;
    result.set_name(stop.name);
    result.mutable_coordinates()->set_lat(stop.point.lat);
    result.mutable_coordinates()->set_lng(stop.point.lng);

    return result;
}

void Serialization::SerializeStops() {
    for (const auto& stop : db_.GetAllRawStops()) {
        *data_base_.mutable_transport_catalogue()->add_stops() = std::move(SerializeStop(stop));
    }
}

transport_catalogue_serialize::Distance Serialization::SerializeDistance(const domain::Stop* from, const domain::Stop* to, size_t distance) {
    transport_catalogue_serialize::Distance result;
    result.set_from(from->name);
    result.set_to(to->name);
    result.set_distance(distance);

    return result;
}

void Serialization::SerializeDistances() {
    for (const auto& [from_to, distance] : db_.GetDistancesBetweenStops()) {
        *data_base_.mutable_transport_catalogue()->add_stops_distance() = std::move(SerializeDistance(from_to.first, from_to.second, distance));
    }
}

transport_catalogue_serialize::Bus Serialization::SerializeBus(const domain::Bus& bus) {
    transport_catalogue_serialize::Bus result;
    result.set_route_type(bus.route_type == RouteType::Circular ? true : false);
    result.set_number(bus.number);
    for (const auto& stop : bus.stops) {
        result.add_stops(stop->name);
    }
    result.set_route_stops_count(bus.route_stops_count);
    result.set_route_length(bus.route_length);
    result.set_curvature(bus.curvature);
    if (bus.final_stop) {
        result.set_final_stop(bus.final_stop->name);
    } else {
        result.set_final_stop("");
    }
    
    return result;
}

void Serialization::SerializeBuses() {
    for (const auto& bus : db_.GetAllRawBuses()) {
        *data_base_.mutable_transport_catalogue()->add_buses() = std::move(SerializeBus(bus));
    }
}

void Serialization::SerializeTransportCatalogue() {
    SerializeStops();
    SerializeDistances();
    SerializeBuses();
}

void Serialization::DeserializeStop(const transport_catalogue_serialize::Stop& stop) {
    domain::Stop result;
    result.name = stop.name();
    result.point.lat = stop.coordinates().lat();
    result.point.lng = stop.coordinates().lng();
    db_.AddStop(std::move(result));
}

void Serialization::DeserializeStops() {
    for (const auto& stop : data_base_.transport_catalogue().stops()) {
        DeserializeStop(stop);
    }
}

void Serialization::DeserializeDistance(const transport_catalogue_serialize::Distance& distance) {
    db_.AddDistanceBetweenStops(distance.from(), distance.distance(), distance.to());
}

void Serialization::DeserializeDistances() {
    for (const auto& distance : data_base_.transport_catalogue().stops_distance()) {
        DeserializeDistance(distance);
    }
}

void Serialization::DeserializeBus(const transport_catalogue_serialize::Bus& bus) {
    std::unordered_set<const Stop*> unique_stops;
    domain::Bus result;

    result.route_type = bus.route_type() ? RouteType::Circular : RouteType::Pendulum;
    result.number = bus.number();
    for (const auto& stop : bus.stops()) {
        const Stop* bus_stop = db_.FindStop(stop);
        result.stops.emplace_back(bus_stop);
        unique_stops.emplace(bus_stop);
    }
    result.route_stops_count = bus.route_stops_count();
    result.unique_stops_count = unique_stops.size();
    result.route_length = bus.route_length();
    result.curvature = bus.curvature();

    if (!bus.final_stop().empty()) {
        result.final_stop = db_.FindStop(bus.final_stop());
    }

    db_.AddBus(std::move(result));

    for (const auto& stop : unique_stops) {
        db_.AddBusThroughStop(stop, bus.number());
    }
}

void Serialization::DeserializeBuses() {
    for (const auto& bus : data_base_.transport_catalogue().buses()) {
        DeserializeBus(bus);
    }
}

void Serialization::DeserializeTransportCatalogue() {
    DeserializeStops();
    DeserializeDistances();
    DeserializeBuses();
}

renderer_serialize::Color Serialization::SetSerialColor(const svg::Color& color) {
    renderer_serialize::Color result;

    if (std::holds_alternative<std::monostate>(color)) {
        result.set_is_none(true);
    } else if (std::holds_alternative<std::string>(color)) {
        result.set_name(std::get<std::string>(color));
    } else {
        bool is_rgba = std::holds_alternative<svg::Rgba>(color);
        result.mutable_rgba()->set_is_rgba(is_rgba);

        if (is_rgba) {
            svg::Rgba rgba = std::get<svg::Rgba>(color);
            result.mutable_rgba()->set_red(rgba.red);
            result.mutable_rgba()->set_green(rgba.green);
            result.mutable_rgba()->set_blue(rgba.blue);
            result.mutable_rgba()->set_opacity(rgba.opacity);
        } else {
            svg::Rgb rgb = std::get<svg::Rgb>(color);
            result.mutable_rgba()->set_red(rgb.red);
            result.mutable_rgba()->set_green(rgb.green);
            result.mutable_rgba()->set_blue(rgb.blue);
        }
    }
    return result;
}

void Serialization::SerializeMapRenderer() {
    //map_renderer_.PrintRenderSettings();
    const renderer::RenderSettings settings = map_renderer_.GetRenderSettings();

    data_base_.mutable_map_renderer()->mutable_screen()->set_width(settings.width);
    data_base_.mutable_map_renderer()->mutable_screen()->set_height(settings.height);
    data_base_.mutable_map_renderer()->mutable_screen()->set_padding(settings.padding);
    data_base_.mutable_map_renderer()->set_stop_radius(settings.stop_radius);
    data_base_.mutable_map_renderer()->set_line_width(settings.line_width);

    data_base_.mutable_map_renderer()->mutable_bus()->set_font_size(settings.bus_label_font_size);
    data_base_.mutable_map_renderer()->mutable_bus()->mutable_offset()->set_x(settings.bus_label_offset.x);
    data_base_.mutable_map_renderer()->mutable_bus()->mutable_offset()->set_y(settings.bus_label_offset.y);

    data_base_.mutable_map_renderer()->mutable_stop()->set_font_size(settings.stop_label_font_size);
    data_base_.mutable_map_renderer()->mutable_stop()->mutable_offset()->set_x(settings.stop_label_offset.x);
    data_base_.mutable_map_renderer()->mutable_stop()->mutable_offset()->set_y(settings.stop_label_offset.y);
    
    data_base_.mutable_map_renderer()->mutable_background()->set_width(settings.underlayer_width);
    *data_base_.mutable_map_renderer()->mutable_background()->mutable_color() = SetSerialColor(settings.underlayer_color);

    for (const auto& color : settings.color_palette) {
        data_base_.mutable_map_renderer()->mutable_color_palette()->Add(SetSerialColor(color));
    }
}

svg::Color Serialization::SetDeserialColor(const renderer_serialize::Color& color) {
    if (color.is_none()) {
        return std::monostate();
    } else if (!color.name().empty()) {
        return color.name();
    } else {
        bool is_rgba = color.rgba().is_rgba();
        if (is_rgba) {
            svg::Rgba result;
            result.red = color.rgba().red();
            result.green = color.rgba().green();
            result.blue = color.rgba().blue();
            result.opacity = color.rgba().opacity();
            return result;
        } else {
            svg::Rgb result;
            result.red = color.rgba().red();
            result.green = color.rgba().green();
            result.blue = color.rgba().blue();
            return result;
        }
    }
}

void Serialization::DeserializeMapRenderer() {
    renderer::RenderSettings settings;

    settings.width = data_base_.map_renderer().screen().width();
    settings.height = data_base_.map_renderer().screen().height();
    settings.padding = data_base_.map_renderer().screen().padding();
    settings.stop_radius = data_base_.map_renderer().stop_radius();
    settings.line_width = data_base_.map_renderer().line_width();

    settings.bus_label_font_size = static_cast<int>(data_base_.map_renderer().bus().font_size());
    settings.bus_label_offset.x = data_base_.map_renderer().bus().offset().x();
    settings.bus_label_offset.y = data_base_.map_renderer().bus().offset().y();

    settings.stop_label_font_size = static_cast<int>(data_base_.map_renderer().stop().font_size());
    settings.stop_label_offset.x = data_base_.map_renderer().stop().offset().x();
    settings.stop_label_offset.y = data_base_.map_renderer().stop().offset().y();

    settings.underlayer_width = data_base_.map_renderer().background().width();
    settings.underlayer_color = SetDeserialColor(data_base_.map_renderer().background().color());

    for (const auto& color : data_base_.map_renderer().color_palette()) {
        settings.color_palette.emplace_back(SetDeserialColor(color));
    }

    map_renderer_.SetRenderSettings(std::move(settings));
    //map_renderer_.PrintRenderSettings();
}

void Serialization::SerializeRoutingSettings() {
    //router_.PrintRoutingSettings();
    const router::RoutingSettings settings = router_.GetRoutingSettings();

    data_base_.mutable_router()->mutable_settings()->set_bus_wait_time(settings.bus_wait_time);
    data_base_.mutable_router()->mutable_settings()->set_bus_velocity(settings.bus_velocity);
}

void Serialization::SerializeGraph() {
    router_serialize::Graph result;
    
    size_t edge_count = router_.GetGraph().GetEdgeCount();
    for (size_t i = 0; i < edge_count; ++i) {
        const graph::Edge<double>& edge = router_.GetGraph().GetEdge(i);
        router_serialize::Edge s_edge;
        s_edge.set_name(edge.name);
        s_edge.set_span_count(edge.span_count);
        s_edge.set_from(edge.from);
        s_edge.set_to(edge.to);
        s_edge.set_weight(edge.weight);

        *result.add_edge() = s_edge;
    }

    size_t vertex_count = router_.GetGraph().GetVertexCount();
    for (size_t i = 0; i < vertex_count; ++i) {
        router_serialize::Vertex vertex;
        for (const auto& edge_id : router_.GetGraph().GetIncidentEdges(i)) {
            vertex.add_edge_id(edge_id);
        }
        *result.add_vertex() = vertex;
    }
    *data_base_.mutable_router()->mutable_graph() = result;    
}

void Serialization::SerializeStopIds() {
    for (const auto& [name, id] : router_.GetStopIds()) {
        router_serialize::StopId stop_id;
        stop_id.set_name(name);
        stop_id.set_id(id);
        *data_base_.mutable_router()->add_stop_id() = stop_id;
    }
}

void Serialization::SerializeRouter() {
    SerializeRoutingSettings();
    SerializeGraph();
    SerializeStopIds();
}

void Serialization::DeserializeRoutingSettings() {
    router::RoutingSettings settings;

    settings.bus_wait_time = data_base_.router().settings().bus_wait_time();
    settings.bus_velocity = data_base_.router().settings().bus_velocity();

    router_.SetRoutingSettings(std::move(settings));
    //router_.PrintRoutingSettings();
}

void Serialization::DeserializeGraph() {
    
    std::vector<graph::Edge<double>> edges(data_base_.router().graph().edge_size());
    for (size_t i = 0; i < edges.size(); ++i) {
        const router_serialize::Edge& e = data_base_.router().graph().edge(i);
        edges[i] = {e.name(), static_cast<size_t>(e.span_count()),
        static_cast<size_t>(e.from()), static_cast<size_t>(e.to()), e.weight()};
    }

    std::vector<std::vector<graph::EdgeId>> incidence_lists(data_base_.router().graph().vertex_size());
    for (size_t i = 0; i < incidence_lists.size(); ++i) {
        const router_serialize::Vertex& v = data_base_.router().graph().vertex(i);
        incidence_lists[i].reserve(v.edge_id_size());
        for (const auto& id : v.edge_id()) {
            incidence_lists[i].push_back(id);
        }
    }

    graph::DirectedWeightedGraph<double> graph(edges, incidence_lists);

    router_.SetGraph(std::move(graph));
}

void Serialization::DeserializeStopIds() {
    std::map<std::string, graph::VertexId> stop_ids;
    for (const auto& s : data_base_.router().stop_id()) {
        stop_ids[s.name()] = s.id();
    }

    router_.SetStopIds(std::move(stop_ids));
}

void Serialization::DeserializeRouter() {
    DeserializeRoutingSettings();
    DeserializeGraph();
    DeserializeStopIds();
}

} // namespace serialization
