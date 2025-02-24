#include "evaluator.hpp"

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <ranges>

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

int termDenotation(std::string_view term, int w, const IModel& m)
{
	return m.termInterpretation(term, w);
}

const std::set<std::vector<int>>& predicateDenotation(std::string_view predicate, int w, const IModel& m)
{
	return m.predicateInterpretation(predicate, w);
}

int variableDenotation(std::string_view variable, const Possibility& p)
{
	return p.assignment.at(p.referentSystem->value(variable));
}

} // ANONYMOUS NAMESPACE

/**
* @brief Evaluates a unary logical expression on an InformationState.
*
* Applies an operator (such as necessity, possibility, or negation) to
* modify the given state accordingly.
*
* @param expr The unary expression to evaluate.
* @param params The input information state and IModel pointer
* @return The modified InformationState after applying the operation.
* @throws std::invalid_argument if the operator is invalid.
*/
InformationState Evaluator::operator()(const std::shared_ptr<UnaryNode>& expr, std::variant<std::pair<InformationState, const IModel*>> params) const
{
	InformationState hypothetical_update = std::visit(Evaluator(), expr->scope, params);
	InformationState& input_state = (std::get<std::pair<InformationState, const IModel*>>(params)).first;

	if (expr->op == Operator::E_POS) {
		if (hypothetical_update.empty()) {
			input_state.clear();
		}
	}
	else if (expr->op == Operator::E_NEC) {
		if (!subsistsIn(input_state, hypothetical_update)) {
			input_state.clear();
		}
	}
	else if (expr->op == Operator::NEG) {
		filter(input_state, [&](const Possibility& p) -> bool { return !subsistsIn(p, hypothetical_update); });
	}
	else {
		throw(std::invalid_argument("Invalid operator for unary formula"));
	}

	return std::move(input_state);
}

/**
* @brief Evaluates a binary logical expression on an InformationState.
*
* Processes logical operations such as conjunction, disjunction, and
* implication, modifying the state accordingly.
*
* @param expr The binary expression to evaluate.
* @param params The input information state and IModel pointer
* @return The modified InformationState after applying the operation.
* @throws std::invalid_argument if the operator is invalid.
*/
InformationState Evaluator::operator()(const std::shared_ptr<BinaryNode>& expr, std::variant<std::pair<InformationState, const IModel*>> params) const
{
	const IModel* model = (std::get<std::pair<InformationState, const IModel*>>(params)).second;

	if (expr->op == Operator::CON) {
		return std::visit(
			Evaluator(),
			expr->rhs, 
			std::variant<std::pair<InformationState, const IModel*>>(std::make_pair(std::visit(Evaluator(), expr->lhs, params), model))
		);
	}

	InformationState& input_state = (std::get<std::pair<InformationState, const IModel*>>(params)).first;
	InformationState hypothetical_update_lhs = std::visit(Evaluator(), expr->lhs, params);

	if (expr->op == Operator::DIS) {
		InformationState hypothetical_update_rhs = std::visit(
			Evaluator(),
			expr->rhs,
			std::variant<std::pair<InformationState, const IModel*>>(std::make_pair(std::visit(Evaluator(), negate(expr->lhs), params), model))
		);

		const auto in_lhs_or_in_rhs = [&](const Possibility& p) -> bool {
			return hypothetical_update_lhs.contains(p) || hypothetical_update_rhs.contains(p);
		};

		filter(input_state, in_lhs_or_in_rhs);
	}
	else if (expr->op == Operator::IMP) {
		InformationState hypothetical_update_consequent = std::visit(
			Evaluator(),
			expr->rhs,
			std::variant<std::pair<InformationState, const IModel*>>(std::make_pair(hypothetical_update_lhs, model))
		);

		auto all_descendants_subsist = [&](const Possibility& p) -> bool {
			auto not_descendant_or_subsists = [&](const Possibility& p_star) -> bool {
				return !isDescendantOf(p_star, p, hypothetical_update_lhs) || subsistsIn(p_star, hypothetical_update_consequent);
			};
			return std::ranges::all_of(hypothetical_update_lhs, not_descendant_or_subsists);
		};

		const auto if_subsists_all_descendants_do = [&](const Possibility& p) -> bool {
			return !subsistsIn(p, hypothetical_update_lhs) || all_descendants_subsist(p);
		};

		filter(input_state, if_subsists_all_descendants_do);
	}
	else {
		throw(std::invalid_argument("Invalid operator for binary formula"));
	}

	return std::move(input_state);
}

