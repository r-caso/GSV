#include "qml_model_adapter.hpp"

namespace iif_sadaf::talk::GSV {

/**
 * @brief Constructs an adapter for an existing QMLModel instance.
 *
 * @param qmlModel Reference to an existing QMLModel object.
 */
QMLModelAdapter::QMLModelAdapter(const QMLModel::QMLModel& QMLModelModel)
    : pImpl(std::make_unique<Impl>(QMLModelModel)) {}

/**
 * @brief Constructs an adapter that takes ownership of a QMLModel instance.
 *
 * @param qmlModel A unique pointer to a QMLModel instance.
 */
QMLModelAdapter::QMLModelAdapter(std::unique_ptr<QMLModel::QMLModel> QMLModelModel)
    : pImpl(std::make_unique<Impl>(std::move(QMLModelModel))) {}

/**
 * @brief Retrieves the cardinality of the model's world.
 *
 * @return The number of worlds in the model.
 */
int QMLModelAdapter::worldCardinality() const
{
    return pImpl->getModel().world_cardinality();
}

/**
 * @brief Retrieves the domain cardinality for a given model.
 *
 * @return The number of individuals in the domain of the model.
 */
int QMLModelAdapter::domainCardinality() const
{
    return pImpl->getModel().domain_cardinality();
}

/**
 * @brief Retrieves the interpretation of a given term within a specified world.
 *
 * @param term The term to be interpreted.
 * @param world The world in which the term is interpreted.
 * @return An expected value containing the interpretation result or an error message.
 */
std::expected<int, std::string> QMLModelAdapter::termInterpretation(std::string_view term, int world) const
{
    return pImpl->getModel().termInterpretation(term, world);
}

/**
 * @brief Interprets a given predicate within a specified world.
 *
 * @param predicate The predicate to be interpreted.
 * @param world The world in which the predicate is interpreted.
 * @return An expected value containing a pointer to a set of vector<int> results
 *         or an error message.
 */
std::expected<const std::set<std::vector<int>>*, std::string> QMLModelAdapter::predicateInterpretation(std::string_view predicate, int world) const
{
    return pImpl->getModel().predicateInterpretation(predicate, world);
}

}
