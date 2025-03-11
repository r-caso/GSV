#include "expression.hpp"

#include <string>

namespace iif_sadaf::talk::GSV {

struct Formatter {
    std::string operator()(std::shared_ptr<UnaryNode> expr) const;
    std::string operator()(std::shared_ptr<BinaryNode> expr) const;
    std::string operator()(std::shared_ptr<QuantificationNode> expr) const;
    std::string operator()(std::shared_ptr<PredicationNode> expr) const;
    std::string operator()(std::shared_ptr<IdentityNode> expr) const;
};

}