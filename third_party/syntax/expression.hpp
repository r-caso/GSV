#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>

struct Term {
    enum class Type : uint8_t {
        CONSTANT,
        VARIABLE
    };

    Type type;
    std::string literal;
};

struct UnaryNode;
struct BinaryNode;
struct QuantificationNode;
struct IdentityNode;
struct PredicationNode;

using Expression = std::variant<
    std::shared_ptr<UnaryNode>,
    std::shared_ptr<BinaryNode>,
    std::shared_ptr<QuantificationNode>,
    std::shared_ptr<IdentityNode>,
    std::shared_ptr<PredicationNode>
>;

enum class Operator : uint8_t {
    NEG,
    CON,
    DIS,
    IMP,
    E_NEC,
    E_POS
};

enum class Quantifier : uint8_t {
    EXISTENTIAL,
    UNIVERSAL
};

struct UnaryNode {
    UnaryNode(Operator op, Expression scope);

    Operator op;
    Expression scope;
};

struct BinaryNode {
    BinaryNode(Operator op, Expression lhs, Expression rhs);

    Operator op;
    Expression lhs;
    Expression rhs;
};

struct QuantificationNode {
    QuantificationNode(Quantifier quantifier, Term variable, Expression scope);

    Quantifier quantifier;
    Term variable;
    Expression scope;
};

struct IdentityNode {
    IdentityNode(Term lhs, Term rhs);

    Term lhs;
    Term rhs;
};

struct PredicationNode {
    PredicationNode(std::string predicate, std::vector<Term> arguments);

    std::string predicate;
    std::vector<Term> arguments;
};

Expression negate(const Expression& expr);