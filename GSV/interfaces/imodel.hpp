#pragma once

#include <expected>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace iif_sadaf::talk::GSV {

/**
 * @brief Interface for class representing a model for Quantified Modal Logic.
 *
 * The IModel interface defines the minimal requirements on any implementation of 
 * a QML model that works with the GSV evaluator library.
 * 
 * Any such implementation should contain four functions:
 * - a function retrieving the cardinality of the set W of worlds
 * - a function retrieving the cardinality of the domain of individuals
 * - a function that retrieves, for any possible world in the model, the interpretation of a singular term at that world (represented by an `int`), and returns an error message if the term is not interpreted in the model
 * - a function that retrieves, for any possible world in the model, the interpretation of a predicate at that world (represented by a `std::set<std::vector<int>>`), and returns an error message if the predicate is not interpreted in the model
 */
struct IModel {
public:
	virtual int worldCardinality() const = 0;
	virtual int domainCardinality() const = 0;
	virtual std::expected<int, std::string> termInterpretation(std::string_view term, int world) const = 0;
	virtual std::expected<const std::set<std::vector<int>>*, std::string> predicateInterpretation(std::string_view predicate, int world) const = 0;
	virtual ~IModel() {};
};

}