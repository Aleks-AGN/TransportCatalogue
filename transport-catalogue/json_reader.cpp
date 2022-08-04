#include "json_reader.h"
#include "geo.h"
#include <unordered_set>
#include <sstream>

namespace transport_catalogue {

using namespace std::string_literals;

std::pair<Stop, bool> JsonReader::ParseStopRequest(const Dict& request) const {
    Stop stop;

    stop.name = request.at("name"s).AsString();
    stop.point.lat = request.at("latitude"s).AsDouble();
    stop.point.lng = request.at("longitude"s).AsDouble();
    bool has_road_distances = !request.at("road_distances"s).AsMap().empty();

    return {std::move(stop), has_road_distances};
}

void JsonReader::AddBuses(TransportCatalogue& db, const Array& base_requests,
    const std::vector<int> bus_request_ids) const {

    for (int id : bus_request_ids) {
        const auto& request = base_requests.at(id).AsMap();
        Bus bus;
        bus.number = request.at("name"s).AsString();
        bus.route_type = request.at("is_roundtrip"s).AsBool() ? RouteType::Circular : RouteType::Pendulum;
        bus.route_stops_count = 0;
        bus.route_length = 0;
        double calc_route_length = 0.0;

        std::string prev_stop;
        std::string curr_stop;  
        std::unordered_set<const Stop*> unique_stops;
        
        const auto& stops = request.at("stops"s).AsArray();
        for (const auto& stop : stops) {
            prev_stop = std::move(curr_stop);
            curr_stop = stop.AsString();
            const Stop* bus_stop = db.FindStop(curr_stop);
            bus.stops.emplace_back(bus_stop);
            unique_stops.emplace(bus_stop);
            if (bus.route_stops_count) {
                calc_route_length += geo::ComputeDistance(
                                    db.FindStop(prev_stop)->point,
                                    db.FindStop(curr_stop)->point);
                bus.route_length +=	db.GetDistanceBetweenStops(prev_stop, curr_stop);
                if (bus.route_type == RouteType::Pendulum) {
                    bus.route_length +=	db.GetDistanceBetweenStops(curr_stop, prev_stop);
                }
            }
            ++bus.route_stops_count;
        }
        if (bus.route_type == RouteType::Pendulum) {
            bus.final_stop = bus.stops.back();
            bus.route_stops_count = bus.route_stops_count * 2 - 1;
            std::vector<const Stop*> temp_stops(bus.stops);
            for (int i = temp_stops.size() - 2; i >= 0; --i) {
                bus.stops.emplace_back(temp_stops[i]);
            }
            calc_route_length *= 2;
        }
        bus.unique_stops_count = unique_stops.size();
        bus.curvature = bus.route_length / calc_route_length;
        std::string bus_number = bus.number;

        db.AddBus(std::move(bus));

        for (const auto& stop : unique_stops) {
            db.AddBusThroughStop(stop, bus_number);
        }
    }
}

void JsonReader::AddRoadDistances(TransportCatalogue& db, const Array& base_requests,
    const std::vector<int> stop_with_distances_request_ids) const {

    for (int id : stop_with_distances_request_ids) {
        const auto& request = base_requests.at(id).AsMap();

        std::string stop_from = request.at("name"s).AsString();

        for (const auto& [stop_to, distance] : request.at("road_distances"s).AsMap()) {
            db.AddDistanceBetweenStops(stop_from, distance.AsInt(), stop_to);
        }    
    }
}

void JsonReader::UpdateTransportCatalogue(TransportCatalogue& db) const {
    const Array& base_requests = input_doc_.GetRoot().AsMap().at("base_requests"s).AsArray();

    std::vector<int> stop_with_distances_request_ids;
    std::vector<int> bus_request_ids;
    stop_with_distances_request_ids.reserve(base_requests.size());
    bus_request_ids.reserve(base_requests.size());
    
    for (size_t id = 0; id < base_requests.size(); ++id) {
        const auto& request = base_requests.at(id).AsMap();
        
        if (request.at("type"s) == "Stop"s) {
            auto [stop, has_road_distances] = ParseStopRequest(request);
            if (has_road_distances) {
                stop_with_distances_request_ids.emplace_back(id);
            }
            db.AddStop(std::move(stop));
        } else if (request.at("type"s) == "Bus"s) {
            bus_request_ids.emplace_back(id);
        }
    }
    
    AddRoadDistances(db, base_requests, stop_with_distances_request_ids);

    AddBuses(db, base_requests, bus_request_ids);
}

void JsonReader::UpdateMapRenderer(renderer::MapRenderer& renderer) const {
    const auto& render_settings = input_doc_.GetRoot().AsMap().at("render_settings"s).AsMap();
    renderer::RenderSettings settings;
    
    settings.width = render_settings.at("width").AsDouble();
    settings.height = render_settings.at("height").AsDouble();
    settings.padding = render_settings.at("padding").AsDouble();
    settings.stop_radius = render_settings.at("stop_radius").AsDouble();
    settings.line_width = render_settings.at("line_width").AsDouble();
    
    settings.bus_label_font_size = render_settings.at("bus_label_font_size").AsInt();
    const Array& bus_label_offset = render_settings.at("bus_label_offset").AsArray();
    settings.bus_label_offset = { bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble() };

    settings.stop_label_font_size = render_settings.at("stop_label_font_size").AsInt();
    const Array& stop_label_offset = render_settings.at("stop_label_offset").AsArray();
    settings.stop_label_offset = { stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble() };

    if (render_settings.at("underlayer_color").IsArray()) {
        const Array& array = render_settings.at("underlayer_color").AsArray();
        if (array.size() == 3) {
            svg::Rgb rgb_color(array[0].AsInt(), array[1].AsInt(), array[2].AsInt());
            settings.underlayer_color = rgb_color;
        } else if (array.size() == 4) {
            svg::Rgba rgba_color(array[0].AsInt(), array[1].AsInt(), array[2].AsInt(), array[3].AsDouble());;
            settings.underlayer_color = rgba_color;
        }
    } else if (render_settings.at("underlayer_color").IsString()) {
        settings.underlayer_color = render_settings.at("underlayer_color").AsString();
    }

    settings.underlayer_width = render_settings.at("underlayer_width").AsDouble();

    const Array& color_palette = render_settings.at("color_palette").AsArray();

    for (const Node& node : color_palette) {
        if (node.IsArray()) {
            const Array& array = node.AsArray();
            if (array.size() == 3) {
                svg::Rgb rgb_color(array[0].AsInt(), array[1].AsInt(), array[2].AsInt());
                settings.color_palette.emplace_back(rgb_color);
            }
            else if (array.size() == 4) {
                svg::Rgba rgba_color(array[0].AsInt(), array[1].AsInt(), array[2].AsInt(), array[3].AsDouble());
                settings.color_palette.emplace_back(rgba_color);
            }
        }
        else if (node.IsString()) {
            settings.color_palette.emplace_back(node.AsString());
        }
    }

    renderer.SetRenderSettings(std::move(settings));
    //renderer.PrintRenderSettings();
}

Node JsonReader::ProcessStatRequests(RequestHandler& request_handler) const {
    const Array& stat_requests = input_doc_.GetRoot().AsMap().at("stat_requests"s).AsArray();
    Array responses;
    responses.reserve(stat_requests.size());

    for (const auto& stat_request : stat_requests) {
        const auto& request = stat_request.AsMap();
        Dict response;
        response.emplace("request_id"s, request.at("id"s).AsInt());
        std::string type = request.at("type"s).AsString();
        
        if (type == "Bus"s) {
            std::string name = request.at("name"s).AsString();
            const Bus* bus = request_handler.GetTransportCatalogue().FindBus(name);
            if (bus != nullptr) {
                auto [route_stops_count, unique_stops_count, route_length, curvature] = request_handler.GetTransportCatalogue().GetRouteInfo(bus);
                response.emplace("curvature"s, curvature);
                response.emplace("route_length"s, static_cast<int>(route_length));
                response.emplace("stop_count"s, static_cast<int>(route_stops_count));
                response.emplace("unique_stop_count"s, static_cast<int>(unique_stops_count));
            } else {
                response.emplace("error_message"s, "not found"s);  
            }
        } else if (type == "Stop"s) {
            std::string name = request.at("name"s).AsString();
            const Stop* stop = request_handler.GetTransportCatalogue().FindStop(name);
            if (stop != nullptr) {
                Array buses;
                const auto buses_through_stop = request_handler.GetTransportCatalogue().GetBusesThroughStop(stop);
                if (buses_through_stop != nullptr) {
                    buses.reserve(buses_through_stop->size());
                    for (const auto& bus : *buses_through_stop) {
                        buses.emplace_back(bus->number);
                    }
                }
                response.emplace("buses"s, std::move(buses));
            } else {
                response.emplace("error_message"s, "not found"s);  
            }
        } else if (type == "Map"s) {
            std::ostringstream strm;
            request_handler.RenderMap().Render(strm);
            response.emplace("map"s, std::move(strm.str()));
        }
        responses.emplace_back(response);
    }
    return responses;
}

} // namespace transport_catalogue
