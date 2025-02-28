#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "referent_system.hpp"

namespace iif_sadaf::talk::GSV {

/**
 * @brief Represents a possibility as understood in the underlying semantics.
 *
 * The `Possibility` class models possiblities in the GSV framework, 
 * which are defined as tuples of a referent system, an assignment if
 * individuals to pegs, and a possible world index.
 *
 * This class supports copy and move semantics, allowing for efficient
 * duplication and transfer of instances.
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
