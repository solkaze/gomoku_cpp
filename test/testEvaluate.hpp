#ifndef TEST_EVALUATE_HPP
#define TEST_EVALUATE_HPP

#include "testClass.hpp"

constexpr int SCORE_FIVE        = 1000000;
constexpr int SCORE_NEAR_WIN    = 80000;
constexpr int SCORE_OPEN_FOUR   = 90000;
constexpr int SCORE_CLOSE_FOUR  = 10000;
constexpr int SCORE_OPEN_THREE  = 5000;
constexpr int SCORE_CLOSE_THREE = 500;
constexpr int SCORE_OPEN_TWO    = 100;

int evaluate(const BitBoard& computer, const BitBoard& opponent);

bool checkChance(int board[][BOARD_SIZE], int comStone, int oppStone);

#endif // TEST_EVALUATE_HPP