#include "formatter.hpp"

namespace iif_sadaf::talk::GSV {

std::string Formatter::operator()(std::shared_ptr<UnaryNode> expr) const
{
    std::string op = expr->op == Operator::NEG ? "-" :
        expr->op == Operator::E_NEC ? "L" :
        "M";

    return op + std::visit(Formatter(), expr->scope);
}

std::string Formatter::operator()(std::shared_ptr<BinaryNode> expr) const
{
    std::string op = expr->op == Operator::CON ? " & " :
        expr->op == Operator::DIS ? " v " :
        " -> ";

    return "(" + std::visit(Formatter(), expr->lhs) + op + std::visit(Formatter(), expr->rhs) + ")";
}

std::string Formatter::operator()(std::shared_ptr<QuantificationNode> expr) const
{
    std::string quantifier = expr->quantifier == Quantifier::EXISTENTIAL ? "E" : "A";

    return quantifier + expr->variable.literal + " " + std::visit(Formatter(), expr->scope);
}

std::string Formatter::operator()(std::shared_ptr<PredicationNode> expr) const
{
    std::string formula = expr->predicate + "(";

    for (const Term& arg : expr->arguments) {
        formula += arg.literal + ", ";
    }

    formula.resize(formula.size() - 2);

    return formula + ")";
}

std::string Formatter::operator()(std::shared_ptr<IdentityNode> expr) const
{
    return expr->lhs.literal + " = " + expr->rhs.literal;
}

}