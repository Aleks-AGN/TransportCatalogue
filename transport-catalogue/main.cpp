#include "transport_catalogue.h"
#include "json_reader.h"
#include "json.h"
#include "request_handler.h"
#include "map_renderer.h"

//#include "input_reader.h"
//#include "stat_reader.h"

#include <iostream>
#include <sstream>
#include <string>

using namespace std;

int main() {
    
    using namespace transport_catalogue;

    const json::Document input_doc = json::Load(cin);
    //json::Print(input_doc, cout);

    JsonReader json_reader(input_doc);

    TransportCatalogue db;
    renderer::MapRenderer renderer;

    json_reader.UpdateTransportCatalogue(db);
    json_reader.UpdateMapRenderer(renderer);

    RequestHandler request_handler(db, renderer);
    //request_handler.RenderMap().Render(cout);

    auto response = json_reader.ProcessStatRequests(request_handler);

    json::Print(Document{std::move(response)}, cout);

    return 0;
}
