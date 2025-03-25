#include "evaluator.hpp"

#include <algorithm>
#include <expected>
#include <format>
#include <functional>
#include <ranges>
#include <stdexcept>

#include <QMLExpression/formatter.hpp>

#include "possibility.hpp"

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

QMLExpression::Expression negate(const QMLExpression::Expression& expr)
{
	return std::make_shared<QMLExpression::UnaryNode>(QMLExpression::Operator::NEGATION, expr);
}

} // ANONYMOUS NAMESPACE

/**
 * @brief Evaluates a unary logical expression and updates the information state accordingly.
 *
 * This function applies a unary operator (such as necessity, possibility, or negation)
 * to an expression and modifies the provided information state based on the result.
 *
 * @param expr A shared pointer to a QMLExpression::UnaryNode representing the unary expression.
 * @param params A variant containing a pair of the current InformationState and a pointer to the model (IModel).
 * @return std::expected<InformationState, std::string> The updated information state if evaluation is successful,
 *         or an error message if evaluation fails.
 *
 * @details The function first evaluates the prejacent (the inner expression of the unary operator).
 *          If evaluation fails, an error message is returned. Otherwise, it applies the appropriate
 *          modification to the information state:
 *          - **EPISTEMIC_POSSIBILITY**: If the prejacent state is empty, the input state is cleared.
 *          - **EPISTEMIC_NECESSITY**: If the prejacent state is not contained in the input state, the input state is cleared.
 *          - **NEGATION**: The input state is filtered to remove elements that subsist in the prejacent update.
 *
 *          If an unrecognized operator is encountered, an error message is returned.
 */
std::expected<InformationState, std::string> Evaluator::operator()(const std::shared_ptr<QMLExpression::UnaryNode>& expr, std::variant<std::pair<InformationState, const IModel*>> params) const
{
	const auto prejacent_update = std::visit(Evaluator(), expr->scope, params);

	if (!prejacent_update.has_value()) {
		return std::unexpected(
			std::format(
				"In evaluating formula {}:\n{}",
				std::visit(QMLExpression::Formatter(), QMLExpression::Expression(expr)),
				prejacent_update.error()
			)
		);
	}

	InformationState& input_state = std::get<std::pair<InformationState, const IModel*>>(params).first;

	if (expr->op == QMLExpression::Operator::EPISTEMIC_POSSIBILITY) {
		if (prejacent_update.value().empty()) {
			input_state.clear();
		}
	}
	else if (expr->op == QMLExpression::Operator::EPISTEMIC_NECESSITY) {
		if (!subsistsIn(input_state, prejacent_update.value())) {
			input_state.clear();
		}
	}
	else if (expr->op == QMLExpression::Operator::NEGATION) {
		filter(input_state, [&](const Possibility& p) -> bool { return !subsistsIn(p, prejacent_update.value()); });
	}
	else {
		return std::unexpected(
			std::format(
				"In evaluating formula {}:\n{}",
				std::visit(QMLExpression::Formatter(), QMLExpression::Expression(expr)),
				"Invalid unary operator"
			)
		);
	}

	return std::move(input_state);
}

/**
 * @brief Evaluates a binary logical expression and updates the information state accordingly.
 *
 * This function applies binary logical operators (such as conjunction, disjunction, and implication)
 * to an expression and modifies the provided information state based on the result.
 *
 * @param expr A shared pointer to a QMLExpression::BinaryNode representing the binary expression.
 * @param params A variant containing a pair of the current InformationState and a pointer to the model (IModel).
 * @return std::expected<InformationState, std::string> The updated information state if evaluation is successful,
 *         or an error message if evaluation fails.
 *
 * @details The function evaluates the left-hand side (lhs) and right-hand side (rhs) of the binary expression
 *          and modifies the information state based on the operator:
 *
 *          - **CONJUNCTION**: The lhs is evaluated first, and the resulting state is then used to evaluate the rhs.
 *          - **DISJUNCTION**: The lhs is negated and evaluated separately, then the rhs is evaluated using the
 *            negated lhs state. The final state contains possibilities present in either lhs or rhs.
 *          - **IMPLICATION**: Evaluates the lhs, then checks if every possibility in the lhs has all its
 *            descendants subsisting in the rhs update.
 *
 *          If an unrecognized operator is encountered, an error message is returned.
 *
 *          If any evaluation fails at any step, the function returns an error message indicating which part of
 *          the formula caused the failure.
 */
