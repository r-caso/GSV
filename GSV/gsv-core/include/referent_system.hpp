#pragma once

#include <expected>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>

namespace iif_sadaf::talk::GSV {

/**
 * @brief Represents a referent system for variable assignments.
 *
 * The `ReferentSystem` class provides a framework for handling variable-to-integer 
 * mappings within GAV. It allows for retrieval of variable values and tracks the
 * number of pegs (or reference points) within the system.
 *
 * This class supports both copy and move semantics, ensuring flexibility in managing 
 * instances efficiently.
 */
struct ReferentSystem {
public:
	ReferentSystem() = default;
    ReferentSystem(const ReferentSystem& other) = default;
    ReferentSystem& operator=(const ReferentSystem& other) = default;
    ReferentSystem(ReferentSystem&& other) noexcept;
    ReferentSystem& operator=(ReferentSystem&& other) noexcept;

	std::expected<int, std::string> value(std::string_view variable) const;

	int pegs = 0;
	std::unordered_map<std::string_view, int> variablePegAssociation = {};
};

std::set<std::string_view> domain(const ReferentSystem& r);
bool extends(const ReferentSystem& r2, const ReferentSystem& r1);
std::string str(const ReferentSystem& r);
std::string repr(const ReferentSystem& r);

}
