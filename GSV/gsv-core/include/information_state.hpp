#pragma once

#include <set>
#include <string>
#include <string_view>

#include "imodel.hpp"
#include "possibility.hpp"

namespace iif_sadaf::talk::GSV {

/**
 * @brief An alias for `std::set<Possibility>`
 */
using InformationState = std::set<Possibility>;

InformationState create(const IModel& model);
InformationState update(const InformationState& input_state, std::string_view variable, int individual);
bool extends(const InformationState& s2, const InformationState& s1);

bool isDescendantOf(const Possibility& p2, const Possibility& p1, const InformationState& s);
bool subsistsIn(const Possibility& p, const InformationState& s);
bool subsistsIn(const InformationState& s1, const InformationState& s2);

std::string str(const InformationState& state);
std::string repr(const InformationState& state);

}