std::expected<InformationState, std::string> Evaluator::operator()(const std::shared_ptr<QMLExpression::BinaryNode>& expr, std::variant<std::pair<InformationState, const IModel*>> params) const
{
	const IModel* model = (std::get<std::pair<InformationState, const IModel*>>(params)).second;

	// Conjunction is sequential update, treated separately
	if (expr->op == QMLExpression::Operator::CONJUNCTION) {
		const auto lhs_update = std::visit(Evaluator(), expr->lhs, params);

		if (!lhs_update.has_value()) {
			return std::unexpected(
				std::format(
					"In evaluating formula {}:\n{}",
					std::visit(QMLExpression::Formatter(), QMLExpression::Expression(expr)), 
					lhs_update.error()
				)
			);
		}

		return std::visit(
				Evaluator(),
				expr->rhs,
				std::variant<std::pair<InformationState, const IModel*>>(std::make_pair(lhs_update.value(), model)
			)
		);
	}

	// All other updates are filtering updates
	InformationState& input_state = (std::get<std::pair<InformationState, const IModel*>>(params)).first;
	const auto hypothetical_lhs_update = std::visit(Evaluator(), expr->lhs, params);

	if (!hypothetical_lhs_update.has_value()) {
		return std::unexpected(
			std::format(
				"In evaluating formula {}:\n{}",
				std::visit(QMLExpression::Formatter(), QMLExpression::Expression(expr)),
				hypothetical_lhs_update.error()
			)
		);
	}

	if (expr->op == QMLExpression::Operator::DISJUNCTION) {
		const auto negated_lhs_update = std::visit(Evaluator(), negate(expr->lhs), params);
		
		if (!negated_lhs_update.has_value()) {
			return std::unexpected(
				std::format(
					"In evaluating formula {}:\n{}", 
					std::visit(QMLExpression::Formatter(), QMLExpression::Expression(expr)),
					negated_lhs_update.error()
				)
			);
		}
		
		const auto hypothetical_rhs_update = std::visit(
			Evaluator(),
			expr->rhs,
			std::variant<std::pair<InformationState, const IModel*>>(std::make_pair(negated_lhs_update.value(), model))
		);

		if (!hypothetical_rhs_update.has_value()) {
			return std::unexpected(
				std::format(
					"In evaluating formula {}:\n{}", 
					std::visit(QMLExpression::Formatter(), QMLExpression::Expression(expr)),
					hypothetical_rhs_update.error()
				)
			);
		}

		const auto in_lhs_or_in_rhs = [&](const Possibility& p) -> bool {
			return hypothetical_lhs_update.value().contains(p) || hypothetical_rhs_update.value().contains(p);
		};

		filter(input_state, in_lhs_or_in_rhs);
	}
	else if (expr->op == QMLExpression::Operator::CONDITIONAL) {
		const auto hypothetical_consequent_update = std::visit(
			Evaluator(),
			expr->rhs,
			std::variant<std::pair<InformationState, const IModel*>>(std::make_pair(hypothetical_lhs_update.value(), model))
		);

		if (!hypothetical_consequent_update.has_value()) {
			return std::unexpected(
				std::format(
					"In evaluating formula {}:\n{}", 
					std::visit(QMLExpression::Formatter(), QMLExpression::Expression(expr)), 
					hypothetical_consequent_update.error()
				)
			);
		}

		const auto all_descendants_subsist = [&](const Possibility& p) -> bool {
			const auto not_descendant_or_subsists = [&](const Possibility& p_star) -> bool {
				return !isDescendantOf(p_star, p, hypothetical_lhs_update.value()) || subsistsIn(p_star, hypothetical_consequent_update.value());
			};
			return std::ranges::all_of(hypothetical_lhs_update.value(), not_descendant_or_subsists);
		};

		const auto if_subsists_all_descendants_do = [&](const Possibility& p) -> bool {
			return !subsistsIn(p, hypothetical_lhs_update.value()) || all_descendants_subsist(p);
		};

		filter(input_state, if_subsists_all_descendants_do);
	}
	else {
		return std::unexpected(
			std::format(
				"In evaluating formula {}:\n{}",
				std::visit(QMLExpression::Formatter(), QMLExpression::Expression(expr)),
				"Invalid operator for binary formula"
			)
		);
	}

	return std::move(input_state);
}

/**
 * @brief Evaluates a quantified logical expression and updates the information state accordingly.
 *
 * This function processes logical quantification (existential or universal) over a variable,
 * applying the quantifier's scope to all possible values in the model's domain.
 *
 * @param expr A shared pointer to the `QuantificationNode` representing the quantified expression.
 * @param params A variant containing the current `InformationState` and a pointer to the `IModel`.
 * @return std::expected<InformationState, std::string> The updated information state after
 *         applying quantification, or an error message if evaluation fails.
 *
 * @details
 * - **Existential Quantification**: Evaluates the scope of the quantifier for all values in the domain,
 *   then merges all resulting information states.
 * - **Universal Quantification**: Evaluates the scope of the quantifier for all values in the domain and filters
 *   the input state, keeping only those possibilities that subsist in all hypothetical updates.
 * - If an error occurs during evaluation (e.g., invalid quantifier or undefined term),
 *   an error message is returned instead of an updated state.
 */
