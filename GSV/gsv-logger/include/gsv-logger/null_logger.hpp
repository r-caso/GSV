#pragma once

#include "ilogger.hpp"

namespace iif_sadaf::talk::GSV {

class NullLogger : public GSVLogger {
    void log(const std::string& message) const override {}
};

inline NullLogger NullLoggerInstance;

}
