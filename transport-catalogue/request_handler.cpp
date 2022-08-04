#include "request_handler.h"
#include "svg.h"

#include <unordered_map>


namespace transport_catalogue {
    
svg::Document RequestHandler::RenderMap() const {
    return renderer_.GetSvgDocument(db_.GetAllBuses());
}

const TransportCatalogue& RequestHandler::GetTransportCatalogue() const {
    return db_;
}
    
} // namespace transport_catalogue
