#include "gsv-utils/error_reporting.hpp"

#include <format>

namespace iif_sadaf::talk::GSV {

std::string explain_failure(const std::string& formula, const std::string& cause)
{
	return std::format("In evaluating formula {}:\n{}", formula, cause);
}

}
