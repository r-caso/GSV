#include "semantic_relations.hpp"

#include <algorithm>
#include <format>
#include <functional>
#include <ranges>
#include <stdexcept>
#include <variant>
#include <vector>

#include <QMLExpression/formatter.hpp>

#include "evaluator.hpp"
#include "imodel.hpp"
#include "information_state.hpp"
#include "possibility.hpp"

namespace iif_sadaf::talk::GSV {

/**
 * @brief Determines whether an expression is consistent with a given information state and model.
 * 
 * This function evaluates the given expression against the provided information state and model.
 * If the evaluation succeeds and results in a non-empty information state, the expression is 
 * considered consistent.
 * 
 * @param expr The expression to evaluate.
 * @param state The initial information state.
 * @param model The model used for evaluation.
 * @return std::expected<bool, std::string> `true` if the expression is consistent (i.e., 
 *         it does not result in an empty state), `false` otherwise. Returns an error message 
 *         if evaluation fails.
 * 
 * @details
 * - If evaluation produces an empty information state, the expression is considered inconsistent.
 * - If an error occurs during evaluation, the error message is returned instead.
 */
std::expected<bool, std::string> consistent(const QMLExpression::Expression& expr, const InformationState& state, const IModel& model)
{
	const auto hypothetical_update = evaluate(expr, state, model);

	if (!hypothetical_update.has_value()) {
		return std::unexpected(
			std::format(
				"In evaluating formula {}:\n{}",
				std::visit(QMLExpression::Formatter(), QMLExpression::Expression(expr)),
				hypothetical_update.error()
			)
		);
	}
	
	return !hypothetical_update.value().empty();
}

/**
 * @brief Checks whether an information state allows a given expression.
 *
 * This function determines if the given expression is consistent with the provided
 * information state and model. It simply delegates to `consistent()`, meaning an expression
 * is "allowed" if it does not result in an empty information state.
 *
 * @param state The initial information state.
 * @param expr The expression to evaluate.
 * @param model The model used for evaluation.
 * @return std::expected<bool, std::string> `true` if the expression is consistent with the state,
 *         `false` otherwise. Returns an error message if evaluation fails.
 */
std::expected<bool, std::string> allows(const InformationState& state, const QMLExpression::Expression& expr, const IModel& model)
{
	return consistent(expr, state, model);
}

/**
 * @brief Determines whether an information state supports a given expression.
 *
 * This function checks if the evaluated update of the given expression
 * subsists in the original information state. An expression is "supported"
 * if its evaluation does not introduce information that is absent from the state.
 *
 * @param state The initial information state.
 * @param expr The expression to evaluate.
 * @param model The model used for evaluation.
 * @return std::expected<bool, std::string> `true` if the evaluated update subsists
 *         in the initial state, `false` otherwise. Returns an error message if evaluation fails.
 */
std::expected<bool, std::string> supports(const InformationState& state, const QMLExpression::Expression& expr, const IModel& model)
{
	const auto hypothetical_update = evaluate(expr, state, model);
	
	if (!hypothetical_update.has_value()) {
		return std::unexpected(
			std::format(
				"In evaluating formula {}:\n{}",
				std::visit(QMLExpression::Formatter(), QMLExpression::Expression(expr)),
				hypothetical_update.error()
			)
		);
	}
	
	return subsistsIn(state, hypothetical_update.value());
}

/**
 * @brief Checks if an expression is supported by a given information state.
 *
 * This function is equivalent to `supports(state, expr, model)`, verifying
 * whether the evaluation of the expression does not introduce information
 * absent from the given information state.
 *
 * @param expr The expression to evaluate.
 * @param state The initial information state.
 * @param model The model used for evaluation.
 * @return std::expected<bool, std::string> `true` if the expression is supported
 *         by the state, `false` otherwise. Returns an error message if evaluation fails.
 */
std::expected<bool, std::string> isSupportedBy(const QMLExpression::Expression& expr, const InformationState& state, const IModel& model)
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
 * @brief Determines whether an expression is consistent within a given model.
 *
 * This function checks if there exists at least one information state within
 * the model where the given expression does not lead to an empty update.
 * It iterates over different possible information states and ensures that
 * at least one state allows a non-empty update of the expression.
 *
 * @param expr The expression to check for consistency.
 * @param model The model against which the expression is evaluated.
 * @return std::expected<bool, std::string> `true` if the expression is
 *         consistent in at least one information state, `false` otherwise.
 *         Returns an error message if evaluation fails.
 */
std::expected<bool, std::string> consistent(const QMLExpression::Expression& expr, const IModel& model)
{
	for (const int i : std::views::iota(0, model.world_cardinality())) {
		std::vector<InformationState> states = generateSubStates(model.world_cardinality() - 1, i);
		const auto is_consistent = [&](const InformationState& state) -> bool {
			const auto result = consistent(expr, state, model); 
			if (!result.has_value()) {
				throw std::runtime_error(result.error());
			}
			return result.value();
		};
		try {
			if (!std::ranges::any_of(states, is_consistent)) {
				return false;
			}
		}
		catch (const std::runtime_error& e) {
			return std::unexpected(e.what());
		}
	}
	return true;
}

/**
 * @brief Determines whether an expression is coherent within a given model.
 *
 * This function checks if there exists at least one non-empty information state
 * that supports the given expression. It iterates over different possible
 * information states and ensures that at least one state both (1) is not empty
 * and (2) supports the expression.
 *
 * @param expr The expression to check for coherence.
 * @param model The model against which the expression is evaluated.
 * @return std::expected<bool, std::string> `true` if the expression is coherent
 *         in at least one information state, `false` otherwise.
 *         Returns an error message if evaluation fails.
 */
std::expected<bool, std::string> coherent(const QMLExpression::Expression& expr, const IModel& model)
{
	for (const int i : std::views::iota(0, model.world_cardinality())) {
		std::vector<InformationState> states = generateSubStates(model.world_cardinality() - 1, i);
		const auto is_not_empty_or_supports_expression = [&](const InformationState& state) -> bool {
			const auto result = supports(state, expr, model);
			if (!result.has_value()) {
				throw std::runtime_error(result.error());
			}
			return !state.empty() && result.value(); 
		};
		try {
			if (!std::ranges::any_of(states, is_not_empty_or_supports_expression)) {
				return false;
			}
		}
		catch (const std::runtime_error& e) {
			return std::unexpected(e.what());
		}
	}
	return true;
}

/**
 * @brief Determines whether a set of premises entails a conclusion within a given model.
 *
 * This function evaluates whether the conclusion follows from the premises in all
 * possible information states. It iterates through subsets of possible worlds and
 * applies updates from each premise to the current information state. The conclusion
 * is then evaluated to check whether it is supported in the updated state.
 *
 * @param premises A vector of expressions representing the premises.
 * @param conclusion The expression representing the conclusion.
 * @param model The model against which entailment is evaluated.
 * @return std::expected<bool, std::string> `true` if the conclusion is supported
 *         in all states updated by the premises, `false` otherwise.
 *         Returns an error message if evaluation fails.
 */
std::expected<bool, std::string> entails(const std::vector<QMLExpression::Expression>& premises, const QMLExpression::Expression& conclusion, const IModel& model)
{
	for (const int i : std::views::iota(0, model.world_cardinality())) {
		std::vector<InformationState> states = generateSubStates(model.world_cardinality() - 1, i);
		for (InformationState& input_state : states) {
			// Update input state with premises
			for (const QMLExpression::Expression& expr : premises) {
				const auto update = evaluate(expr, input_state, model);
				if (!update.has_value()) {
					return std::unexpected(
						std::format(
							"In evaluating formula {}:\n{}",
							std::visit(QMLExpression::Formatter(), QMLExpression::Expression(expr)),
							update.error()
						)
					);
				}
				input_state = update.value();
			}

			// check if update with conclusion exists
			const auto update = evaluate(conclusion, input_state, model);

			// update does not exist
			if (!update.has_value()) {
				return std::unexpected(
					std::format(
						"In evaluating formula {}:\n{}",
						std::visit(QMLExpression::Formatter(), QMLExpression::Expression(conclusion)),
						update.error()
					)
				);
			}

			// update exists, check for support
			const auto does_support = supports(input_state, conclusion, model);
			if (!does_support.has_value()) {
				return std::unexpected(
					std::format(
						"In evaluating formula {}:\n{}",
						std::visit(QMLExpression::Formatter(), QMLExpression::Expression(conclusion)),
						does_support.error()
					)
				);
			}
			if (!does_support.value()) {
				return false;
			}
		}
	}
	return true;
}

namespace {

std::expected<bool, std::string> similar(const Possibility& p1, const Possibility& p2)
{
	const auto have_same_denotation = [&](std::string_view variable) -> bool {
		const auto denotation_at_p1 = variableDenotation(variable, p1);
		const auto denotation_at_p2 = variableDenotation(variable, p2);
		if (!denotation_at_p1.has_value()) {
			throw std::out_of_range(denotation_at_p1.error());
		}
		if (!denotation_at_p2.has_value()) {
			throw std::out_of_range(denotation_at_p2.error());
		}
		return denotation_at_p1.value() == denotation_at_p2.value();
	};

	try {
		return p1.world == p2.world
			&& domain(*p1.referentSystem) == domain(*p2.referentSystem)
			&& std::ranges::all_of(domain(*p1.referentSystem), have_same_denotation);
	}
	catch (const std::out_of_range& e) {
		return std::unexpected(e.what());
	}
}

std::expected<bool, std::string> similar(const InformationState& s1, const InformationState& s2)
{
	const auto has_similar_possibility_in_s2 = [&](const Possibility p) -> bool { 
		const auto is_similar_to_p = [&](const Possibility p_dash) -> bool {
			const auto comparison_result = similar(p, p_dash);
			if (!comparison_result.has_value()) {
				throw std::out_of_range(comparison_result.error());
			}
			return comparison_result.value();
		};
		return std::ranges::any_of(s2, is_similar_to_p); 
	};

	const auto has_similar_possibility_in_s1 = [&](const Possibility p) -> bool { 
		const auto is_similar_to_p = [&](const Possibility p_dash) -> bool {
			const auto comparison_result = similar(p, p_dash);
			if (!comparison_result.has_value()) {
				throw std::out_of_range(comparison_result.error());
			}
			return comparison_result.value();
		};
		return std::ranges::any_of(s1, is_similar_to_p);
	};

	try {
		return std::ranges::all_of(s1, has_similar_possibility_in_s2)
			&& std::ranges::all_of(s2, has_similar_possibility_in_s1);
	}
	catch (const std::out_of_range& e) {
		return std::unexpected(e.what());
	}
}

} // ANONYMOUS NAMESPACE

/**
 * @brief Determines whether two expressions are logically equivalent within a given model.
 *
 * This function evaluates whether the two expressions always produce similar updates
 * to an information state across all possible subsets of worlds in the model. It
 * iterates through these subsets, applying each expression and comparing their
 * resulting states for similarity.
 *
 * @param expr1 The first expression to compare.
 * @param expr2 The second expression to compare.
 * @param model The model against which equivalence is evaluated.
 * @return std::expected<bool, std::string> `true` if the expressions always produce
 *         similar updates, `false` otherwise. Returns an error message if evaluation fails.
 */
std::expected<bool, std::string> equivalent(const QMLExpression::Expression& expr1, const QMLExpression::Expression& expr2, const IModel& model)
{
	for (const int i : std::views::iota(0, model.world_cardinality())) {
		std::vector<InformationState> states = generateSubStates(model.world_cardinality() - 1, i);

		const auto dissimilar_updates = [&](const InformationState& state) ->bool { 
			const auto expr1_update = evaluate(expr1, state, model);
			if (!expr1_update.has_value()) {
				throw std::out_of_range(expr1_update.error());
			}
			const auto expr2_update = evaluate(expr2, state, model);
			if (!expr2_update.has_value()) {
				throw std::out_of_range(expr2_update.error());
			}

			return !similar(expr1_update.value(), expr2_update.value());
		};

		try {
			if (std::ranges::any_of(states, dissimilar_updates)) {
				return false;
			}
		}
		catch (const std::out_of_range& e) {
			return std::unexpected(e.what());
		}
	}

	return true;
}

}
