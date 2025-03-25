#pragma once

#include <expected>

#include <QMLExpression/expression.hpp>

#include "information_state.hpp"

namespace iif_sadaf::talk::GSV {

/**
 * @brief Implements the GSV evaluation function for QML formulas
 *
 * The Evaluator struct applies logical operations on `InformationState` objects
 * using the visitor pattern. It also takes an IModel* as parameter.
 * 
 * It evaluates different types of logical expressions, including unary, binary,
 * quantification, identity, and predication nodes. The evaluation modifies or filters 
 * the given `InformationState`, based on the logical rules applied, and the semantic information provided by `IModel*`.
 * 
 * Due to the way `std::visit` is implemented in C++, the input `InformationState`
 * and `IModel*` must be wrapped in a `std::variant` and passed as a single argument.
 */
struct Evaluator {
    std::expected<InformationState, std::string> operator()(const std::shared_ptr<QMLExpression::UnaryNode>& expr, std::variant<std::pair<InformationState, const IModel*>> params) const;
    std::expected<InformationState, std::string> operator()(const std::shared_ptr<QMLExpression::BinaryNode>& expr, std::variant<std::pair<InformationState, const IModel*>> params) const;
    std::expected<InformationState, std::string> operator()(const std::shared_ptr<QMLExpression::QuantificationNode>& expr, std::variant<std::pair<InformationState, const IModel*>> params) const;
    std::expected<InformationState, std::string> operator()(const std::shared_ptr<QMLExpression::IdentityNode>& expr, std::variant<std::pair<InformationState, const IModel*>> params) const;
    std::expected<InformationState, std::string> operator()(const std::shared_ptr<QMLExpression::PredicationNode>& expr, std::variant<std::pair<InformationState, const IModel*>> params) const;
};

std::expected<InformationState, std::string> evaluate(const QMLExpression::Expression& expr, const InformationState& input_state, const IModel& model);

}
