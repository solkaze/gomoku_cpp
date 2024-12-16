#ifndef TEST_EVALUATE_HPP
#define TEST_EVALUATE_HPP

#include "testClass.hpp"

constexpr int SCORE_FIVE        = 1000000;
constexpr int SCORE_OPEN_FOUR   = 10000;
constexpr int SCORE_CLOSE_FOUR  = 1000;
constexpr int SCORE_OPEN_THREE  = 5000;
constexpr int SCORE_CLOSE_THREE = 500;
constexpr int SCORE_OPEN_TWO    = 500;

int evaluate(const BitBoard& computer, const BitBoard& opponent);

#endif // TEST_EVALUATE_HPP