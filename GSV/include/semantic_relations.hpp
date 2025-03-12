#pragma once

#include <expected>
#include <string>
#include <vector>

#include "expression.hpp"
#include "information_state.hpp"
#include "imodel.hpp"

namespace iif_sadaf::talk::GSV {

std::expected<bool, std::string> consistent(const Expression& expr, const InformationState& state, const IModel& model);
std::expected<bool, std::string> allows(const InformationState& state, const Expression& expr, const IModel& model);
std::expected<bool, std::string> supports(const InformationState& state, const Expression& expr, const IModel& model);
std::expected<bool, std::string> isSupportedBy(const Expression& expr, const InformationState& state, const IModel& model);

std::expected<bool, std::string> consistent(const Expression& expr, const IModel& model);
std::expected<bool, std::string> coherent(const Expression& expr, const IModel& model);
std::expected<bool, std::string> entails(const std::vector<Expression>& premises, const Expression& conclusion, const IModel& model);
std::expected<bool, std::string> equivalent(const Expression& expr1, const Expression& expr2, const IModel& model);

}
