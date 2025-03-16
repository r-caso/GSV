#pragma once

#include <expected>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "imodel.hpp"

namespace GSV = iif_sadaf::talk::GSV;

struct Model : GSV::IModel{
public:
	Model(int worlds, int individuals);

	int world_cardinality() const override;
	int domain_cardinality() const override;
	std::expected<int, std::string> termInterpretation(std::string_view term, int world) const override;
	std::expected<const std::set<std::vector<int>>*, std::string> predicateInterpretation(std::string_view predicate, int world) const override;

	int worlds;
	int individuals;

	std::unordered_map<std::string_view, std::unordered_map<int, int>> m_termInterpretation = {};
	std::unordered_map<std::string_view, std::unordered_map<int, std::set<std::vector<int>>>> m_predicateInterpretation = {};
};

std::string str(const Model& m);