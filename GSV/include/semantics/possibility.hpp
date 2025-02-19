#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "referent_system.hpp"

namespace iif_sadaf::talk::GSV {

/**
 * @brief Represents a possibility as understood in the underlying semantics.
 *
 * Possibilities are just tuples of a referent system, an assignment of individuals to pegs, and a possible world.
 * 
 * The class also contains a few convenience functions for handling the first two components.
 */
struct Possibility {
public:
	Possibility(std::shared_ptr<ReferentSystem> r_system, int world);
	
	Possibility(const Possibility& other)
		: referentSystem(other.referentSystem)
		, assignment(other.assignment)
		, world(other.world)
	{ }

	Possibility& operator=(const Possibility& other)
	{
		if (this != &other) {
			this->referentSystem = other.referentSystem;
			this->assignment = other.assignment;
			this->world = other.world;
		}

		return *this;
	}

	Possibility(Possibility&& other) noexcept
		: referentSystem(std::move(other.referentSystem))
		, assignment(std::move(other.assignment))
		, world(other.world)
	{ }

	Possibility& operator=(Possibility&& other) noexcept
	{
		if (this != &other) {
			this->referentSystem = std::move(other.referentSystem);
			this->assignment = std::move(other.assignment);
			this->world = other.world;
		}
		return *this;
	}

	int getAssignment(int peg) const;
	void update(std::string_view variable, int individual);
	
	std::shared_ptr<ReferentSystem> referentSystem;
	std::unordered_map<int, int> assignment;
	int world;
};

bool extends(const Possibility& p2, const Possibility& p1);
bool operator<(const Possibility& p1, const Possibility& p2);
std::string str(const Possibility& p);

}