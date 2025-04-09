#pragma once

#include <expected>
#include <string>
#include <vector>

#include <QMLExpression/expression.hpp>

#include "ilogger.hpp"
#include "imodel.hpp"
#include "information_state.hpp"
#include "gsv-logger/logger_utils.hpp"

namespace iif_sadaf::talk::GSV {

std::expected<bool, std::string> consistent(const QMLExpression::Expression& expr, const InformationState& state, const IModel& model, GSVLogger* logger = nullptr, bool log_details = false);
std::expected<bool, std::string> allows(const InformationState& state, const QMLExpression::Expression& expr, const IModel& model, GSVLogger* logger = nullptr, bool log_details = false);
std::expected<bool, std::string> supports(const InformationState& state, const QMLExpression::Expression& expr, const IModel& model, GSVLogger* logger = nullptr, bool log_details = false);
std::expected<bool, std::string> isSupportedBy(const QMLExpression::Expression& expr, const InformationState& state, const IModel& model, GSVLogger* logger = nullptr, bool log_details = false);

std::expected<bool, std::string> consistent(const QMLExpression::Expression& expr, const IModel& model, GSVLogger* logger = nullptr, bool log_details = false);
std::expected<bool, std::string> coherent(const QMLExpression::Expression& expr, const IModel& model, GSVLogger* logger = nullptr, bool log_details = false);
std::expected<bool, std::string> entails(const std::vector<QMLExpression::Expression>& premises, const QMLExpression::Expression& conclusion, const IModel& model, GSVLogger* logger = nullptr, bool log_details = false);
std::expected<bool, std::string> entails_0(const std::vector<QMLExpression::Expression>& premises, const QMLExpression::Expression& conclusion, const IModel& model, GSVLogger* logger = nullptr, bool log_details = false);
std::expected<bool, std::string> entails_G(const std::vector<QMLExpression::Expression>& premises, const QMLExpression::Expression& conclusion, const IModel& model, GSVLogger* logger = nullptr, bool log_details = false);
std::expected<bool, std::string> entails_C(const std::vector<QMLExpression::Expression>& premises, const QMLExpression::Expression& conclusion, const IModel& model, GSVLogger* logger = nullptr, bool log_details = false);
std::expected<bool, std::string> equivalent(const QMLExpression::Expression& expr1, const QMLExpression::Expression& expr2, const IModel& model, GSVLogger* logger = nullptr, bool log_details = false);

}
