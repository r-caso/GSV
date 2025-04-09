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

#include "gsv-utils/error_reporting.hpp"

namespace iif_sadaf::talk::GSV {

/**
 * @brief Determines whether an expression is consistent with a given information state, relative to a base model.
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
std::expected<bool, std::string> consistent(const QMLExpression::Expression& expr, const InformationState& state, const IModel& model, GSVLogger* logger)
{
	const std::string formula = std::visit(QMLExpression::Formatter(), QMLExpression::Expression(expr));

	logger = normalize(logger);
	logger->log(std::format("Evaluating formula '{}' for consistency with current information state", formula));
	logger->log(std::format("Current state is:\n{}", str(state, false)));

	const auto hypothetical_update = evaluate(expr, state, model);

	if (!hypothetical_update.has_value()) {
		const std::string error_message = explain_failure(formula, hypothetical_update.error());
		logger->log(std::format("Evaluation failed with the following error:\n{}", error_message));
		return std::unexpected(error_message);
	}
	
	const std::string result_string = hypothetical_update.value().empty() ? "False" : "True";
	logger->log(std::format("Evaluation result: {}", result_string));
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
std::expected<bool, std::string> allows(const InformationState& state, const QMLExpression::Expression& expr, const IModel& model, GSVLogger* logger)
{
	logger = normalize(logger);
	return consistent(expr, state, model, logger);
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
std::expected<bool, std::string> supports(const InformationState& state, const QMLExpression::Expression& expr, const IModel& model, GSVLogger* logger)
{
	const std::string formula = std::visit(QMLExpression::Formatter(), QMLExpression::Expression(expr));

	logger = normalize(logger);
	logger->log(std::format("Evaluating formula '{}' for support by current information state", formula));
	logger->log(std::format("Current state is:\n{}", str(state, false)));

	const auto hypothetical_update = evaluate(expr, state, model);
	
	if (!hypothetical_update.has_value()) {
		const std::string error_message = explain_failure(formula, hypothetical_update.error());
		logger->log(std::format("Evaluation failed with the following error:\n{}", error_message));
		return std::unexpected(error_message);
	}
	
	const bool evaluation_result = subsistsIn(state, hypothetical_update.value());
	const std::string result_string = evaluation_result ? "True" : "False";
	logger->log(std::format("Evaluation result: {}", result_string));
	return evaluation_result;
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
std::expected<bool, std::string> isSupportedBy(const QMLExpression::Expression& expr, const InformationState& state, const IModel& model, GSVLogger* logger)
{
	logger = normalize(logger);

	return supports(state, expr, model, logger);
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
 * This function checks if there exists at least one information state definable in terms of
 * the base model where the given expression does not lead to an empty update.
 * It iterates over different possible information states and ensures that
 * at least one state allows a non-empty update of the expression.
 *
 * @param expr The expression to check for consistency.
 * @param model The model against which the expression is evaluated.
 * @return std::expected<bool, std::string> `true` if the expression is
 *         consistent in at least one information state, `false` otherwise.
 *         Returns an error message if evaluation fails.
 */
std::expected<bool, std::string> consistent(const QMLExpression::Expression& expr, const IModel& model, GSVLogger* logger)
{
	const std::string formula = std::visit(QMLExpression::Formatter(), QMLExpression::Expression(expr));

	logger = normalize(logger);
	logger->log(std::format("Evaluating formula '{}' for consistency", formula));

	for (const int i : std::views::iota(0, model.worldCardinality())) {
		std::vector<InformationState> states = generateSubStates(model.worldCardinality() - 1, i);
		const auto is_consistent = [&](const InformationState& state) -> bool {
			const auto result = consistent(expr, state, model); 
			if (!result.has_value()) {
				throw std::runtime_error(result.error());
			}
			const bool result_value = result.value();
			if (!result_value) {
				logger->log(std::format("Formula is inconsistent with the following information state:\n{}", str(state, false)));
			}
			return result_value;
		};
		try {
			if (!std::ranges::any_of(states, is_consistent)) {
				logger->log("Evaluation result: False");
				return false;
			}
		}
		catch (const std::runtime_error& e) {
			const std::string error_message = e.what();
			logger->log(std::format("Evaluation failed with the following error:\n{}", error_message));
			return std::unexpected(error_message);
		}
	}
	logger->log("Evaluation result: True");
	return true;
}

/**
 * @brief Determines whether an expression is coherent within a given model.
 *
 * This function checks if there exists at least one non-empty information state definable with respect to the base model
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
std::expected<bool, std::string> coherent(const QMLExpression::Expression& expr, const IModel& model, GSVLogger* logger)
{
	const std::string formula = std::visit(QMLExpression::Formatter(), QMLExpression::Expression(expr));

	logger = normalize(logger);
	logger->log(std::format("Evaluating formula '{}' for coherence", formula));

	for (const int i : std::views::iota(0, model.worldCardinality())) {
		std::vector<InformationState> states = generateSubStates(model.worldCardinality() - 1, i);
		const auto is_not_empty_or_supports_expression = [&](const InformationState& state) -> bool {
			const auto result = supports(state, expr, model);
			if (!result.has_value()) {
				throw std::runtime_error(result.error());
			}
			const bool is_coherent= !state.empty() && result.value();
			if (!is_coherent) {
				logger->log(std::format("Formula is incoherent due to the following information state:\n{}", str(state, false)));
			}
			return is_coherent; 
		};
		try {
			if (!std::ranges::any_of(states, is_not_empty_or_supports_expression)) {
				logger->log("Evaluation result: False");
				return false;
			}
		}
		catch (const std::runtime_error& e) {
			const std::string error_message = e.what();
			logger->log(std::format("Evaluation failed with the following error:\n{}", error_message));
			return std::unexpected(error_message);
		}
	}
	logger->log("Evaluation result: True");
	return true;
}

/**
 * @brief An implementation of GSV's logical consequence relation.
 *
 * This function is an alias for `entails_G()`.
 *
 * @param premises A vector of expressions representing the premises.
 * @param conclusion The expression representing the conclusion.
 * @param model The model against which entailment is evaluated.
 * @return std::expected<bool, std::string> `true` if the conclusion is supported
 *         in all states updated by the premises, `false` otherwise.
 *         Returns an error message if evaluation fails.
 */
std::expected<bool, std::string> entails(const std::vector<QMLExpression::Expression>& premises, const QMLExpression::Expression& conclusion, const IModel& model, GSVLogger* logger)
{
	logger = normalize(logger);

	return entails_G(premises, conclusion, model, logger);
}

namespace {
	std::expected<void, std::string> sequentiallyUpdate(InformationState& state, const std::vector<QMLExpression::Expression>& expressions, const IModel& model)
	{
		for (const QMLExpression::Expression& expr : expressions) {
			const auto update = evaluate(expr, state, model);
			if (!update.has_value()) {
				return std::unexpected(explain_failure(std::visit(QMLExpression::Formatter(), QMLExpression::Expression(expr)), update.error()));
			}
			state = update.value();
		}
		return {};
	}
}

/**
 * @brief An implementation of Veltmann's Update Semantics' logical consequence relation at the ignorant state.
 *
 * This function determines whether the state that results from sequentially updating the
 * ignorant state with the premises supports the conclusion (provided the update with the
 * conclusion is defined).
 *  
 * The 0 subscript encodes the fact that the entailment relation is evaluated relative
 * to the ignorant state only.
 *
 * @param premises A vector of expressions representing the premises.
 * @param conclusion The expression representing the conclusion.
 * @param model The model against which entailment is evaluated.
 * @return std::expected<bool, std::string> `true` if the conclusion is supported
 *         in all states updated by the premises, `false` otherwise.
 *         Returns an error message if evaluation fails.
 */
std::expected<bool, std::string> entails_0(const std::vector<QMLExpression::Expression>& premises, const QMLExpression::Expression& conclusion, const IModel& model, GSVLogger* logger)
{
	const std::string formula = std::visit(QMLExpression::Formatter(), QMLExpression::Expression(conclusion));
	std::string premise_formulas;

	if (!premises.empty()) {
		for (const auto& premise : premises) {
			premise_formulas += std::visit(QMLExpression::Formatter(), QMLExpression::Expression(premise)) + ", ";
		}
		premise_formulas.resize(premise_formulas.size() - 2);
	}

	logger = normalize(logger);
	logger->log(std::format("Evaluating entailment relative to the ignorant state\n- Premises: {}\n- Conclusion: {}", premise_formulas, formula));

	InformationState ignorant_state = create(model);

	// update input state with premises
	const auto sequential_update = sequentiallyUpdate(ignorant_state, premises, model);
	if (!sequential_update.has_value()) {
		const std::string error_message = sequential_update.error();
		logger->log(std::format("Evaluation failed with the following error:\n{}", error_message));
		return std::unexpected(error_message);
	}

	// check if update with conclusion exists
	const auto conclusion_update = evaluate(conclusion, ignorant_state, model);
	if (!conclusion_update.has_value()) {
		const std::string error_message = conclusion_update.error();
		logger->log(std::format("Evaluation failed with the following error:\n{}", error_message));
		return std::unexpected(error_message);
	}

	// update exists, check for support
	const auto does_support = supports(ignorant_state, conclusion, model);
	if (!does_support.has_value()) {
		const std::string error_message = does_support.error();
		logger->log(std::format("Evaluation failed with the following error:\n{}", error_message));
		return std::unexpected(error_message);
	}

	const bool entailment_holds = does_support.value();
	const std::string result_string = entailment_holds ? "True" : "False";
	logger->log(std::format("Evaluation result: {}", result_string));
	return entailment_holds;
}

/**
 * @brief An implementation of Veltmann's Update Semantics' logical consequence relation at every state.
 *
 * This function iterates through all possible information states definable relative to
 * the base model, and determines whether, for each of them, the state that results from
 * sequentially updating that state with the premises supports the conclusion
 * (provided the update with the conclusion is defined). It this holds, the entailment
 * relation holds. Otherwise, it fails to hold.
 *
 * The G subscript (for general) encodes the fact that the entailment relation is evaluated
 * relative to every possible information state.
 * 
 * @param premises A vector of expressions representing the premises.
 * @param conclusion The expression representing the conclusion.
 * @param model The model against which entailment is evaluated.
 * @return std::expected<bool, std::string> `true` if the conclusion is supported
 *         in all states updated by the premises, `false` otherwise.
 *         Returns an error message if evaluation fails.
 */
std::expected<bool, std::string> entails_G(const std::vector<QMLExpression::Expression>& premises, const QMLExpression::Expression& conclusion, const IModel& model, GSVLogger* logger)
{
	const std::string formula = std::visit(QMLExpression::Formatter(), QMLExpression::Expression(conclusion));
	std::string premise_formulas;

	if (!premises.empty()) {
		for (const auto& premise : premises) {
			premise_formulas += std::visit(QMLExpression::Formatter(), QMLExpression::Expression(premise)) + ", ";
		}
		premise_formulas.resize(premise_formulas.size() - 2);
	}

	logger = normalize(logger);
	logger->log(std::format("Evaluating entailment relative to every state\n- Premises: {}\n- Conclusion: {}", premise_formulas, formula));

	for (const int i : std::views::iota(0, model.worldCardinality())) {
		std::vector<InformationState> states = generateSubStates(model.worldCardinality() - 1, i);
		for (InformationState& input_state : states) {
			// update input state with premises
			const auto sequential_update = sequentiallyUpdate(input_state, premises, model);
			if (!sequential_update.has_value()) {
				const std::string error_message = sequential_update.error();
				logger->log(std::format("Evaluation failed with the following error:\n{}", error_message));
				return std::unexpected(error_message);
			}

			// check if update with conclusion exists
			const auto conclusion_update = evaluate(conclusion, input_state, model);
			if (!conclusion_update.has_value()) {
				const std::string error_message = conclusion_update.error();
				logger->log(std::format("Evaluation failed with the following error:\n{}", error_message));
				return std::unexpected(error_message);
			}

			// update exists, check for support
			const auto does_support = supports(input_state, conclusion, model);
			if (!does_support.has_value()) {
				const std::string error_message = does_support.error();
				logger->log(std::format("Evaluation failed with the following error:\n{}", error_message));
				return std::unexpected(error_message);
			}
			if (!does_support.value()) {
				logger->log(std::format("The following information state provides a counterexample to the argument:\n{}", str(input_state, false)));
				logger->log("Evaluation result: False");
				return false;
			}
		}
	}
	logger->log("Evaluation result: True");
	return true;
}

/**
 * @brief An implementation of Veltmann's Update Semantics' entailment-as-support logical consequence relation.
 *
 * This function iterates through all possible information states definable relative to
 * the base model, and determines whether they either fail to satisfy some of the premises,
 * or they satisfy the conclusion. If this holds, the entailment relation holds. Otherwise,
 * it fails to hold.
 * 
 * The C subscript (for classical) encodes the fact that the entailment relation is the
 * one that resembles the most the classical notion of entailment, in which the premises
 * have no dynamic effect on the context at which the conclusion is evaluated.
 *
 * @param premises A vector of expressions representing the premises.
 * @param conclusion The expression representing the conclusion.
 * @param model The model against which entailment is evaluated.
 * @return std::expected<bool, std::string> `true` if the conclusion is supported
 *         in all states that support the premises, `false` otherwise.
 *         Returns an error message if evaluation fails.
 */
std::expected<bool, std::string> entails_C(const std::vector<QMLExpression::Expression>& premises, const QMLExpression::Expression& conclusion, const IModel& model, GSVLogger* logger)
{
	const std::string formula = std::visit(QMLExpression::Formatter(), QMLExpression::Expression(conclusion));
	std::string premise_formulas;

	if (!premises.empty()) {
		for (const auto& premise : premises) {
			premise_formulas += std::visit(QMLExpression::Formatter(), QMLExpression::Expression(premise)) + ", ";
		}
		premise_formulas.resize(premise_formulas.size() - 2);
	}

	logger = normalize(logger);
	logger->log(std::format("Evaluating entailment as support relative to every state\n- Premises: {}\n- Conclusion: {}", premise_formulas, formula));

	for (const int i : std::views::iota(0, model.worldCardinality())) {
		std::vector<InformationState> states = generateSubStates(model.worldCardinality() - 1, i);
		for (InformationState& input_state : states) {
			//go through every premise and check for support
			bool supports_every_premise = true;

			for (const auto& premise : premises) {
				const auto supports_premise = supports(input_state, premise, model);
				if (!supports_premise.has_value()) {
					const std::string error_message = supports_premise.error();
					logger->log(std::format("Evaluation failed with the following error:\n{}", error_message));
					return std::unexpected(error_message);
				}
				if (!supports_premise.value()) {
					supports_every_premise = false;
					break;
				}
			}

			// if state does not support every permise, not a counterexample (continue to next iteration)
			if (!supports_every_premise) {
				continue;
			}

			// check whether state supports conclusion
			const auto result = supports(input_state, conclusion, model);
			if (!result.has_value()) {
				const std::string error_message = result.error();
				logger->log(std::format("Evaluation failed with the following error:\n{}", error_message));
				return std::unexpected(error_message);
			}

			// if it does, go to next iteration (not a counterexample)
			if (result.value()) {
				continue;
			}

			// if it does not, return false
			logger->log(std::format("The following information state provides a counterexample to the argument:\n{}", str(input_state, false)));
			logger->log("Evaluation result: False");
			return false;
		}
	}
	// if we get here, no state is a counterexample, return true
	logger->log("Evaluation result: True");
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
 * @brief Determines whether two expressions are logically equivalent, relative to a given model.
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
std::expected<bool, std::string> equivalent(const QMLExpression::Expression& expr1, const QMLExpression::Expression& expr2, const IModel& model, GSVLogger* logger)
{
	const std::string formula1 = std::visit(QMLExpression::Formatter(), QMLExpression::Expression(expr1));
	const std::string formula2 = std::visit(QMLExpression::Formatter(), QMLExpression::Expression(expr2));

	logger = normalize(logger);
	logger->log(std::format("Evaluating equivalence between\n- LHS formula: {}\n- RHS formula: {}", formula1, formula2));

	for (const int i : std::views::iota(0, model.worldCardinality())) {
		std::vector<InformationState> states = generateSubStates(model.worldCardinality() - 1, i);

		const auto dissimilar_updates = [&](const InformationState& state) ->bool { 
			const auto expr1_update = evaluate(expr1, state, model);
			if (!expr1_update.has_value()) {
				throw std::out_of_range(expr1_update.error());
			}
			const auto expr2_update = evaluate(expr2, state, model);
			if (!expr2_update.has_value()) {
				throw std::out_of_range(expr2_update.error());
			}
			const bool comparison_result = !similar(expr1_update.value(), expr2_update.value());
			if (comparison_result) {
				logger->log(std::format("The following information state provides a counterexample to the equivalence:\n{}", str(state, false)));
			}
			return comparison_result;
		};

		try {
			if (std::ranges::any_of(states, dissimilar_updates)) {
				logger->log("Evaluation result: False");
				return false;
			}
		}
		catch (const std::out_of_range& e) {
			const std::string error_message = e.what();
			logger->log(std::format("Evaluation failed with the following error:\n{}", error_message));
			return std::unexpected(error_message);
		}
	}

	logger->log("Evaluation result: True");
	return true;
}

}
