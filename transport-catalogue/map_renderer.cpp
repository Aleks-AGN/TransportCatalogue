#include "map_renderer.h"

namespace renderer {

using namespace std::literals;


bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {
        (coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_
    };
}

void MapRenderer::SetRenderSettings(RenderSettings settings) {
        render_settings_ = std::move(settings);
    }

void MapRenderer::PrintRenderSettings() const {

    std::cout << "width = "s << render_settings_.width << std::endl;
    std::cout << "height = "s << render_settings_.height << std::endl;
    std::cout << "padding = "s << render_settings_.padding << std::endl;
    std::cout << "stop_radius = "s << render_settings_.stop_radius << std::endl;
    std::cout << "line_width = "s << render_settings_.line_width << std::endl;
    std::cout << "-----------------------------------------------------------"s << std::endl;
    std::cout << "bus_label_font_size = "s << render_settings_.bus_label_font_size << std::endl;
    std::cout << "bus_label_offset dx = "s << render_settings_.bus_label_offset.x << std::endl;
    std::cout << "bus_label_offset dy = "s << render_settings_.bus_label_offset.y << std::endl;
    std::cout << "-----------------------------------------------------------"s << std::endl;
    std::cout << "stop_label_font_size = "s << render_settings_.stop_label_font_size << std::endl;
    std::cout << "stop_label_offset dx = "s << render_settings_.stop_label_offset.x << std::endl;
    std::cout << "stop_label_offset dy = "s << render_settings_.stop_label_offset.y << std::endl;
    std::cout << "-----------------------------------------------------------"s << std::endl;
    std::cout << "underlayer_color = "s << render_settings_.underlayer_color << std::endl;
    std::cout << "-----------------------------------------------------------"s << std::endl;
    std::cout << "underlayer_width = "s << render_settings_.underlayer_width << std::endl;
    std::cout << "-----------------------------------------------------------"s << std::endl;
    std::cout << "color_palette first color = "s << render_settings_.color_palette[0] << std::endl;
    std::cout << "color_palette second color = "s << render_settings_.color_palette[1] << std::endl;
    std::cout << "color_palette third color = "s << render_settings_.color_palette[2] << std::endl;
}

svg::Polyline MapRenderer::GetBusRoute(const domain::Bus* bus, const SphereProjector& proj, size_t color_number) const {

    svg::Polyline result;
        
    for (const auto& stop : bus->stops) {
        result.AddPoint(proj(stop->point));
    }

    result.SetFillColor("none"s);
    result.SetStrokeColor(render_settings_.color_palette[color_number]);
    result.SetStrokeWidth(render_settings_.line_width);
    result.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    result.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        
    return result;
}

std::vector<svg::Text> MapRenderer::GetBusTitle(const domain::Bus* bus, const SphereProjector& proj, size_t color_number) const {

    std::vector<svg::Text> result;
    svg::Text text_underlayer;
    svg::Text text_title;
    
    svg::Point start_point = proj(bus->stops[0]->point);
    text_underlayer.SetPosition(start_point);
    text_title.SetPosition(start_point);
    text_underlayer.SetOffset(render_settings_.bus_label_offset);
    text_title.SetOffset(render_settings_.bus_label_offset);
    text_underlayer.SetFontSize(render_settings_.bus_label_font_size);
    text_title.SetFontSize(render_settings_.bus_label_font_size);
    text_underlayer.SetFontFamily("Verdana"s);
    text_title.SetFontFamily("Verdana"s);
    text_underlayer.SetFontWeight("bold"s);
    text_title.SetFontWeight("bold"s);
    text_underlayer.SetData(bus->number);
    text_title.SetData(bus->number);
    text_underlayer.SetFillColor(render_settings_.underlayer_color);
    text_underlayer.SetStrokeColor(render_settings_.underlayer_color);
    text_underlayer.SetStrokeWidth(render_settings_.underlayer_width);
    text_underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    text_underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    text_title.SetFillColor(render_settings_.color_palette[color_number]);

    if ((bus->route_type == domain::RouteType::Pendulum) && (bus->final_stop) && (bus->final_stop->name != bus->stops[0]->name)) {
        svg::Text text_final_underlayer = text_underlayer;
        svg::Text text_final_title = text_title;
        svg::Point final_point = proj(bus->final_stop->point);
        text_final_underlayer.SetPosition(final_point);
        text_final_title.SetPosition(final_point);
        text_final_underlayer.SetData(bus->number);
        text_final_title.SetData(bus->number);
        result.emplace_back(std::move(text_underlayer));
        result.emplace_back(std::move(text_title));
        result.emplace_back(std::move(text_final_underlayer));
        result.emplace_back(std::move(text_final_title));
    } else {
        result.emplace_back(std::move(text_underlayer));
        result.emplace_back(std::move(text_title));
    }
    return result;
}

svg::Circle MapRenderer::GetStopCircle(const domain::Stop* stop, const SphereProjector& proj) const {

    svg::Circle result;
        
    result.SetCenter(proj(stop->point));
    result.SetRadius(render_settings_.stop_radius);
    result.SetFillColor("white"s);
        
    return result;
}

std::vector<svg::Text> MapRenderer::GetStopTitle(const domain::Stop* stop, const SphereProjector& proj) const {

    std::vector<svg::Text> result;
    svg::Text text_underlayer;
    svg::Text text_title;

    svg::Point stop_point = proj(stop->point);
    text_underlayer.SetPosition(stop_point);
    text_title.SetPosition(stop_point);
    text_underlayer.SetOffset(render_settings_.stop_label_offset);
    text_title.SetOffset(render_settings_.stop_label_offset);
    text_underlayer.SetFontSize(render_settings_.stop_label_font_size);
    text_title.SetFontSize(render_settings_.stop_label_font_size);
    text_underlayer.SetFontFamily("Verdana"s);
    text_title.SetFontFamily("Verdana"s);
    text_underlayer.SetData(stop->name);
    text_title.SetData(stop->name);
    text_underlayer.SetFillColor(render_settings_.underlayer_color);
    text_underlayer.SetStrokeColor(render_settings_.underlayer_color);
    text_underlayer.SetStrokeWidth(render_settings_.underlayer_width);
    text_underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    text_underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    text_title.SetFillColor("black"s);
    
    result.emplace_back(std::move(text_underlayer));
    result.emplace_back(std::move(text_title));
        
    return result;
}

svg::Document MapRenderer::GetSvgDocument(const std::unordered_map<std::string_view, const domain::Bus*>& buses) const {
    svg::Document result; 
    std::vector<geo::Coordinates> geo_coords;
    std::vector<std::string_view> bus_names;
    std::map<std::string_view, const domain::Stop*> buses_stops;

    bus_names.reserve (buses.size());
    for (const auto& it : buses) { bus_names.emplace_back(it.first); }
    std::sort(bus_names.begin(), bus_names.end());

    for (const auto& bus_name : bus_names) {
        if (buses.at(bus_name)->stops.size() == 0) continue;
        for (const auto& stop : buses.at(bus_name)->stops) {
            geo_coords.emplace_back(stop->point);
            buses_stops[stop->name] = stop;
        }
    }
    
    SphereProjector proj(geo_coords.begin(), geo_coords.end(), render_settings_.width, render_settings_.height, render_settings_.padding);

    std::vector<svg::Polyline> bus_routes;
    std::vector<svg::Text> bus_titles;
    size_t color_number = 0;
    for (const auto& bus_name : bus_names) {
        if (buses.at(bus_name)->stops.size() == 0) continue;

        bus_routes.emplace_back(GetBusRoute(buses.at(bus_name), proj, color_number));
        for (const auto& title : GetBusTitle(buses.at(bus_name), proj, color_number)) {
            bus_titles.emplace_back(title);
        }

        if (color_number < (render_settings_.color_palette.size() - 1)) {
            ++color_number;
        } else {
            color_number = 0;
        }
    }

    std::vector<svg::Circle> stop_circles;
    std::vector<svg::Text> stop_titles;
    for (const auto& [stop_name, stop] : buses_stops) {
        stop_circles.emplace_back(GetStopCircle(stop, proj));
        for (const auto& title : GetStopTitle(stop, proj)) {
            stop_titles.emplace_back(title);
        }
    }

    for (const auto& bus_route : bus_routes) { result.Add(bus_route); }
    for (const auto& bus_title : bus_titles) { result.Add(bus_title); }
    for (const auto& stop_circle : stop_circles) { result.Add(stop_circle); }
    for (const auto& stop_title : stop_titles) { result.Add(stop_title); }

    return result;
}

} // namespace renderer
