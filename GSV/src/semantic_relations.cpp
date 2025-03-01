#include "semantic_relations.hpp"

#include <algorithm>
#include <functional>
#include <ranges>
#include <stdexcept>
#include <variant>
#include <vector>

#include "evaluator.hpp"
#include "imodel.hpp"
#include "information_state.hpp"
#include "possibility.hpp"

namespace iif_sadaf::talk::GSV {

/**
 * @brief Checks if an expression is consistent with a given information state, relative to a model.
 *
 * This function evaluates the expression with respect to the provided state. If the
 * evaluation does not result in an empty information state, the expression is considered
 * consistent. If an out of range exception occurs during evaluation (signaling that the
 * update does not exist, or is not defined, for the input formula), the expression
 * is deemed inconsistent.
 * 
 * @param expr The expression to check for consistency.
 * @param state The information state used for evaluation.
 * @param model The model providing contextual information for evaluation.
 * @return `true` if the expression is consistent with the state and model, otherwise `false`.
 * @throws `std::invalid_argument' if `expr` contains an invalid logical operator or quantifier.
 */
bool consistent(const Expression& expr, const InformationState& state, const IModel& model) 
{
	try {
		return !evaluate(expr, state, model).empty();
	}
	catch (const std::out_of_range&) {
		return false;
	}
}

/**
 * @brief Determines whether an information state allows a given expression.
 *
 * This function is a direct alias for `consistent`, checking if the expression is
 * compatible with the given state, relative to the provided model.
 * 
 * @param state The information state.
 * @param expr The expression to check.
 * @param model The model providing evaluation context.
 * @return `true` if the expression is allowed in the given state and model, otherwise `false`.
 * @throws `std::invalid_argument' if `expr` contains an invalid logical operator or quantifier.
 */
bool allows(const InformationState& state, const Expression& expr, const IModel& model)
{
	return consistent(expr, state, model);
}

/**
 * @brief Checks if an information state supports a given expression.
 *
 * The function evaluates the expression with respect to the input state, relative to the 
 * provided model. If the output information state subsists in the original state, the expression
 * is considered supported. If an out of range exception occurs during evaluation (signaling that the
 * update does not exist, or is not defined, for the input formula), the expression is not supported
 * by the input state.
 * 
 * @param state The information state to check.
 * @param expr The expression being tested for support.
 * @param model The model providing evaluation context.
 * @return `true` if the expression is supported by the state and model, otherwise `false`.
 * @throws `std::invalid_argument' if `expr` contains an invalid logical operator or quantifier.
 */
bool supports(const InformationState& state, const Expression& expr, const IModel& model)
{
	try {
		return subsistsIn(state, evaluate(expr, state, model));
	}
	catch (const std::out_of_range&) {
		return false;
	}
}

/**
 * @brief Determines whether an expression is supported by an information state.
 *
 * This function is an alias for `supports`, checking if the given state provides
 * support for the specified expression.
 * 
 * @param expr The expression being tested for support.
 * @param state The information state to check against.
 * @param model The model providing evaluation context.
 * @return `true` if the expression is supported by the state and model, otherwise `false`.
 * @throws `std::invalid_argument' if `expr` contains an invalid logical operator or quantifier.
 */
bool isSupportedBy(const Expression& expr, const InformationState& state, const IModel& model)
{
	return supports(state, expr, model);
}

namespace {

std::vector<InformationState> generateSubStates(int n, int k) {
	std::vector<InformationState> result;

	if (k == 0) {
		result.push_back(InformationState());
		return result;
	}

	if (k > n + 1) {
		return result;
	}

	int estimate = 1;
	for (int i = 1; i <= k; i++) {
		estimate = estimate * (n + 2 - i) / i;
	}
	result.reserve(estimate);

	std::function<void(int, InformationState&)> backtrack =
		[&](int start, InformationState& current) {
			if (current.size() == k) {
				result.push_back(current);
				return;
			}

			ReferentSystem r;

			for (int i = start; i <= n; ++i) {
				Possibility p(std::make_shared<ReferentSystem>(r), i);
				current.insert(p);

				backtrack(i + 1, current);

				current.erase(p);
			}
		};

	InformationState current;
	backtrack(0, current);

	return result;
}

} // ANONYMOUS NAMESPACE

/**
 * @brief Determines whether an expression is consistent relative to a given model.
 *
 * This function iterates over all possible information states given the base model,
 * and checks if at least one of them is consistent with the given expression.
 * If the expression is inconsistent in all such information states,
 * the function returns `false`.
 *
 * @param expr The expression to check for consistency.
 * @param model The model providing the world structure for evaluation.
 * @return `true` if the expression is consistent across all worlds, otherwise `false`.
 * @throws `std::invalid_argument' if `expr` contains an invalid logical operator or quantifier.
 */
bool consistent(const Expression& expr, const IModel& model)
{
	for (const int i : std::views::iota(0, model.world_cardinality())) {
		std::vector<InformationState> states = generateSubStates(model.world_cardinality() - 1, i);
		if (!std::ranges::any_of(states, [&](const InformationState& state) -> bool { return consistent(expr, state, model); })) {
			return false;
		}
	}
	return true;
}

/**
 * @brief Determines whether an expression is coherent relative to a given model.
 *
 * This function iterates over all possible information states given the base model,
 * and checks if at least one of them both supports the given expression and
 * is non-empty. If the expression is not supported in any such information state,
 * the function returns `false`.
 *
 * @param expr The expression to check for coherence.
 * @param model The model providing the world structure for evaluation.
 * @return `true` if the expression is coherent across all worlds, otherwise `false`.
 * @throws `std::invalid_argument' if `expr` contains an invalid logical operator or quantifier.
 */
bool coherent(const Expression& expr, const IModel& model)
{
	for (const int i : std::views::iota(0, model.world_cardinality())) {
		std::vector<InformationState> states = generateSubStates(model.world_cardinality() - 1, i);
		if (!std::ranges::any_of(states, [&](const InformationState& state) -> bool { return !state.empty() && supports(state, expr, model); })) {
			return false;
		}
	}
	return true;
}

/**
 * @brief Determines whether a set of premises entails a given conclusion, relative to a given model.
 *
 * This function iterates over all possible information states relative to the base model,
 * and checks whether applying the premises results in a state that supports the conclusion.
 * If in any world the premises fail to support the conclusion, entailment fails.
 *
 * @param premises A vector of expressions representing the premises.
 * @param conclusion The expression representing the conclusion.
 * @param model The model in which entailment is evaluated.
 * @return `true` if the premises entail the conclusion in all worlds, otherwise `false`.
 * @throws `std::invalid_argument' if `expr` contains an invalid logical operator or quantifier.
 */
bool entails(const std::vector<Expression>& premises, const Expression& conclusion, const IModel& model)
{
	for (const int i : std::views::iota(0, model.world_cardinality())) {
		std::vector<InformationState> states = generateSubStates(model.world_cardinality() - 1, i);
		for (InformationState& input_state : states) {
			try {
				for (const Expression& expr : premises) {
					input_state = evaluate(expr, input_state, model);
				}

				(void)evaluate(conclusion, input_state, model);

				// If we get to this point, update exists, so we check for support

				if (!supports(input_state, conclusion, model)) {
					return false;
				}
			}
			catch (const std::out_of_range&) {
				; // If update does not exist, then state does not count against entailment
			}
		}
	}
	return true;
}

namespace {

bool similar(const Possibility& p1, const Possibility& p2)
{
	const auto have_same_denotation = [&](std::string_view variable) -> bool {
		return p1.assignment.at(p1.referentSystem->value(variable)) == p2.assignment.at(p2.referentSystem->value(variable));
	};

	try {
		return p1.world == p2.world
			&& domain(*p1.referentSystem) == domain(*p2.referentSystem)
			&& std::ranges::all_of(domain(*p1.referentSystem), have_same_denotation);
	}
	catch (const std::out_of_range&) {
		return false;
	}
}

bool similar(const InformationState& s1, const InformationState& s2)
{
	const auto has_similar_possibility_in_s2 = [&](const Possibility p) -> bool { 
		const auto is_similar_to_p = [&](const Possibility p_dash) -> bool {
			return similar(p, p_dash);
		};
		return std::ranges::any_of(s2, is_similar_to_p); 
	};

	const auto has_similar_possibility_in_s1 = [&](const Possibility p) -> bool { 
		const auto is_similar_to_p = [&](const Possibility p_dash) -> bool {
			return similar(p, p_dash);
		};
		return std::ranges::any_of(s1, is_similar_to_p);
	};

	try {
		return std::ranges::all_of(s1, has_similar_possibility_in_s2)
			&& std::ranges::all_of(s2, has_similar_possibility_in_s1);
	}
	catch (const std::out_of_range&) {
		return false;
	}
}

} // ANONYMOUS NAMESPACE

/**
 * @brief Determines whether two expressions are equivalent, relative to a given model.
 *
 * Two expressions are considered equivalent if, in every possible information state,
 * relative to the base model, their evaluation results in similar information states
 * (under the GSV definition of similarity).
 *
 * @param expr1 The first expression to compare.
 * @param expr2 The second expression to compare.
 * @param model The model in which equivalence is evaluated.
 * @return `true` if the expressions are equivalent in all worlds, otherwise `false`.
 * @throws `std::invalid_argument' if `expr1` or `expr2' contain an invalid logical operator or quantifier.
 */
bool equivalent(const Expression& expr1, const Expression& expr2, const IModel& model)
{
	for (const int i : std::views::iota(0, model.world_cardinality())) {
		std::vector<InformationState> states = generateSubStates(model.world_cardinality() - 1, i);

		const auto dissimilar_updates = [&](const InformationState& state) ->bool { 
			return !similar(evaluate(expr1, state, model), evaluate(expr2, state, model));
		};

		try {
			if (std::ranges::any_of(states, dissimilar_updates)) {
				return false;
			}
		}
		catch (const std::out_of_range&) {
			return false;
		}
	}

	return true;
}

}
