#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"

namespace transport_catalogue {
    
// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
class RequestHandler {
public:
    RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer)
        : db_(db) , renderer_(renderer) {
    }

    const TransportCatalogue& GetTransportCatalogue() const;

    svg::Document RenderMap() const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};

} // namespace transport_catalogue
