#include "evaluator.hpp"

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <ranges>

#include "semantic_relations.hpp"
#include "variable.hpp"

namespace iif_sadaf::talk::GSV {

namespace {
	void filter(InformationState& state, const std::function<bool(const Possibility&)>& predicate) {
		for (auto it = state.begin(); it != state.end(); ) {
			if (!predicate(*it)) {
				it = state.erase(it);
			}
			else {
				++it;
			}
		}
	}
}

/**
* @brief Evaluates a unary logical expression on an InformationState.
*
* Applies an operator (such as necessity, possibility, or negation) to
* modify the given state accordingly.
*
* @param expr The unary expression to evaluate.
* @param state The current information state.
* @return The modified InformationState after applying the operation.
* @throws std::invalid_argument if the operator is invalid.
*/
InformationState Evaluator::operator()(std::shared_ptr<UnaryNode> expr, std::variant<InformationState> state) const
{
	InformationState hypothetical = std::visit(Evaluator(), expr->scope, state);
	InformationState& s = std::get<InformationState>(state);

	if (expr->op == Operator::E_POS) {
		if (hypothetical.empty()) {
			s.clear();
		}
	}
	else if (expr->op == Operator::E_NEC) {
		if (!subsistsIn(s, hypothetical)) {
			s.clear();
		}
	}
	else if (expr->op == Operator::NEG) {
		filter(s, [&](const Possibility& p) -> bool { return !subsistsIn(p, hypothetical); });
	}
	else {
		throw(std::invalid_argument("Invalid operator for unary formula"));
	}

	return s;
}

/**
* @brief Evaluates a binary logical expression on an InformationState.
*
* Processes logical operations such as conjunction, disjunction, and
* implication, modifying the state accordingly.
*
* @param expr The binary expression to evaluate.
* @param state The current information state.
* @return The modified InformationState after applying the operation.
* @throws std::invalid_argument if the operator is invalid.
*/
InformationState Evaluator::operator()(std::shared_ptr<BinaryNode> expr, std::variant<InformationState> state) const
{
	if (expr->op == Operator::CON) {
		return std::visit(Evaluator(), expr->rhs, std::variant<InformationState>(std::visit(Evaluator(), expr->lhs, state)));
	}

	InformationState& s = std::get<InformationState>(state);
	InformationState hypothetical_lhs = std::visit(Evaluator(), expr->lhs, state);

	if (expr->op == Operator::DIS) {
		InformationState hypothetical_rhs = std::visit(Evaluator(), expr->rhs, std::variant<InformationState>(std::visit(Evaluator(), negate(expr->lhs), state)));

		const auto in_lhs_or_in_rhs = [&](const Possibility& p) -> bool {
			return hypothetical_lhs.contains(p) || hypothetical_rhs.contains(p);
		};

		filter(s, in_lhs_or_in_rhs);
	}
	else if (expr->op == Operator::IMP) {
		InformationState hypothetical_consequent = std::visit(Evaluator(), expr->rhs, std::variant<InformationState>(hypothetical_lhs));

		auto all_descendants_subsist = [&](const Possibility& p) -> bool {
			auto not_descendant_or_subsists = [&](const Possibility& p_star) -> bool {
				return !isDescendantOf(p_star, p, hypothetical_lhs) || subsistsIn(p_star, hypothetical_consequent);
			};
			return std::ranges::all_of(hypothetical_lhs.possibilities, not_descendant_or_subsists);
		};

		const auto if_subsists_all_descendants_do = [&](const Possibility& p) -> bool {
			return !subsistsIn(p, hypothetical_lhs) || all_descendants_subsist(p);
		};

		filter(s, if_subsists_all_descendants_do);
	}
	else {
		throw(std::invalid_argument("Invalid operator for binary formula"));
	}

	return s;
}

/**
* @brief Evaluates a quantified expression on an InformationState.
*
* Handles existential and universal quantifiers by iterating over possible
* individuals in the model and updating the state accordingly.
*
* @param expr The quantification expression to evaluate.
* @param state The current information state.
* @return The modified InformationState after applying the quantification.
* @throws std::invalid_argument if the quantifier is invalid.
*/
InformationState Evaluator::operator()(std::shared_ptr<QuantificationNode> expr, std::variant<InformationState> state) const
{
	InformationState& s = std::get<InformationState>(state);
	
	if (expr->quantifier == Quantifier::EXISTENTIAL) {
		std::vector<InformationState> all_state_variants;

		for (int i : std::views::iota(0, s.model.domain_cardinality())) {
			InformationState s_variant = update(s, expr->variable, i);
			all_state_variants.push_back(std::visit(Evaluator(), expr->scope, std::variant<InformationState>(s_variant)));
		}

		InformationState output(s.model, false);
		for (const auto& state_variant : all_state_variants) {
			for (const auto& p : state_variant.possibilities) {
				output.possibilities.insert(p);
			}
		}

		return output;
	}
	else if (expr->quantifier == Quantifier::UNIVERSAL) {
		std::vector<InformationState> all_hypothetical_updates;

		for (int d : std::views::iota(0, s.model.domain_cardinality())) {
			InformationState hypothetical = std::visit(Evaluator(), expr->scope, std::variant<InformationState>(update(s, expr->variable, d)));
			all_hypothetical_updates.push_back(hypothetical);
		}

		const auto subsists_in_all_hyp_updates = [&](const Possibility& p) -> bool {
			const auto p_subsists_in_hyp_update = [&](const InformationState& hypothetical) -> bool {
				return subsistsIn(p, hypothetical); 
			};
			return std::ranges::all_of(all_hypothetical_updates, p_subsists_in_hyp_update);
		};

		filter(s, subsists_in_all_hyp_updates);
	}
	else {
		throw(std::invalid_argument("Invalid quantifier"));
	}
	return s;
}

/**
* @brief Evaluates an identity expression, filtering based on variable or term equality.
*
* Compares the denotation of two terms or variables and retains only the
* possibilities where they are equal.
* 
* May throw std::out_of_range if either the LHS or the RHS of the identity lack
* an interpretation in the base model for the information state, or are variables
* without a binding quantifier or a proper anaphoric antecedent.
*
* @param expr The identity expression to evaluate.
* @param state The current information state.
* @return The filtered InformationState after applying identity conditions.
*/
InformationState Evaluator::operator()(std::shared_ptr<IdentityNode> expr, std::variant<InformationState> state) const
{
	InformationState& s = std::get<InformationState>(state);

	auto assigns_same_denotation = [&](const Possibility& p) -> bool { 
		const int lhs_denotation = isVariable(expr->lhs) ? variableDenotation(expr->lhs, p) : termDenotation(expr->lhs, p.world, s.model);
		const int rhs_denotation = isVariable(expr->lhs) ? variableDenotation(expr->rhs, p) : termDenotation(expr->rhs, p.world, s.model);
		return lhs_denotation == rhs_denotation;
	};

	filter(s, assigns_same_denotation);

	return s;
}

/**
 * @brief Evaluates a predicate expression by filtering states based on predicate denotation.
 *
 * Checks if a given predicate holds in the current world and filters
 * possibilities accordingly.
 * 
 * May throw std::out_of_range if (i) any argument to the predicate lacks an interpretation
 * in the base model for the information state, or is a variable without a binding quantifier
 * or a proper anaphoric antecedent, or (ii) the predicate lacks an interpretation in the
 * base model for the information state.
 *
 * @param expr The predicate expression to evaluate.
 * @param state The current information state.
 * @return The filtered InformationState after evaluating the predicate.
 */
InformationState Evaluator::operator()(std::shared_ptr<PredicationNode> expr, std::variant<InformationState> state) const
{
	InformationState& s = std::get<InformationState>(state);

	auto tuple_in_extension = [&](const Possibility& p) -> bool {
		std::vector<int> tuple;
		
		for (const std::string& argument : expr->arguments) {
			const int denotation = isVariable(argument) ? variableDenotation(argument, p) : termDenotation(argument, p.world, s.model);
			tuple.push_back(denotation);
		}

		return predicateDenotation(expr->predicate, p.world, s.model).contains(tuple);
	};

	filter(s, tuple_in_extension);

	return s;
}

}