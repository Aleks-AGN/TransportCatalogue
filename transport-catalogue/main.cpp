#include "transport_catalogue.h"
#include "json_reader.h"
#include "json.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "serialization.h"

//#include "input_reader.h"
//#include "stat_reader.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    using namespace transport_catalogue;

    const json::Document input_doc = json::Load(std::cin);
    //json::Print(input_doc, cout);

    JsonReader json_reader(input_doc);

    TransportCatalogue db;
    renderer::MapRenderer renderer;
    router::Router router(db);

    serialization::Serialization serialization(db, renderer, router, json_reader.GetSerializationSettings());

    if (mode == "make_base"sv) {

        json_reader.UpdateTransportCatalogue(db);
        
        json_reader.UpdateMapRenderer(renderer);
        
        json_reader.UpdateRouter(router);

        router.BuildGraph(db);

        serialization.SerializeDataBase();

    } else if (mode == "process_requests"sv) {

        serialization.DeserializeDataBase();
        
        RequestHandler request_handler(db, renderer, router);
        
        auto response = json_reader.ProcessStatRequests(request_handler);

        json::Print(Document{std::move(response)}, std::cout);

    } else {
        PrintUsage();
        return 1;
    }
}
