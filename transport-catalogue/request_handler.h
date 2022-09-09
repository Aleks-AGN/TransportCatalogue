#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

namespace transport_catalogue {
    
// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
class RequestHandler {
public:
    RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer, const router::Router& router)
        : db_(db), renderer_(renderer), router_(router) {
    }

    const TransportCatalogue& GetTransportCatalogue() const;

    svg::Document RenderMap() const;

    std::optional<graph::Router<double>::RouteInfo> BuildRoute(const Stop* from_stop, const Stop* to_stop) const;

    json::Array GetEdgesItems(const std::vector<graph::EdgeId>& edges) const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник", "Визуализатор Карты" и "Маршрутизатор"
    const TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
    const router::Router& router_;
};

} // namespace transport_catalogue
