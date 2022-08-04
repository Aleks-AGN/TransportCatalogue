#include "stat_reader.h"
#include "domain.h"

#include <iomanip>

namespace transport_catalogue {

namespace stat_queries_utils {

using namespace std::string_literals;

void PrintQueriesData(TransportCatalogue& catalogue, std::vector<std::string>& queries, std::ostream& os) {
    
    for (std::string& query : queries) {
        std::string delim = " "s;
        size_t pos = query.find(delim);
        std::string query_type = query.substr(0, pos);
        std::string query_data = query.substr(pos + delim.length());

        if (query_type == "Bus"s) {
            const Bus* bus = catalogue.FindBus(query_data);
            if (bus != nullptr)	{
                auto [route_stops_count, unique_stops_count, route_length, curvature] = catalogue.GetRouteInfo(bus);
                os << std::setprecision(6) << "Bus "s << query_data << ": "s
                          << route_stops_count << " stops on route, "s
                          << unique_stops_count << " unique stops, "s
                          << route_length << " route length, "s
                          << curvature << " curvature"s << std::endl;
            } else {
                os << "Bus "s << query_data << ": not found"s << std::endl;
            }
        } else if (query_type == "Stop"s) {
            const Stop* stop = catalogue.FindStop(query_data);
            if (stop != nullptr) {
                auto buses_through_stop = catalogue.GetBusesThroughStop(stop);
                if (buses_through_stop != nullptr) {
                    os << "Stop "s << query_data << ": buses"s;
                    for (const auto& bus : *buses_through_stop) {
                        os << " "s << bus->number;
                    }
                    os << std::endl;
                } else {
                    os << "Stop " << query_data << ": no buses" << std::endl;
                }
            } else {
                os << "Stop "s << query_data << ": not found"s << std::endl;
            }
        }
    }
}

void ReadTransportCatalogue(TransportCatalogue& catalogue, std::istream& is, std::ostream& os) {
    std::vector<std::string> queries;
    
    std::string line;
    std::getline(is, line);    
    size_t queries_count = std::stoul(line);
    
    queries.reserve(queries_count);

    for (size_t i = 0; i < queries_count; ++i) {
        std::getline(is, line);
        queries.emplace_back(std::move(line));
    }

    PrintQueriesData(catalogue, queries, os);
}

} // namespace stat_queries_utils

} // namespace transport_catalogue
