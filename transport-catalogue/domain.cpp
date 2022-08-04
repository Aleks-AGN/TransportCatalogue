#include "domain.h"

namespace domain {

size_t Stop::Hash() const {
    return std::hash<std::string>{}(name)
           + 37 * std::hash<double>{}(point.lng)
           + 37 * 37 * std::hash<double>{}(point.lat);
}

} // namespace domain