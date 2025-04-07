#pragma once

#include <string>

namespace iif_sadaf::talk::GSV {

class GSVLogger {
public:
	virtual void log(const std::string& message) const = 0;
	
	virtual void increaseDepth() {}
	virtual void decreaseDepth() {}
	virtual std::string currentIndent() const { return {}; }

	virtual ~GSVLogger() = default;
};

}