#include "transport_catalogue.h"

namespace transport_catalogue {

void TransportCatalogue::AddBus(const Bus& bus) {
    const auto pos = buses_.insert(buses_.begin(), std::move(bus));
    index_buses_.insert({pos->number, &(*pos)});
}

void TransportCatalogue::AddStop(const Stop& stop) {
    const auto pos = stops_.insert(stops_.begin(), std::move(stop));
    index_stops_.insert({pos->name, &(*pos)});
}

const Bus* TransportCatalogue::FindBus(const std::string& name) const {
    auto it = index_buses_.find(name);
    return it == index_buses_.end() ? nullptr : it->second;
}

const Stop* TransportCatalogue::FindStop(const std::string& name) const {
    auto it = index_stops_.find(name);
    return it == index_stops_.end() ? nullptr : it->second;
}

std::tuple<size_t, size_t, size_t, double> TransportCatalogue::GetRouteInfo(const Bus* bus) const {
    return {bus->route_stops_count, bus->unique_stops_count, bus->route_length, bus->curvature};
}

void TransportCatalogue::AddBusThroughStop(const Stop* stop, const std::string& bus_number) {
    index_buses_through_stop_[stop].emplace(FindBus(bus_number));
}

const std::set<const Bus*, detail::CompareBuses>* TransportCatalogue::GetBusesThroughStop(const Stop* stop) {
    auto it = index_buses_through_stop_.find(stop);
    return it == index_buses_through_stop_.end() ? nullptr : &(it->second);
}

void TransportCatalogue::AddDistanceBetweenStops(const std::string& from_stop, const size_t distance, const std::string& to_stop) {
    index_distances_between_stops_.emplace(std::pair(FindStop(from_stop), FindStop(to_stop)), distance);
}

size_t TransportCatalogue::GetDistanceBetweenStops(const std::string& from_stop, const std::string& to_stop) {
    auto it = index_distances_between_stops_.find(std::pair(FindStop(from_stop), FindStop(to_stop)));

    if (it != index_distances_between_stops_.end()) {
        return it->second;
    } else {
        it = index_distances_between_stops_.find(std::pair(FindStop(to_stop), FindStop(from_stop)));
        return it->second;
    }
}

} // namespace transport_catalogue
