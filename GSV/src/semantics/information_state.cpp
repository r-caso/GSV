#include "information_state.hpp"

#include <algorithm>
#include <iostream>
#include <memory>

namespace iif_sadaf::talk::GSV {

/**
 * @brief Constructs an InformationState based on a given model.
 *
 * Initializes the information state and optionally populates it with possibilities.
 *
 * @param model The model to which this information state belongs.
 * @param create_possibilities Whether to initialize the possibilities set.
 */
InformationState::InformationState(const IModel& model, bool create_possibilities)
	: possibilities()
	, model(model)
{
	auto r_system = std::make_shared<ReferentSystem>();

	if (!create_possibilities) {
		return;
	}

	const int number_of_worlds = model.world_cardinality();
	for (int i = 0; i < number_of_worlds; ++i) {
		possibilities.emplace(r_system, i);
	}
}

/**
 * @brief Checks if the information state is empty.
 *
 * @return True if there are no possibilities, false otherwise.
 */
bool InformationState::empty() const
{
	return possibilities.empty();
}

/**
 * @brief Clears all possibilities from the information state.
 */
void InformationState::clear()
{
	possibilities.clear();
}

/**
 * @brief Returns an iterator to the beginning of the possibilities set.
 *
 * @return Iterator to the beginning of the possibilities.
 */
std::set<Possibility>::iterator InformationState::begin() 
{
	return possibilities.begin();
}

/**
 * @brief Returns an iterator to the end of the possibilities set.
 *
 * @return Iterator to the end of the possibilities.
 */
std::set<Possibility>::iterator InformationState::end() 
{
	return possibilities.end();
}

/**
 * @brief Removes a possibility from the set.
 *
 * @param it Iterator pointing to the possibility to erase.
 * @return Iterator following the last removed element.
 */
std::set<Possibility>::iterator InformationState::erase(std::set<Possibility>::iterator it)
{
	return possibilities.erase(it);
}


/**
 * @brief Checks if a given possibility is present in the information state.
 *
 * @param p The possibility to check.
 * @return True if the possibility is present, false otherwise.
 */
bool InformationState::contains(const Possibility& p) const
{
	return possibilities.contains(p);
}

/*
* NON-MEMBER INTERFACE FUNCTIONS
*/


/**
 * @brief Updates the information state with a new variable-individual assignment.
 *
 * Creates a new information state where each possibility has been updated with the
 * given variable-individual assignment.
 *
 * @param input_state The original information state.
 * @param variable The variable to be added or updated.
 * @param individual The individual assigned to the variable.
 * @return A new updated information state.
 */
InformationState update(const InformationState& input_state, std::string_view variable, int individual)
{
	InformationState output_state(input_state.model, false);

	auto r_star = std::make_shared<ReferentSystem>();

	for (const auto& p : input_state.possibilities) {
		Possibility p_star(r_star, p.world);
		p_star.assignment = p.assignment;
		r_star->pegs = p.referentSystem->pegs;
		for (const auto& map : p.referentSystem->variablePegAssociation) {
			auto var = map.first;
			int peg = map.second;
			r_star->variablePegAssociation[var] = peg;
		}

		p_star.update(variable, individual);

		output_state.possibilities.insert(p_star);
	}

	return output_state;
}

/**
 * @brief Determines if one information state extends another.
 *
 * Checks whether every possibility in s2 extends at least one possibility in s1.
 *
 * @param s2 The potentially extending information state.
 * @param s1 The base information state.
 * @return True if s2 extends s1, false otherwise.
 */
bool extends(const InformationState& s2, const InformationState& s1)
{
	const auto extends_possibility_in_s1 = [&](const Possibility& p2) -> bool {
		const auto is_extended_by_p2 = [&](const Possibility& p1) -> bool {
			return extends(p2, p1); 
		};
		return std::ranges::any_of(s1.possibilities, is_extended_by_p2);
	};

	return std::ranges::all_of(s2.possibilities, extends_possibility_in_s1);
}

/*
* NON-INTERFACE FUNCTIONS
*/

std::string str(const InformationState& state)
{
	std::string desc;

	desc += "--------------------\n";
	for (const Possibility& p : state.possibilities) {
		desc += str(p);
		desc += "--------------------\n";
	}

	desc.pop_back();

	return desc;
}

/**
 * @brief Determines if one possibility is a descendant of another within an information state.
 *
 * A possibility p2 is a descendant of p1 if it extends p1 and is contained in the given information state.
 *
 * @param p2 The potential descendant possibility.
 * @param p1 The potential ancestor possibility.
 * @param s The information state in which the relationship is checked.
 * @return True if p2 is a descendant of p1 in s, false otherwise.
 */
bool isDescendantOf(const Possibility& p2, const Possibility& p1, const InformationState& s)
{
	return s.possibilities.contains(p2) && (extends(p2, p1));
}

/**
 * @brief Checks if a possibility subsists in an information state.
 *
 * A possibility subsists in an information state if at least one of its descendants exists within the state.
 *
 * @param p The possibility to check.
 * @param s The information state.
 * @return True if p subsists in s, false otherwise.
 */
bool subsistsIn(const Possibility& p, const InformationState& s)
{
	const auto is_descendant_of_p_in_s = [&](const Possibility& p1) -> bool { return isDescendantOf(p1, p, s); };
	return std::ranges::any_of(s.possibilities, is_descendant_of_p_in_s);
}

/**
 * @brief Checks if an information state subsists within another.
 *
 * An information state s1 subsists in s2 if all possibilities in s1 have corresponding possibilities in s2.
 *
 * @param s1 The potential subsisting state.
 * @param s2 The state in which s1 may subsist.
 * @return True if s1 subsists in s2, false otherwise.
 */
bool subsistsIn(const InformationState& s1, const InformationState& s2)
{
	const auto subsists_in_s2 = [&](const Possibility& p) -> bool { return subsistsIn(p, s2); };
	return std::ranges::all_of(s1.possibilities, subsists_in_s2);
}

}