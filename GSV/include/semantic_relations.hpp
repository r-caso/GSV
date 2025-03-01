#pragma once

#include <vector>

#include "expression.hpp"
#include "information_state.hpp"
#include "imodel.hpp"

namespace iif_sadaf::talk::GSV {

bool consistent(const Expression& expr, const InformationState& state, const IModel& model);
bool allows(const InformationState& state, const Expression& expr, const IModel& model);
bool supports(const InformationState& state, const Expression& expr, const IModel& model);
bool isSupportedBy(const Expression& expr, const InformationState& state, const IModel& model);

bool consistent(const Expression& expr, const IModel& model);
bool coherent(const Expression& expr, const IModel& model);
bool entails(const std::vector<Expression>& premises, const Expression& conclusion, const IModel& model);
bool equivalent(const Expression& expr1, const Expression& expr2, const IModel& model);

}
