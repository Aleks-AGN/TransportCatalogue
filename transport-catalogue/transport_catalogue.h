#pragma once

#include "domain.h"
#include "geo.h"

#include <string>
#include <deque>
#include <unordered_map>
#include <set>

namespace transport_catalogue {

using namespace domain;

namespace detail {

struct CompareBuses {
    bool operator() (const Bus* lhs, const Bus* rhs) const {
        return lhs->number < rhs->number;
    }
};

struct PairHash {
    size_t operator() (const std::pair<const Stop*, const Stop*>& pair) const {
        return pair.first->Hash() + 37 * pair.second->Hash();
    }
};

} // namespace detail

class TransportCatalogue {
public:

    TransportCatalogue() = default;

    void AddBus(const Bus& bus);

    void AddStop(const Stop& stop);

    const Bus* FindBus(const std::string& name) const;

    const Stop* FindStop(const std::string& name) const;

    std::tuple<size_t, size_t, size_t, double> GetRouteInfo(const Bus* bus) const;

    void AddBusThroughStop(const Stop* stop, const std::string& bus_number);

    const std::set<const Bus*, detail::CompareBuses>* GetBusesThroughStop(const Stop* stop) const;

    void AddDistanceBetweenStops(const std::string& from_stop, const size_t distance, const std::string& to_stop);

    size_t GetDistanceBetweenStops(const std::string& from_stop, const std::string& to_stop) const;

    const std::unordered_map<std::string_view, const Bus*>& GetAllBuses() const;

    const std::unordered_map<std::string_view, const Stop*>& GetAllStops() const;

private:

    std::deque<Bus> buses_;
    std::deque<Stop> stops_;
    std::unordered_map<std::string_view, const Bus*> index_buses_;
    std::unordered_map<std::string_view, const Stop*> index_stops_;
    std::unordered_map<const Stop*, std::set<const Bus*, detail::CompareBuses>> index_buses_through_stop_;
    std::unordered_map<std::pair<const Stop*, const Stop*>, size_t, detail::PairHash> index_distances_between_stops_;
};

} // namespace transport_catalogue
