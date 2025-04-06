#pragma once 

#include <memory>

#include <QMLModel/qml-model.hpp>

#include "imodel.hpp"

namespace iif_sadaf::talk::GSV {

/**
 * @brief Adapter class to interface with a QMLModel.
 *
 * This class adapts a QMLModel to conform to the IModel interface, providing
 * implementations for model-related operations such as retrieving world and
 * domain cardinalities, and the interpretation of terms and predicates.
 */
class QMLModelAdapter : public IModel {
public:
	explicit QMLModelAdapter(const QMLModel::QMLModel& qmlModel);
	explicit QMLModelAdapter(std::unique_ptr<QMLModel::QMLModel> qmlModel);

	QMLModelAdapter(const QMLModelAdapter&) = delete;
	QMLModelAdapter& operator=(const QMLModelAdapter&) = delete;
	QMLModelAdapter(QMLModelAdapter&&) noexcept = default;
	QMLModelAdapter& operator=(QMLModelAdapter&&) noexcept = default;
	~QMLModelAdapter() override = default;

	int worldCardinality() const override;
	int domainCardinality() const override;
	std::expected<int, std::string> termInterpretation(std::string_view term, int world) const override;
	std::expected<const std::set<std::vector<int>>*, std::string> predicateInterpretation(std::string_view predicate, int world) const override;

private:
	class Impl {
	public:
		explicit Impl(const QMLModel::QMLModel& model) : ownedModel(nullptr), modelRef(&model) {}
		explicit Impl(std::unique_ptr<QMLModel::QMLModel> model)
			: ownedModel(std::move(model)), modelRef(ownedModel.get()) {
		}
		const QMLModel::QMLModel& getModel() const { return *modelRef; }

	private:
		std::unique_ptr<QMLModel::QMLModel> ownedModel;
		const QMLModel::QMLModel* modelRef;
	};
	std::unique_ptr<Impl> pImpl;
};

}