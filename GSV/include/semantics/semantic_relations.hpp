#pragma once

#include <string>

#include "information_state.hpp"
#include "model.hpp"
#include "possibility.hpp"

namespace iif_sadaf::talk::GSV {

int termDenotation(std::string_view term, int world, const IModel& model);
const std::set<std::vector<int>>& predicateDenotation(std::string_view predicate, int world, const IModel& model);
int variableDenotation(std::string_view variable, const Possibility& possibility);

}