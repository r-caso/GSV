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

QuantificationNode::QuantificationNode(Quantifier quantifier, std::string variable, Expression scope)
    : quantifier(quantifier), variable(std::move(variable)), scope(std::move(scope))
{ }

IdentityNode::IdentityNode(std::string lhs, std::string rhs)
    : lhs(std::move(lhs)), rhs(std::move(rhs))
{ }

PredicationNode::PredicationNode(std::string predicate, std::vector<std::string> arguments)
    : predicate(std::move(predicate)), arguments(std::move(arguments))
{ }

/*
* NON-MEMBER FUNCTIONS
*/

Expression negate(const Expression& expr)
{
    return std::make_shared<UnaryNode>(Operator::NEG, expr);
}