#include "request_handler.h"
#include "svg.h"

#include <unordered_map>


namespace transport_catalogue {
    
const TransportCatalogue& RequestHandler::GetTransportCatalogue() const {
    return db_;
}

svg::Document RequestHandler::RenderMap() const {
    return renderer_.GetSvgDocument(db_.GetAllBuses());
}

std::optional<graph::Router<double>::RouteInfo> RequestHandler::BuildRoute(const Stop* from_stop, const Stop* to_stop) const {
    return router_.GetRouteInfo(from_stop, to_stop);
}

json::Array RequestHandler::GetEdgesItems(const std::vector<graph::EdgeId>& edges) const {
    return router_.GetEdgesInfo(edges);
}
    
} // namespace transport_catalogue
