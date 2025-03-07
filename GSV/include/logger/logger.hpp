#pragma once

#include <memory>
#include <string>

#include <spdlog/spdlog.h>
#include "spdlog/sinks/stdout_color_sinks.h"

namespace iif_sadaf::talk::GSV {

class Logger {
public:
	static std::shared_ptr<spdlog::logger>& instance()
	{
		if (!s_GSVLogger) {
			s_GSVLogger = spdlog::stdout_color_mt("GSV Evaluator");
			s_GSVLogger->set_level(spdlog::level::trace);
			s_GSVLogger->set_pattern("%^[%T] %n: %v%$");
		}

        return s_GSVLogger;
    }

private:
	Logger() = default;
	Logger(const Logger& other) = default;
	Logger& operator=(const Logger& other) = default;
	~Logger() = default;

	static inline std::shared_ptr<spdlog::logger> s_GSVLogger{ nullptr };
};

}

#define GSV_TRACE(...)    iif_sadaf::talk::GSV::Logger::instance()->trace(__VA_ARGS__)
#define GSV_INFO(...)     iif_sadaf::talk::GSV::Logger::instance()->info(__VA_ARGS__)
#define GSV_WARN(...)     iif_sadaf::talk::GSV::Logger::instance()->warn(__VA_ARGS__)
#define GSV_ERROR(...)    iif_sadaf::talk::GSV::Logger::instance()->error(__VA_ARGS__)
#define GSV_CRITICAL(...) iif_sadaf::talk::GSV::Logger::instance()->critical(__VA_ARGS__)

struct Formatter {
	std::string operator()(std::shared_ptr<UnaryNode> expr) const;
	std::string operator()(std::shared_ptr<BinaryNode> expr) const;
	std::string operator()(std::shared_ptr<QuantificationNode> expr) const;
	std::string operator()(std::shared_ptr<PredicationNode> expr) const;
	std::string operator()(std::shared_ptr<IdentityNode> expr) const;
};