#pragma once

#include "geo.h"

#include <string>
#include <vector>

namespace domain {

enum class RouteType {
    Pendulum,
    Circular
};

struct Stop {
    std::string name;
    geo::Coordinates point;

    size_t Hash() const;
};

struct Bus {
    std::string number;
    RouteType route_type;
    std::vector<const Stop*> stops;
    size_t route_stops_count;
    size_t unique_stops_count;
    size_t route_length;
    double curvature;
    const Stop* final_stop = nullptr;
};

} // namespace domain
