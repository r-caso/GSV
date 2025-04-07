#pragma once

#include "gsv-logger/null_logger.hpp"

namespace iif_sadaf::talk::GSV {

inline GSVLogger* normalize(GSVLogger* logger) {
    return logger ? logger : &NullLoggerInstance;
}

}
