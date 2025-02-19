#pragma once

#include <set>
#include <string>
#include <string_view>

#include "model.hpp"
#include "possibility.hpp"

namespace iif_sadaf::talk::GSV {

/**
 * @brief Represents an information state based on a given model.
 *
 * The InformationState class maintains a set of possibilities
 * and provides operations to manage them.
 */
struct InformationState {
public:
	InformationState(const IModel& model, bool create_possibilities = true);

	InformationState(const InformationState& other)
		: possibilities(other.possibilities),
		model(other.model)
	{ }

	InformationState(InformationState&& other) noexcept
		: possibilities(std::move(other.possibilities))
		, model(other.model)
	{ }

	InformationState& operator=(const InformationState&) = delete;
	InformationState& operator=(InformationState&&) = delete;

	bool empty() const;
	void clear();

	std::set<Possibility>::iterator begin();
	std::set<Possibility>::iterator end();
	std::set<Possibility>::iterator erase(std::set<Possibility>::iterator it);
	bool contains(const Possibility& p) const;

	std::set<Possibility> possibilities = {};
	const IModel& model;
};

InformationState update(const InformationState& input_state, std::string_view variable, int individual);
bool extends(const InformationState& s2, const InformationState& s1);

std::string str(const InformationState& state);

bool isDescendantOf(const Possibility& p2, const Possibility& p1, const InformationState& s);
bool subsistsIn(const Possibility& p, const InformationState& s);
bool subsistsIn(const InformationState& s1, const InformationState& s2);

}
