#include "qml_model_adapter.hpp"

namespace iif_sadaf::talk::GSV {

class QMLModelAdapter::Impl {
public:
    explicit Impl(const QMLModel::QMLModel& model) : ownedModel(nullptr), modelRef(&model) {}
    
    explicit Impl(std::unique_ptr<QMLModel::QMLModel> model) 
        : ownedModel(std::move(model)), modelRef(ownedModel.get()) {}
    
    const QMLModel::QMLModel& getModel() const { return *modelRef; }

private:
    std::unique_ptr<QMLModel::QMLModel> ownedModel;
    const QMLModel::QMLModel* modelRef;
};

QMLModelAdapter::QMLModelAdapter(const QMLModel::QMLModel& QMLModelModel)
    : pImpl(std::make_unique<Impl>(QMLModelModel)) {}

QMLModelAdapter::QMLModelAdapter(std::unique_ptr<QMLModel::QMLModel> QMLModelModel)
    : pImpl(std::make_unique<Impl>(std::move(QMLModelModel))) {}

int QMLModelAdapter::world_cardinality() const
{
    return pImpl->getModel().world_cardinality();
}

int QMLModelAdapter::domain_cardinality() const
{
    return pImpl->getModel().domain_cardinality();
}

std::expected<int, std::string> QMLModelAdapter::termInterpretation(std::string_view term, int world) const
{
    return pImpl->getModel().termInterpretation(term, world);
}

std::expected<const std::set<std::vector<int>>*, std::string> QMLModelAdapter::predicateInterpretation(std::string_view predicate, int world) const
{
    return pImpl->getModel().predicateInterpretation(predicate, world);
}

}
