#ifndef OPERATER_HPP
#define OPERATER_HPP

#include "common.hpp"

// AND演算
BitLine operator&(const BitLine& lhs, const BitLine& rhs);

// OR演算
BitLine operator|(const BitLine& lhs, const BitLine& rhs);

// XOR演算
BitLine operator^(const BitLine& lhs, const BitLine& rhs);

// NOT演算
BitLine operator~(const BitLine& lhs);

#endif