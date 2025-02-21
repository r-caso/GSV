#pragma once

#include <set>
#include <string>
#include <string_view>

#include "model.hpp"
#include "possibility.hpp"

namespace iif_sadaf::talk::GSV {

/**
 * @brief InformationState alias for `std::set<Possibility>`
 *
 * Represents an information state, defined as a set of possiblities.
 */
using InformationState = std::set<Possibility>;

InformationState create(const Model& model);
InformationState update(const InformationState& input_state, std::string_view variable, int individual);
bool extends(const InformationState& s2, const InformationState& s1);

bool isDescendantOf(const Possibility& p2, const Possibility& p1, const InformationState& s);
bool subsistsIn(const Possibility& p, const InformationState& s);
bool subsistsIn(const InformationState& s1, const InformationState& s2);

std::string str(const InformationState& state);
}
