#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

int main() {

    using namespace transport_catalogue;
    
    TransportCatalogue catalogue;

	input_queries_utils::UpdateTransportCatalogue(catalogue);
    stat_queries_utils::ReadTransportCatalogue(catalogue);

    return 0;
}
