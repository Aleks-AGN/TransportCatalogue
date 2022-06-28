#pragma once

#include "transport_catalogue.h"
#include <iostream>
#include <vector>

namespace transport_catalogue {

namespace input_queries_utils {

void ParseStopQueries(TransportCatalogue& catalogue, std::vector<std::pair<std::string, std::string>>& stop_queries);

void ParseBusQueries(TransportCatalogue& catalogue, std::vector<std::string>& bus_queries);

void UpdateTransportCatalogue(TransportCatalogue& catalogue, std::istream& is);

} // namespace input_queries_utils

} // namespace transport_catalogue
