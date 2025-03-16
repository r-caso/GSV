#include "model.hpp"

#include <format>

/**
 * @brief Constructs a Model with a given number of worlds and individuals.
 *
 * @param worlds The number of worlds in the model.
 * @param individuals The number of individuals in the model.
 */
Model::Model(int worlds, int individuals)
	: worlds(worlds)
	, individuals(individuals)
{ }

/**
 * @brief Returns the cardinality of the set of possible worlds in the model.
 *
 * @return The number of worlds in the model.
 */
int Model::world_cardinality() const
{
	return worlds;
}

/**
 * @brief Returns the cardinality of the domain of individuals in the model.
 *
 * @return The number of individuals in the model.
 */
int Model::domain_cardinality() const
{
	return individuals;
}

/**
 * @brief Retrieves the interpretation of a term in a given world.
 *
 * @param term The term to be interpreted.
 * @param world The world in which the term is interpreted.
 * @return The assigned individual for the term in the given world.
 */
std::expected<int, std::string> Model::termInterpretation(std::string_view term, int world) const
{
	if (!m_termInterpretation.contains(std::string(term))) {
		return std::unexpected(std::format("Non-existent term: {}", std::string(term)));
	}
	return m_termInterpretation.at(std::string(term)).at(world);
}

/**
 * @brief Retrieves the interpretation of a predicate in a given world.
 *
 * @param predicate The predicate to be interpreted.
 * @param world The world in which the predicate is interpreted.
 * @return The set of tuples representing the predicate's interpretation.
 */
std::expected<const std::set<std::vector<int>>*, std::string> Model::predicateInterpretation(std::string_view predicate, int world) const
{
	if (!m_predicateInterpretation.contains(std::string(predicate))) {
		return std::unexpected(std::format("Non-existent predicate: {}", std::string(predicate)));
	}
	return &m_predicateInterpretation.at(std::string(predicate)).at(world);
}

/*
* NON-MEMBER FUNCTIONS
*/

std::string str(const Model& m)
{
	return std::format(
		"World domain cardinality:      {}\nIndividual domain cardinality: {}",
		std::to_string(m.world_cardinality()),
		std::to_string(m.domain_cardinality())
	);
}