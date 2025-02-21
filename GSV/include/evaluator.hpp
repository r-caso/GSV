#pragma once

#include "expression.hpp"
#include "information_state.hpp"

namespace iif_sadaf::talk::GSV {

/**
 * @brief Represents an evaluator for logical expressions.
 *
 * The Evaluator struct applies logical operations on `InformationState` objects
 * using the visitor pattern. It also takes an IModel object as input. It evaluates 
 * different types of logical expressions, including unary, binary, quantification,
 * identity, and predication nodes. The evaluation modifies or filters the given
 * `InformationState` based on the logical rules applied.
 * 
 * Due to the way `std::visit` is implemented in C++, the input `InformationState`
 * and `IModel*` must be wrapped in a `std::variant`.
 */
struct Evaluator {
    InformationState operator()(std::shared_ptr<UnaryNode> expr, std::variant<std::pair<InformationState, const IModel*>> params) const;
    InformationState operator()(std::shared_ptr<BinaryNode> expr, std::variant<std::pair<InformationState, const IModel*>> params) const;
    InformationState operator()(std::shared_ptr<QuantificationNode> expr, std::variant<std::pair<InformationState, const IModel*>> params) const;
    InformationState operator()(std::shared_ptr<IdentityNode> expr, std::variant<std::pair<InformationState, const IModel*>> params) const;
    InformationState operator()(std::shared_ptr<PredicationNode> expr, std::variant<std::pair<InformationState, const IModel*>> params) const;
};

}
