#include "evaluator.hpp"

#include <algorithm>
#include <functional>
#include <ranges>
#include <stdexcept>

#include "logger.hpp"
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
	GSV_TRACE("Started evaluation of formula " + std::visit(Formatter(), Expression(expr)));

	InformationState hypothetical_update = std::visit(Evaluator(), expr->scope, params);
	InformationState& input_state = (std::get<std::pair<InformationState, const IModel*>>(params)).first;

	GSV_TRACE("Input context is:\n" + repr(input_state));
	GSV_TRACE("Hypothetical prejacent update is:\n" + repr(hypothetical_update));

	if (expr->op == Operator::E_POS) {
		GSV_TRACE("Running test on prejacent update");
		if (hypothetical_update.empty()) {
			GSV_TRACE("fail");
			input_state.clear();
		}
		GSV_TRACE("pass");
	}
	else if (expr->op == Operator::E_NEC) {
		GSV_TRACE("Running test on prejacent update");
		if (!subsistsIn(input_state, hypothetical_update)) {
			GSV_TRACE("fail");
			input_state.clear();
		}
		GSV_TRACE("pass");
	}
	else if (expr->op == Operator::NEG) {
		GSV_TRACE("Filtering input state");
		filter(input_state, [&](const Possibility& p) -> bool { return !subsistsIn(p, hypothetical_update); });
	}
	else {
		throw(std::invalid_argument("Invalid operator for unary formula"));
	}

	GSV_TRACE("Output context is:\n" + repr(input_state));
	GSV_TRACE("Finished evaluation of formula " + std::visit(Formatter(), Expression(expr)));

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
	GSV_TRACE("Started evaluation of formula " + std::visit(Formatter(), Expression(expr)));

	const IModel* model = (std::get<std::pair<InformationState, const IModel*>>(params)).second;
	InformationState& input_state = (std::get<std::pair<InformationState, const IModel*>>(params)).first;

	GSV_TRACE("Input context is:\n" + repr(input_state));

	if (expr->op == Operator::CON) {
		GSV_TRACE("Performing sequential updates : [" + std::visit(Formatter(), Expression(expr->lhs)) + "][" + std::visit(Formatter(), Expression(expr->rhs)) + "]");
		const InformationState output = std::visit(
			Evaluator(),
			expr->rhs, 
			std::variant<std::pair<InformationState, const IModel*>>(std::make_pair(std::visit(Evaluator(), expr->lhs, params), model))
		);
		GSV_TRACE("Output context is:\n" + repr(output));
		GSV_TRACE("Finished evaluation of formula " + std::visit(Formatter(), Expression(expr)));
		return output;
	}

	InformationState hypothetical_update_lhs = std::visit(Evaluator(), expr->lhs, params);

	GSV_TRACE("Hypothetical LHS update is:\n" + repr(hypothetical_update_lhs));

	if (expr->op == Operator::DIS) {
		InformationState hypothetical_update_rhs = std::visit(
			Evaluator(),
			expr->rhs,
			std::variant<std::pair<InformationState, const IModel*>>(std::make_pair(std::visit(Evaluator(), negate(expr->lhs), params), model))
		);

		GSV_TRACE("Hypothetical RHS update is:\n" + repr(hypothetical_update_rhs));

		const auto in_lhs_or_in_rhs = [&](const Possibility& p) -> bool {
			return hypothetical_update_lhs.contains(p) || hypothetical_update_rhs.contains(p);
		};

		GSV_TRACE("Filtering input state");
		filter(input_state, in_lhs_or_in_rhs);
	}
	else if (expr->op == Operator::IMP) {
		InformationState hypothetical_update_consequent = std::visit(
			Evaluator(),
			expr->rhs,
			std::variant<std::pair<InformationState, const IModel*>>(std::make_pair(hypothetical_update_lhs, model))
		);

		GSV_TRACE("Hypothetical RHS update is:\n" + repr(hypothetical_update_consequent));

		auto all_descendants_subsist = [&](const Possibility& p) -> bool {
			auto not_descendant_or_subsists = [&](const Possibility& p_star) -> bool {
				return !isDescendantOf(p_star, p, hypothetical_update_lhs) || subsistsIn(p_star, hypothetical_update_consequent);
			};
			return std::ranges::all_of(hypothetical_update_lhs, not_descendant_or_subsists);
		};

		const auto if_subsists_all_descendants_do = [&](const Possibility& p) -> bool {
			return !subsistsIn(p, hypothetical_update_lhs) || all_descendants_subsist(p);
		};

		GSV_TRACE("Filtering input state");
		filter(input_state, if_subsists_all_descendants_do);
	}
	else {
		throw(std::invalid_argument("Invalid operator for binary formula"));
	}

	GSV_TRACE("Output context is:\n" + repr(input_state));
	GSV_TRACE("Finished evaluation of formula " + std::visit(Formatter(), Expression(expr)));

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
	GSV_TRACE("Started evaluation of formula " + std::visit(Formatter(), Expression(expr)));

	InformationState& input_state = (std::get<std::pair<InformationState, const IModel*>>(params)).first;
	const IModel* model = (std::get<std::pair<InformationState, const IModel*>>(params)).second;

	GSV_TRACE("Input context is:\n" + repr(input_state));

	if (expr->quantifier == Quantifier::EXISTENTIAL) {
		std::vector<InformationState> all_state_variants;

        for (const int d : std::views::iota(0, model->domain_cardinality())) {
			GSV_TRACE("Evaluating subformula w.r.t. individual " + std::to_string(d) + " [out of " + std::to_string(model->domain_cardinality() - 1) + "]");
			GSV_TRACE("Constructing information state variants");
			GSV_TRACE("Updating referent system and assignment with [" + expr->variable + ", " + std::to_string(d) + "]");
            const InformationState s_variant = update(input_state, expr->variable, d);
			all_state_variants.push_back(std::visit(
				Evaluator(),
				expr->scope,
				std::variant<std::pair<InformationState, const IModel*>>(std::make_pair(s_variant, model)))
			);
		}

		GSV_TRACE("Expanding input information state");
		InformationState output;
		for (const auto& state_variant : all_state_variants) {
			for (const auto& p : state_variant) {
				output.insert(p);
			}
		}

		GSV_TRACE("Output context is:\n" + repr(output));
		GSV_TRACE("Finished evaluation of formula " + std::visit(Formatter(), Expression(expr)));

		return output;
	}

    if (expr->quantifier == Quantifier::UNIVERSAL) {
		std::vector<InformationState> all_hypothetical_updates;

        for (const int d : std::views::iota(0, model->domain_cardinality())) {
			GSV_TRACE("Evaluating subformula w.r.t. individual " + std::to_string(d) + " [out of " + std::to_string(model->domain_cardinality() - 1) + "]");
			GSV_TRACE("Constructing information state variants");
            const InformationState hypothetical_update = std::visit(
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

		GSV_TRACE("Filtering input state");
		filter(input_state, subsists_in_all_hyp_updates);
	}
	else {
		throw(std::invalid_argument("Invalid quantifier"));
	}

	GSV_TRACE("Output context is:\n" + repr(input_state));
	GSV_TRACE("Finished evaluation of formula " + std::visit(Formatter(), Expression(expr)));

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
	GSV_TRACE("Started evaluation of formula " + std::visit(Formatter(), Expression(expr)));

	InformationState& input_state = (std::get<std::pair<InformationState, const IModel*>>(params)).first;
	const IModel& model = *(std::get<std::pair<InformationState, const IModel*>>(params)).second;

	GSV_TRACE("Input context is:\n" + repr(input_state));

	auto assigns_same_denotation = [&](const Possibility& p) -> bool { 
		const int lhs_denotation = isVariable(expr->lhs) ? variableDenotation(expr->lhs, p) : model.termInterpretation(expr->lhs, p.world);
		const int rhs_denotation = isVariable(expr->lhs) ? variableDenotation(expr->rhs, p) : model.termInterpretation(expr->rhs, p.world);
		return lhs_denotation == rhs_denotation;
	};

	GSV_TRACE("Filtering input state");
	filter(input_state, assigns_same_denotation);

	GSV_TRACE("Output context is:\n" + repr(input_state));
	GSV_TRACE("Finished evaluation of formula " + std::visit(Formatter(), Expression(expr)));

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
	GSV_TRACE("Started evaluation of formula " + std::visit(Formatter(), Expression(expr)));

	InformationState& input_state = (std::get<std::pair<InformationState, const IModel*>>(params)).first;
	const IModel& model = *(std::get<std::pair<InformationState, const IModel*>>(params)).second;

	GSV_TRACE("Input context is:\n" + repr(input_state));

	auto tuple_in_extension = [&](const Possibility& p) -> bool {
		std::vector<int> tuple;
		
		for (const std::string& argument : expr->arguments) {
			const int denotation = isVariable(argument) ? variableDenotation(argument, p) : model.termInterpretation(argument, p.world);
			tuple.push_back(denotation);
		}

		return model.predicateInterpretation(expr->predicate, p.world).contains(tuple);
	};

	GSV_TRACE("Filtering input state");
	filter(input_state, tuple_in_extension);

	GSV_TRACE("Output context is:\n" + repr(input_state));
	GSV_TRACE("Finished evaluation of formula " + std::visit(Formatter(), Expression(expr)));

	return std::move(input_state);
}

/**
 * @brief Evaluates an expression within a given information state and model.
 *
 * This function takes as input an `InformationState' object, applies
 * an `Evaluator` visitor to the input `Expression`, and returns the
 * output `InformationState`, given the semantic values provided by the
 * `Model' object. It utilizes `std::visit` to dynamically apply the
 * appropriate evaluation logic based on the type of `expr`.
 *
 * @param expr The expression to evaluate.
 * @param input_state The initial information state used during evaluation.
 * @param model The model that provides contextual information for evaluation.
 * @return The resulting `InformationState` after evaluating the expression.
 * @throws `std::invalid_argument', if formula does not accord with GSV grammar
 * @throws `std::out_of_range' if interpretation is undefined
 */
InformationState evaluate(const Expression& expr, const InformationState& input_state, const IModel& model)
{
	return std::visit(
		Evaluator(),
		expr,
		std::variant<std::pair<InformationState, const IModel*>>(std::make_pair(input_state, &model))
	);
}

}
