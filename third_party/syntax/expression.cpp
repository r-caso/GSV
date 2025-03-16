#include "expression.hpp"

/*
* Constructors
*/
UnaryNode::UnaryNode(Operator op, Expression scope)
    : op(op), scope(std::move(scope))
{ }

BinaryNode::BinaryNode(Operator op, Expression lhs, Expression rhs)
    : op(op), lhs(std::move(lhs)), rhs(std::move(rhs))
{ }

QuantificationNode::QuantificationNode(Quantifier quantifier, Term variable, Expression scope)
    : quantifier(quantifier), variable(std::move(variable)), scope(std::move(scope))
{ }

IdentityNode::IdentityNode(Term lhs, Term rhs)
    : lhs(std::move(lhs)), rhs(std::move(rhs))
{ }

PredicationNode::PredicationNode(std::string predicate, std::vector<Term> arguments)
    : predicate(std::move(predicate)), arguments(std::move(arguments))
{ }

/*
* NON-MEMBER FUNCTIONS
*/

Expression negate(const Expression& expr)
{
    return std::make_shared<UnaryNode>(Operator::NEG, expr);
}