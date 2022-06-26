#include "stat_reader.h"
#include <iostream>
#include <iomanip>
#include <vector>

namespace transport_catalogue {

namespace stat_queries_utils {

using namespace std::string_literals;

void ReadTransportCatalogue(TransportCatalogue& catalogue) {
    std::vector<std::string> queries;
    
    std::string line;
	std::getline(std::cin, line);    
    size_t queries_count = std::stoul(line);
    
    queries.reserve(queries_count);

	for (int i = 0; i < queries_count; ++i) {
        std::getline(std::cin, line);
        queries.emplace_back(std::move(line));
    }

    for (std::string& query : queries) {
        std::string delim = " "s;
		size_t pos = query.find(delim);
		std::string query_type = query.substr(0, pos);
		std::string query_data = query.substr(pos + delim.length());

		if (query_type == "Bus"s) {
			const Bus* bus = catalogue.FindBus(query_data);
			if (bus != nullptr)	{
				auto [route_stops_count, unique_stops_count, route_length, curvature] = catalogue.GetRouteInfo(bus);
				std::cout << std::setprecision(6) << "Bus "s << query_data << ": "s
						<< route_stops_count << " stops on route, "s
						<< unique_stops_count << " unique stops, "s
						<< route_length << " route length, "s
						<< curvature << " curvature"s << std::endl;
			} else {
				std::cout << "Bus "s << query_data << ": not found"s << std::endl;
			}
		} else if (query_type == "Stop"s) {
			const Stop* stop = catalogue.FindStop(query_data);
			if (stop != nullptr) {
				auto buses_through_stop = catalogue.GetBusesThroughStop(stop);
				if (buses_through_stop != nullptr) {
					std::cout << "Stop "s << query_data << ": buses"s;
					for (const auto& bus : *buses_through_stop) {
						std::cout << " "s << bus->number;
					}
					std::cout << std::endl;
				} else {
					std::cout << "Stop " << query_data << ": no buses" << std::endl;
				}
			} else {
				std::cout << "Stop "s << query_data << ": not found"s << std::endl;
			}
		}
    }
}

} // namespace stat_queries_utils

} // namespace transport_catalogue
