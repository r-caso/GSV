#include "semantic_relations.hpp"

namespace iif_sadaf::talk::GSV {

/**
 * @brief Retrieves the denotation of a term in a given world within a model.
 *
 * @param term The term to be interpreted.
 * @param w The world in which the term is interpreted.
 * @param m The model containing the interpretation.
 * @return The assigned individual for the term in the given world.
 * @throws std::out_of_range if the term does not exist.
 */
int termDenotation(std::string_view term, int w, const IModel& m)
{
	return m.termInterpretation(term, w);
}

/**
 * @brief Retrieves the denotation of a predicate in a given world within a model.
 *
 * @param predicate The predicate to be interpreted.
 * @param w The world in which the predicate is interpreted.
 * @param m The model containing the interpretation.
 * @return The set of tuples representing the predicate's interpretation.
 * @throws std::out_of_range if the predicate does not exist.
 */
const std::set<std::vector<int>>& predicateDenotation(std::string_view predicate, int w, const IModel& m)
{
	return m.predicateInterpretation(predicate, w);
}

/**
 * @brief Retrieves the denotation of a variable in a given possibility.
 *
 * @param variable The variable to be interpreted.
 * @param p The possibility containing the variable's assignment.
 * @return The assigned individual for the variable.
 * @throws std::out_of_range if the variable has no associated peg.
 */
int variableDenotation(std::string_view variable, const Possibility& p)
{
	return p.getAssignment(p.referentSystem->value(variable));
}

}