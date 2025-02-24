#include "possibility.hpp"

#include <algorithm>

namespace iif_sadaf::talk::GSV {

Possibility::Possibility(std::shared_ptr<ReferentSystem> r_system, int world)
    : referentSystem(std::move(r_system))
	, assignment({})
	, world(world)
{ }

Possibility::Possibility(Possibility&& other) noexcept
    : referentSystem(std::move(other.referentSystem))
    , assignment(std::move(other.assignment))
	, world(other.world)
{ }

Possibility& Possibility::operator=(Possibility&& other) noexcept
{
	if (this != &other) {
		this->referentSystem = std::move(other.referentSystem);
        this->assignment.clear();
        this->assignment.swap(other.assignment);
		this->world = other.world;
	}
	return *this;
}

/**
* @brief Updates the assignment of a variable to an individual.
*
* The variable is first added to or updated in the associated referent system.
* Then, the assignment is modified to map the peg of the variable to the new individual.
*
* @param variable The variable to update.
* @param individual The new individual assigned to the variable.
*/
void Possibility::update(std::string_view variable, int individual)
{
	referentSystem->variablePegAssociation[variable] = ++(referentSystem->pegs);
	assignment[referentSystem->pegs] = individual;
}

/*
* NON-MEMBER FUNCTIONS
*/

/**
 * @brief Determines whether one Possibility extends another.
 *
 * A Possibility `p2` extends `p1` if:
 * - They have the same world.
 * - Every peg mapped in `p1` has the same individual in `p2`.
 *
 * @param p2 The potential extending Possibility.
 * @param p1 The base Possibility.
 * @return True if `p2` extends `p1`, false otherwise.
 */
bool extends(const Possibility& p2, const Possibility& p1)
{
	const auto peg_is_new_or_maintains_assignment = [&](const std::pair<int, int>& map) -> bool {
		int peg = map.first;
		int individual = map.second;

		return !p1.assignment.contains(peg) || (p1.assignment.at(peg) == p2.assignment.at(peg));
	};

	return (p1.world == p2.world) && std::ranges::all_of(p2.assignment, peg_is_new_or_maintains_assignment);
}

bool operator<(const Possibility& p1, const Possibility& p2)
{
	return p1.world < p2.world;
}

std::string str(const Possibility& p)
{
	std::string desc = "[ ] Referent System:\n" + str(*p.referentSystem);
	desc += "[ ] Assignment function: \n";

	if (p.assignment.empty()) {
		desc += "  [ empty ]\n";
		
	}
	else {
		for (const auto& [peg, individual] : p.assignment) {
			desc += "  - peg_" + std::to_string(peg) + " -> e_" + std::to_string(individual) + "\n";
		}
	}

	desc += "[ ] Possible world: w_" + std::to_string(p.world) + "\n";

	return desc;
}

}