std::expected<InformationState, std::string> Evaluator::operator()(const std::shared_ptr<QMLExpression::QuantificationNode>& expr, std::variant<std::pair<InformationState, const IModel*>> params) const
{
	InformationState& input_state = (std::get<std::pair<InformationState, const IModel*>>(params)).first;
	const IModel* model = (std::get<std::pair<InformationState, const IModel*>>(params)).second;

	if (expr->quantifier == QMLExpression::Quantifier::EXISTENTIAL) {
		std::vector<InformationState> all_state_variants;

        for (const int i : std::views::iota(0, model->domain_cardinality())) {
            const InformationState s_variant = update(input_state, expr->variable.literal, i);
			const auto hypothetical_s_variant_update = std::visit(
				Evaluator(),
				expr->scope,
				std::variant<std::pair<InformationState, const IModel*>>(std::make_pair(s_variant, model))
			);
			
			if (!hypothetical_s_variant_update.has_value()) {
				return std::unexpected(
					std::format(
						"In evaluating formula {}:\n{}",
						std::visit(QMLExpression::Formatter(), QMLExpression::Expression(expr)),
						hypothetical_s_variant_update.error()
					)
				);
			}
			
			all_state_variants.push_back(hypothetical_s_variant_update.value());
		}

		InformationState output;
		for (const auto& state_variant : all_state_variants) {
			for (const auto& p : state_variant) {
				output.insert(p);
			}
		}

		return output;
	}
    if (expr->quantifier == QMLExpression::Quantifier::UNIVERSAL) {
		std::vector<InformationState> all_hypothetical_updates;

        for (const int d : std::views::iota(0, model->domain_cardinality())) {
            const auto hypothetical_update = std::visit(
				Evaluator(),
				expr->scope,
				std::variant<std::pair<InformationState, const IModel*>>(std::make_pair(update(input_state, expr->variable.literal, d), model))
			);

			if (!hypothetical_update.has_value()) {
				return std::unexpected(
					std::format(
						"In evaluating formula {}:\n{}", 
						std::visit(QMLExpression::Formatter(), QMLExpression::Expression(expr)), 
						hypothetical_update.error()
					)
				);
			}

			all_hypothetical_updates.push_back(hypothetical_update.value());
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
		return std::unexpected(
			std::format(
				"In evaluating formula {}:\n{}",
				std::visit(QMLExpression::Formatter(), QMLExpression::Expression(expr)),
				"Invalid quantifier"
			)
		);
	}

	return std::move(input_state);
}

/**
 * @brief Evaluates an identity expression and filters the information state accordingly.
 *
 * This function determines whether two terms (variables or constants) in the given expression
 * denote the same entity within the provided model and information state. It then filters
 * the information state, retaining only those possibilities where the denotations match.
 *
 * @param expr A shared pointer to an IdentityNode representing the identity expression.
 * @param params A variant containing a pair of the current InformationState and a pointer to the model (IModel).
 * @return std::expected<InformationState, std::string> The updated information state if evaluation is successful,
 *         or an error message if evaluation fails.
 *
 * @details The function follows these steps:
 *          1. Extracts the left-hand side (lhs) and right-hand side (rhs) terms from the expression.
 *          2. Determines the denotation of each term:
 *             - If the term is a variable, its denotation is obtained from the current possibility.
 *             - If the term is a constant, its interpretation is retrieved from the model.
 *          3. Compares the denotations to check for identity.
 *          4. Filters the information state, retaining only those possibilities where the lhs and rhs
 *             have the same denotation.
 *
 *          If a denotation is out of range (e.g., an unbound variable), an error message is returned.
 */
std::expected<InformationState, std::string> Evaluator::operator()(const std::shared_ptr<QMLExpression::IdentityNode>& expr, std::variant<std::pair<InformationState, const IModel*>> params) const
{
	InformationState& input_state = (std::get<std::pair<InformationState, const IModel*>>(params)).first;
	const IModel& model = *(std::get<std::pair<InformationState, const IModel*>>(params)).second;

	auto assigns_same_denotation = [&](const Possibility& p) -> bool {
		const auto lhs_denotation = expr->lhs.type == QMLExpression::Term::Type::VARIABLE ? variableDenotation(expr->lhs.literal, p) : model.termInterpretation(expr->lhs.literal, p.world);
		const auto rhs_denotation = expr->rhs.type == QMLExpression::Term::Type::VARIABLE ? variableDenotation(expr->rhs.literal, p) : model.termInterpretation(expr->rhs.literal, p.world);

		if (!lhs_denotation.has_value()) {
			throw std::out_of_range(lhs_denotation.error());
		}
		if (!rhs_denotation.has_value()) {
			throw std::out_of_range(rhs_denotation.error());
		}
		
		return lhs_denotation.value() == rhs_denotation.value();
	};

	try {
		filter(input_state, assigns_same_denotation);
		return std::move(input_state);
	}
	catch (const std::out_of_range& e) {
		return std::unexpected(
			std::format(
				"In evaluating formula {}:\n{}",
				std::visit(QMLExpression::Formatter(), QMLExpression::Expression(expr)),
				e.what()
			)
		);
	}
}

/**
 * @brief Evaluates a predication expression and filters the information state accordingly.
 *
 * This function checks whether a given predicate holds for a set of terms (variables or constants)
 * in each possibility of the current information state. It retains only those possibilities where
 * the predicate applies to the corresponding denotations.
 *
 * @param expr A shared pointer to a PredicationNode representing the predication expression.
 * @param params A variant containing a pair of the current InformationState and a pointer to the model (IModel).
 * @return std::expected<InformationState, std::string> The updated information state if evaluation is successful,
 *         or an error message if evaluation fails.
 *
 * @details The function performs the following steps:
 *          1. Extracts the arguments of the predicate and determines their denotations:
 *             - If an argument is a variable, its denotation is obtained from the current possibility.
 *             - If an argument is a constant, its interpretation is retrieved from the model.
 *          2. Constructs a tuple of these denotations.
 *          3. Checks if the tuple belongs to the extension of the predicate in the given world.
 *          4. Filters the information state, keeping only those possibilities where the predicate holds.
 *
 *          If an argument's denotation is out of range (e.g., an unbound variable) or the predicate
 *          interpretation is missing, an error message is returned.
 */
std::expected<InformationState, std::string> Evaluator::operator()(const std::shared_ptr<QMLExpression::PredicationNode>& expr, std::variant<std::pair<InformationState, const IModel*>> params) const
{
	InformationState& input_state = (std::get<std::pair<InformationState, const IModel*>>(params)).first;
	const IModel& model = *(std::get<std::pair<InformationState, const IModel*>>(params)).second;

	const auto tuple_in_extension = [&](const Possibility& p) -> bool {
		std::vector<int> tuple;
		
		for (const QMLExpression::Term& argument : expr->arguments) {
			const auto denotation = argument.type == QMLExpression::Term::Type::VARIABLE ? variableDenotation(argument.literal, p) : model.termInterpretation(argument.literal, p.world);
			if (denotation.has_value()) {
				tuple.push_back(denotation.value());
			}
			else {
				throw std::out_of_range(denotation.error());
			}
		}

		const auto predint = model.predicateInterpretation(expr->predicate, p.world);
		if (predint.has_value()) {
			return predint.value()->contains(tuple);
		}
		else {
			throw std::out_of_range(predint.error());
		}
	};

	try {
		filter(input_state, tuple_in_extension);
		return std::move(input_state);
	}
	catch (const std::out_of_range& e) {
		return std::unexpected(
			std::format(
				"In evaluating formula {}:\n{}",
				std::visit(QMLExpression::Formatter(), QMLExpression::Expression(expr)),
				e.what()
			)
		);
	}
}

/**
 * @brief Evaluates a logical expression within a given information state, relative to a base model.
 *
 * This function applies an `Evaluator` visitor to the provided expression, computing
 * an updated information state based on the evaluation result.
 *
 * @param expr The logical expression to evaluate.
 * @param input_state The initial information state in which the expression is evaluated.
 * @param model The model providing the interpretation of terms and predicates.
 * @return std::expected<InformationState, std::string> The updated information state if
 *         evaluation is successful, or an error message if evaluation fails.
 *
 * @details The function processes the expression using `std::visit`, dispatching to the
 *          appropriate `Evaluator` method based on the expression type. If evaluation
 *          encounters an error (e.g., an invalid operator or undefined term interpretation),
 *          an error message is returned instead of an updated state.
 */
std::expected<InformationState, std::string> evaluate(const QMLExpression::Expression& expr, const InformationState& input_state, const IModel& model)
{
	return std::visit(
		Evaluator(),
		expr,
		std::variant<std::pair<InformationState, const IModel*>>(std::make_pair(input_state, &model))
	);
}

}
