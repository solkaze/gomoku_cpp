
#include "common.hpp"

// AND 演算
BitLine operator&(const BitLine& lhs, const BitLine& rhs) {
    BitLine result{};
    for (size_t i = 0; i < lhs.size(); ++i) {
        result[i] = lhs[i] & rhs[i];
    }
    return result;
}

// OR 演算
BitLine operator|(const BitLine& lhs, const BitLine& rhs) {
    BitLine result{};
    for (size_t i = 0; i < lhs.size(); ++i) {
        result[i] = lhs[i] | rhs[i];
    }
    return result;
}

// XOR 演算
BitLine operator^(const BitLine& lhs, const BitLine& rhs) {
    BitLine result{};
    for (size_t i = 0; i < lhs.size(); ++i) {
        result[i] = lhs[i] ^ rhs[i];
    }
    return result;
}

// NOT 演算
BitLine operator~(const BitLine& lhs) {
    BitLine result{};
    for (size_t i = 0; i < lhs.size(); ++i) {
        result[i] = ~lhs[i];
    }
    return result;
}