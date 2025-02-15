#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>

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
    QuantificationNode(Quantifier quantifier, std::string variable, Expression scope);

    Quantifier quantifier;
    std::string variable;
    Expression scope;
};

struct IdentityNode {
    IdentityNode(std::string lhs, std::string rhs);

    std::string lhs;
    std::string rhs;
};

struct PredicationNode {
    PredicationNode(std::string predicate, std::vector<std::string> arguments);

    std::string predicate;
    std::vector<std::string> arguments;
};

Expression negate(const Expression& expr);