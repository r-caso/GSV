#pragma once

#include <set>
#include <string_view>
#include <vector>

namespace iif_sadaf::talk::GSV {

/**
 * @brief Interface for class representing a model for QML without accessiblity.
 *
 * The IModel interface defines the minimal requirements on any implementation of 
 * a QML model that works with the GSV evaluator library.
 * 
 * Any such implementation should contain four functions:
 * - a function retrieving the cardinality of the set W of worlds
 * - a function retrieving the cardinality of the domain of individuals
 * - a function that retrieves, for any possible world in W, the interpretation of a singular term at that world (represented by an `int`)
 * - a function that retrieves, for any possible world in W, the interpretation of a predicate at that world (represented by a `std::set<std::vector<int>>`)
 */
struct IModel {
public:
	virtual int world_cardinality() const = 0;
	virtual int domain_cardinality() const = 0;
	virtual int termInterpretation(std::string_view term, int world) const = 0;
	virtual const std::set<std::vector<int>>& predicateInterpretation(std::string_view predicate, int world) const = 0;
	virtual ~IModel() {};
};

}