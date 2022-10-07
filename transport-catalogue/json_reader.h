#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_router.h"

#include <filesystem>

namespace transport_catalogue {

using namespace json;

class JsonReader {
public:
    using Path = std::filesystem::path;

    JsonReader(Document input_doc)
        : input_doc_(std::move(input_doc)) {
    }

    void UpdateTransportCatalogue(TransportCatalogue& db) const;

    void UpdateMapRenderer(renderer::MapRenderer& renderer) const;
    
    void UpdateRouter(router::Router& router) const;

    Path GetSerializationSettings() const;

    Node ProcessStatRequests(RequestHandler& request_handler) const;

    Node ProcessStatRequests(TransportCatalogue& db) const;

private:
    Document input_doc_;

    std::pair<Stop, bool> ParseStopRequest(const Dict& request) const;

    void AddBuses(TransportCatalogue& db, const Array& base_requests,
        const std::vector<int> bus_request_ids) const;

    void AddRoadDistances(TransportCatalogue& db, const Array& base_requests,
        const std::vector<int> stop_with_distances_request_ids) const;
};

} // namespace transport_catalogue