/**
* @brief Evaluates a quantified expression on an InformationState.
*
* Handles existential and universal quantifiers by iterating over possible
* individuals in the model and updating the state accordingly.
*
* @param expr The quantification expression to evaluate.
* @param params The input information state and IModel pointer
* @return The modified InformationState after applying the quantification.
* @throws std::invalid_argument if the quantifier is invalid.
*/
InformationState Evaluator::operator()(const std::shared_ptr<QuantificationNode>& expr, std::variant<std::pair<InformationState, const IModel*>> params) const
{
	InformationState& input_state = (std::get<std::pair<InformationState, const IModel*>>(params)).first;
	const IModel* model = (std::get<std::pair<InformationState, const IModel*>>(params)).second;

	if (expr->quantifier == Quantifier::EXISTENTIAL) {
		std::vector<InformationState> all_state_variants;

		for (int i : std::views::iota(0, model->domain_cardinality())) {
			InformationState s_variant = update(input_state, expr->variable, i);
			all_state_variants.push_back(std::visit(
				Evaluator(),
				expr->scope,
				std::variant<std::pair<InformationState, const IModel*>>(std::make_pair(s_variant, model)))
			);
		}

		InformationState output;
		for (const auto& state_variant : all_state_variants) {
			for (const auto& p : state_variant) {
				output.insert(p);
			}
		}

		return output;
	}
	else if (expr->quantifier == Quantifier::UNIVERSAL) {
		std::vector<InformationState> all_hypothetical_updates;

		for (int d : std::views::iota(0, model->domain_cardinality())) {
			InformationState hypothetical_update = std::visit(
				Evaluator(),
				expr->scope,
				std::variant<std::pair<InformationState, const IModel*>>(std::make_pair(update(input_state, expr->variable, d), model))
			);
			all_hypothetical_updates.push_back(hypothetical_update);
		}

		const auto subsists_in_all_hyp_updates = [&](const Possibility& p) -> bool {
			const auto p_subsists_in_hyp_update = [&](const InformationState& hypothetical_update) -> bool {
				return subsistsIn(p, hypothetical_update);
			};
			return std::ranges::all_of(all_hypothetical_updates, p_subsists_in_hyp_update);
		};

		filter(input_state, subsists_in_all_hyp_updates);
	}
	else {
		throw(std::invalid_argument("Invalid quantifier"));
	}

	return std::move(input_state);
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
* @param params The input information state and IModel pointer
* @return The filtered InformationState after applying identity conditions.
* @throws std::invalid_argument if the quantifier is invalid.
*/
InformationState Evaluator::operator()(const std::shared_ptr<IdentityNode>& expr, std::variant<std::pair<InformationState, const IModel*>> params) const
{
	InformationState& input_state = (std::get<std::pair<InformationState, const IModel*>>(params)).first;
	const IModel& model = *(std::get<std::pair<InformationState, const IModel*>>(params)).second;

	auto assigns_same_denotation = [&](const Possibility& p) -> bool { 
		const int lhs_denotation = isVariable(expr->lhs) ? variableDenotation(expr->lhs, p) : termDenotation(expr->lhs, p.world, model);
		const int rhs_denotation = isVariable(expr->lhs) ? variableDenotation(expr->rhs, p) : termDenotation(expr->rhs, p.world, model);
		return lhs_denotation == rhs_denotation;
	};

	filter(input_state, assigns_same_denotation);

	return std::move(input_state);
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
 * @param params The input information state and IModel pointer
 * @return The filtered InformationState after evaluating the predicate.
 * @throws std::invalid_argument if the quantifier is invalid.
 */
InformationState Evaluator::operator()(const std::shared_ptr<PredicationNode>& expr, std::variant<std::pair<InformationState, const IModel*>> params) const
{
	InformationState& input_state = (std::get<std::pair<InformationState, const IModel*>>(params)).first;
	const IModel& model = *(std::get<std::pair<InformationState, const IModel*>>(params)).second;

	auto tuple_in_extension = [&](const Possibility& p) -> bool {
		std::vector<int> tuple;
		
		for (const std::string& argument : expr->arguments) {
			const int denotation = isVariable(argument) ? variableDenotation(argument, p) : termDenotation(argument, p.world, model);
			tuple.push_back(denotation);
		}

		return predicateDenotation(expr->predicate, p.world, model).contains(tuple);
	};

	filter(input_state, tuple_in_extension);

	return std::move(input_state);
}

}
