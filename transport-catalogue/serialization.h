#pragma once

#include "domain.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <filesystem>
#include <memory>

#include <transport_catalogue.pb.h>
//#include "build/transport_catalogue.pb.h"
//#include "../build/transport_catalogue.pb.h"

namespace serialization {

using namespace transport_catalogue;

class Serialization {
public:
    using Path = std::filesystem::path;

    Serialization(TransportCatalogue& db, renderer::MapRenderer& map_renderer, router::Router& router, const Path& path);

    void SerializeDataBase();

    void DeserializeDataBase();

private:
    Path path_;
    TransportCatalogue& db_;
    renderer::MapRenderer& map_renderer_;
    router::Router& router_;
    transport_catalogue_serialize::DataBase data_base_;
    
    transport_catalogue_serialize::Stop SerializeStop(const domain::Stop& stop);

    void SerializeStops();

    transport_catalogue_serialize::Distance SerializeDistance(const domain::Stop* from, const domain::Stop* to, size_t distance);

    void SerializeDistances();

    transport_catalogue_serialize::Bus SerializeBus(const domain::Bus& bus);

    void SerializeBuses();
    
    void SerializeTransportCatalogue();

    void DeserializeStop(const transport_catalogue_serialize::Stop& stop);

    void DeserializeStops();

    void DeserializeTransportCatalogue();
    
    void DeserializeDistance(const transport_catalogue_serialize::Distance& distance);

    void DeserializeDistances();

    void DeserializeBus(const transport_catalogue_serialize::Bus& bus);

    void DeserializeBuses();

    renderer_serialize::Color SetSerialColor(const svg::Color& color);

    void SerializeMapRenderer();

    svg::Color SetDeserialColor(const renderer_serialize::Color& color);

    void DeserializeMapRenderer();

    void SerializeRoutingSettings();
    
    void SerializeGraph();

    void SerializeStopIds();
    
    void SerializeRouter();

    void DeserializeRoutingSettings();

    void DeserializeGraph();

    void DeserializeStopIds();

    void DeserializeRouter();
};

} // namespace serialization
