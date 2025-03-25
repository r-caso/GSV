#pragma once 

#include <memory>

#include <QMLModel/qml-model.hpp>

#include "imodel.hpp"

namespace iif_sadaf::talk::GSV {

class QMLModelAdapter : public IModel {
public:
	explicit QMLModelAdapter(const QMLModel::QMLModel& qmlModel);
	explicit QMLModelAdapter(std::unique_ptr<QMLModel::QMLModel> qmlModel);

	QMLModelAdapter(const QMLModelAdapter&) = delete;
	QMLModelAdapter& operator=(const QMLModelAdapter&) = delete;
	QMLModelAdapter(QMLModelAdapter&&) noexcept = default;
	QMLModelAdapter& operator=(QMLModelAdapter&&) noexcept = default;
	~QMLModelAdapter() override = default;

	int world_cardinality() const override;
	int domain_cardinality() const override;
	std::expected<int, std::string> termInterpretation(std::string_view term, int world) const override;
	std::expected<const std::set<std::vector<int>>*, std::string> predicateInterpretation(std::string_view predicate, int world) const override;

	std::string str() const
	{
		return "";
	}

private:
	class Impl;
	std::unique_ptr<Impl> pImpl;
};

std::unique_ptr<IModel> createQMLModelAdapter(const QMLModel::QMLModel& qmlModel);
std::unique_ptr<IModel> createQMLModelAdapter(std::unique_ptr<QMLModel::QMLModel> qmlModel);
std::unique_ptr<IModel> createQMLModelAdapter(const void* modelPtr);

}