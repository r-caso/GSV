#include "referent_system.hpp"

#include <algorithm>
#include <stdexcept>

namespace iif_sadaf::talk::GSV {

/**
 * @brief Returns the range (number of pegs) of the ReferentSystem.
 * 
 * Since the range of a referent system is always an initial segment
 * of the natural numbers, the number of pegs represents the range
 * of the referent system. 
 *
 * @return The number of pegs in the referent system.
 */
int ReferentSystem::range() const
{
	return pegs;
}

/**
 * @brief Returns the domain (set of variables) of the ReferentSystem.
 *
 * @return A set of string views representing the variables in the referent
 * system.
 */
std::set<std::string_view> ReferentSystem::domain() const
{
	std::set<std::string_view> domain;
	for (const auto& [variable, peg] : variablePegAssociation) {
		domain.insert(variable);
	}

	return domain;
}

/**
 * @brief Retrieves the peg value associated with a given variable.
 *
 * @param variable The variable whose peg value is to be retrieved.
 * @return The peg value associated with the variable.
 * @throws std::out_of_range If the variable has no associated peg.
 */
int ReferentSystem::value(std::string_view variable) const
{
	if (!variablePegAssociation.contains(variable)) {
		std::string error_msg = "Variable " + std::string(variable) + " has no anaphoric antecedent of binding quantifier";
		throw(std::out_of_range(error_msg));
	}

	return variablePegAssociation.at(variable);
}

void ReferentSystem::update(std::string_view variable)
{
	variablePegAssociation[variable] = ++pegs;
}

/**
 * @brief Represents a referent system for variable assignments.
 *
 * The ReferentSystem class maintains associations between variables and integer pegs.
 */
std::string str(const ReferentSystem& r)
{
	std::string desc = "Number of pegs: " + std::to_string(r.pegs) + "\n";
	desc += "Variable to peg association:\n";

	if (r.variablePegAssociation.empty()) {
		desc += "  [ empty ]\n";
		return desc;
	}

	for (const auto& [variable, peg] : r.variablePegAssociation) {
		desc += "  - " + std::string(variable) + " -> peg_" + std::to_string(peg) + "\n";
	}

	return desc;
}

/**
 * @brief Determines whether one ReferentSystem extends another.
 *
 * This function checks whether the referent system `r2` extends the referent system `r1`.
 * A referent system `r2` extends `r1` if:
 * - The range of `r1` is a subset of the range of `r2`.
 * - The domain of `r1` is a subset of the domain of `r2`.
 * - Variables in `r1` retain their values in `r2`, or their values are new relative to `r1`.
 * - New variables in `r2` have new values relative to `r1`.
 *
 * @param r2 The potential extending ReferentSystem.
 * @param r1 The base ReferentSystem.
 * @return True if `r2` extends `r1`, false otherwise.
 */
bool extends(const ReferentSystem& r2, const ReferentSystem& r1)
{
	if (r1.range() > r2.range()) {
		return false;
	}

	std::set<std::string_view> domain_r1 = r1.domain();
	std::set<std::string_view> domain_r2 = r2.domain();

	if (!std::ranges::includes(domain_r2, domain_r1)) {
		return false;
	}

	const auto old_var_same_or_new_peg = [&](std::string_view variable) -> bool {
		return r1.value(variable) == r2.value(variable) || r1.range() <= r2.value(variable);
	};

	if (!std::ranges::all_of(domain_r1, old_var_same_or_new_peg)) {
		return false;
	}

	const auto new_var_new_peg = [&](std::string_view variable) -> bool {
		return domain_r1.contains(variable) || r1.range() <= r2.value(variable);
	};

	return std::ranges::all_of(domain_r2, new_var_new_peg);
}

}