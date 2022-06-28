#pragma once

#include "transport_catalogue.h"
#include <iostream>
#include <vector>

namespace transport_catalogue {

namespace stat_queries_utils {

void PrintQueriesData(TransportCatalogue& catalogue, std::vector<std::string>& queries, std::ostream& os);

void ReadTransportCatalogue(TransportCatalogue& catalogue, std::istream& is, std::ostream& os);

} // namespace stat_queries_utils

} // namespace transport_catalogue
