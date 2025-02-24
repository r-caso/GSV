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
    Possibility(const Possibility& other) = default;
    Possibility& operator=(const Possibility& other) = default;
	Possibility(Possibility&& other) noexcept;
	Possibility& operator=(Possibility&& other) noexcept;

	void update(std::string_view variable, int individual);
	
	std::shared_ptr<ReferentSystem> referentSystem;
	std::unordered_map<int, int> assignment;
	int world;
};

bool extends(const Possibility& p2, const Possibility& p1);
bool operator<(const Possibility& p1, const Possibility& p2);

std::string str(const Possibility& p);

}