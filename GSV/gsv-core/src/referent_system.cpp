#include "referent_system.hpp"

#include <algorithm>
#include <format>
#include <stdexcept>

namespace iif_sadaf::talk::GSV {

/**
 * @brief Retrieves the set of variables in the referent system.
 * 
 * This function extracts all variables present in the given 
 * ReferentSystem instance and returns them as a set of std::string_view's.
 * 
 * @param r The ReferentSystem instance whose variables are being queried.
 * @return std::set<std::string_view> A set containing all variables in the system.
 */
std::set<std::string_view> domain(const ReferentSystem& r)
{
	std::set<std::string_view> domain;
	for (const auto& [variable, peg] : r.variablePegAssociation) {
		domain.insert(variable);
	}

	return domain;
}

ReferentSystem::ReferentSystem(ReferentSystem&& other) noexcept
    : pegs(other.pegs)
    , variablePegAssociation(std::move(other.variablePegAssociation))
{ }

ReferentSystem& ReferentSystem::operator=(ReferentSystem&& other) noexcept
{
    if (this != &other) {
        this->pegs = other.pegs;
        this->variablePegAssociation = std::move(other.variablePegAssociation);
        other.pegs = 0;
    }
    return *this;
}

/**
 * @brief Retrieves the referent value associated with a given variable.
 *
 * This function checks whether the specified variable exists in the referent system.
 * If the variable is found, its corresponding value is returned. Otherwise, an error
 * message is returned indicating that the variable is not present.
 *
 * @param variable The variable whose referent value is being queried.
 * @return std::expected<int, std::string> The value associated with the variable,
 *         or an error message if the variable does not exist.
 */
std::expected<int, std::string> ReferentSystem::value(std::string_view variable) const
{
	if (!variablePegAssociation.contains(variable)) {
        return std::unexpected(std::format("Referent system does not contain variable {}", std::string(variable)));
	}

	return variablePegAssociation.at(variable);
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
	// TODO check that these calls to value() are safe
	if (r1.pegs > r2.pegs) {
		return false;
	}

	std::set<std::string_view> domain_r1 = domain(r1);
	std::set<std::string_view> domain_r2 = domain(r2);

	if (!std::ranges::includes(domain_r2, domain_r1)) {
		return false;
	}

	const auto old_var_same_or_new_peg = [&](std::string_view variable) -> bool {
		return r1.value(variable).value() == r2.value(variable).value() || r1.pegs <= r2.value(variable).value();
	};

	if (!std::ranges::all_of(domain_r1, old_var_same_or_new_peg)) {
		return false;
	}

	const auto new_var_new_peg = [&](std::string_view variable) -> bool {
		return domain_r1.contains(variable) || r1.pegs <= r2.value(variable).value();
	};

	return std::ranges::all_of(domain_r2, new_var_new_peg);
}

std::string str(const ReferentSystem& r)
{
	if (r.variablePegAssociation.empty()) {
		return "{ }";
	}

	std::string vp_association;

	for (const auto& [variable, peg] : r.variablePegAssociation) {
		vp_association += std::format("{} -> peg{}, ", std::string(variable), std::to_string(peg));
	}

	vp_association.resize(vp_association.size() - 2);

	return std::format("{{ {} }}", vp_association);
}

}
