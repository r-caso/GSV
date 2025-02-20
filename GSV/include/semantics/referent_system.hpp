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

	ReferentSystem(const ReferentSystem& other)
		: pegs(other.pegs)
		, variablePegAssociation(other.variablePegAssociation)
	{ }

	ReferentSystem& operator=(const ReferentSystem& other)
	{
		if (this != &other) {
			this->pegs = other.pegs;
			this->variablePegAssociation = other.variablePegAssociation;
		}

		return *this;
	}

	ReferentSystem(ReferentSystem&& other) noexcept
		: pegs(other.pegs)
		, variablePegAssociation(std::move(other.variablePegAssociation))
	{ }

	ReferentSystem& operator=(ReferentSystem&& other) noexcept
	{
		if (this != &other) {
			this->pegs = other.pegs;
			this->variablePegAssociation = std::move(other.variablePegAssociation);
			other.pegs = 0;
		}
		return *this;
	}

	int value(std::string_view variable) const;

	int pegs = 0;
	std::unordered_map<std::string_view, int> variablePegAssociation = {};
};

bool extends(const ReferentSystem& r2, const ReferentSystem& r1);
std::string str(const ReferentSystem& r);

}