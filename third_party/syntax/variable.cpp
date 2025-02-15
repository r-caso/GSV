#include "variable.hpp"

#include <algorithm>
#include <unordered_map>
#include <vector>

namespace {
    std::unordered_map<uint8_t, std::unordered_map<char, uint8_t>> variable_dfa = {
        {0, { { 'x', 1 },
            { 'y', 1 },
            { 'z', 1 },
            { 'r', 1 },
            { 's', 1 },
            { 't', 1 },
            { 'u', 1 },
            { 'v', 1 },
            { 'w', 1 } }
        },
        {1, { { '_', 2 },
            { '0', 3 },
            { '1', 3 },
            { '2', 3 },
            { '3', 3 },
            { '4', 3 },
            { '5', 3 },
            { '6', 3 },
            { '7', 3 },
            { '8', 3 },
            { '9', 3 } }
        },
        {2, { { '0', 3 },
            { '1', 3 },
            { '2', 3 },
            { '3', 3 },
            { '4', 3 },
            { '5', 3 },
            { '6', 3 },
            { '7', 3 },
            { '8', 3 },
            { '9', 3 } }
        },
        {3, { { '0', 3 },
            { '1', 3 },
            { '2', 3 },
            { '3', 3 },
            { '4', 3 },
            { '5', 3 },
            { '6', 3 },
            { '7', 3 },
            { '8', 3 },
            { '9', 3 } }
        }
    };

    const std::vector<uint8_t> final_states = { 1, 3 };
    const uint8_t initial_state = 0;
}

bool isVariable(std::string_view token)
{
    uint8_t state = initial_state;
    for (const char c : token) {
        if (!variable_dfa.at(state).contains(c)) {
            return false;
        }
        state = variable_dfa.at(state).at(c);
    }
    return std::ranges::find(final_states, state) != final_states.cend();
}