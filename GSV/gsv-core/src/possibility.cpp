#include "possibility.hpp"

#include <algorithm>
#include <format>

namespace iif_sadaf::talk::GSV {

Possibility::Possibility(std::shared_ptr<ReferentSystem> r_system, int world)
    : referentSystem(r_system)
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
        const int peg = map.first;

        return !p1.assignment.contains(peg) || (p1.assignment.at(peg) == p2.assignment.at(peg));
    };

    return (p1.world == p2.world) && std::ranges::all_of(p2.assignment, peg_is_new_or_maintains_assignment);
}

bool operator<(const Possibility& p1, const Possibility& p2)
{
    return p1.world < p2.world;
}

/**
 * @brief Retrieves the denotation of a variable within a given Possibility.
 *
 * This function looks up the peg associated with the given variable in the Possibility's ReferentSystem
 * and then retrieves the corresponding individual from the assignment.
 *
 * @param variable The name of the variable whose denotation is being retrieved.
 * @param p The Possibility in which the variable is interpreted.
 *
 * @return std::expected<int, std::string>
 * - If successful, returns the individual assigned to the variable.
 * - If the variable is not found in the ReferentSystem, returns an error message.
 * - If the peg retrieved from the ReferentSystem is not found in the assignment, returns an error message.
 */
std::expected<int, std::string> variableDenotation(std::string_view variable, const Possibility& p)
{
    const auto peg = p.referentSystem->value(variable);

    if (!peg.has_value()) {
        return std::unexpected(peg.error());
    }

    // Whenever variable exists in referent system, assignment is guaranteed to
    // contain the corresponding peg, so there is no need to check for existence
    // before returning
    return p.assignment.at(peg.value());
}

std::string str(const Possibility& p)
{
    std::string assignment_contents;

    if (p.assignment.empty()) {
        assignment_contents = "{ }";
    }
    else {
        for (const auto& [peg, individual] : p.assignment) {
            assignment_contents += std::format("peg{} -> e{}, ", std::to_string(peg), std::to_string(individual));
        }
        assignment_contents.resize(assignment_contents.size() - 2);
        assignment_contents = std::format("{{ {} }}", assignment_contents);
    }

    return std::format("[ R-System : {}, Assignment : {}, World : w{} ]",
        str(*p.referentSystem),
        assignment_contents,
        std::to_string(p.world)
    );
}

}
