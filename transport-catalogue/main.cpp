#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

int main() {

    using namespace transport_catalogue;
    
    TransportCatalogue catalogue;
    
    /*
    std::istringstream is;
    is.str(
        "13\n"s
        "Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino\n"s
        "Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino\n"s
        "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n"s
        "Bus 750: Tolstopaltsevo - Marushkino - Marushkino - Rasskazovka\n"s
        "Stop Rasskazovka: 55.632761, 37.333324, 9500m to Marushkino\n"s
        "Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam\n"s
        "Stop Biryusinka: 55.581065, 37.64839, 750m to Universam\n"s
        "Stop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, 900m to Biryulyovo Tovarnaya\n"s
        "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo Passazhirskaya\n"s
        "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to Biryulyovo Zapadnoye\n"s
        "Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye\n"s
        "Stop Rossoshanskaya ulitsa: 55.595579, 37.605757\n"s
        "Stop Prazhskaya: 55.611678, 37.603831\n"s 
    );
    input_queries_utils::UpdateTransportCatalogue(catalogue, is);
    */

    input_queries_utils::UpdateTransportCatalogue(catalogue, cin);

    /*
    is.clear();
    is.str(
        "6\n"s
        "Bus 256\n"s
        "Bus 750\n"s
        "Bus 751\n"s
        "Stop Samara\n"s
        "Stop Prazhskaya\n"s
        "Stop Biryulyovo Zapadnoye\n"s
    );
    stat_queries_utils::ReadTransportCatalogue(catalogue, is, cout);
    */

    stat_queries_utils::ReadTransportCatalogue(catalogue, cin, cout);

    return 0;
}
