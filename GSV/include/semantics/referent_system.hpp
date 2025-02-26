#pragma once

#include <set>
#include <string>
#include <string_view>
#include <unordered_map>

namespace iif_sadaf::talk::GSV {

/**
 * @brief Represents a referent system for variable assignments.
 *
 * The ReferentSystem class maintains associations between variables and integer pegs.
 */
struct ReferentSystem {
public:
	ReferentSystem() = default;
    ReferentSystem(const ReferentSystem& other) = default;
    ReferentSystem& operator=(const ReferentSystem& other) = default;
    ReferentSystem(ReferentSystem&& other) noexcept;
    ReferentSystem& operator=(ReferentSystem&& other) noexcept;

	int value(std::string_view variable) const;

	int pegs = 0;
	std::unordered_map<std::string_view, int> variablePegAssociation = {};
};

std::set<std::string_view> domain(const ReferentSystem& r);
bool extends(const ReferentSystem& r2, const ReferentSystem& r1);
std::string str(const ReferentSystem& r);

}
