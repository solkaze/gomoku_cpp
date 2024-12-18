#ifndef EVALUATE_HPP
#define EVALUATE_HPP

#include <thread>
#include <stack>
#include "common.hpp"

enum class GameSet {
    WIN,
    LOSE,
    CONTINUE
};

constexpr int SCORE_FIVE        = 100000;
constexpr int SCORE_NEAR_WIN    = 90000;
constexpr int SCORE_OPEN_FOUR   = 80000;
constexpr int SCORE_CLOSE_FOUR  = 5500;
constexpr int SCORE_OPEN_THREE  = 5000;
constexpr int SCORE_CLOSE_THREE = 500;
constexpr int SCORE_OPEN_TWO    = 100;


GameSet isWin(const BitBoard& computer, const BitBoard& opponent, pair<int, int> put);

int evaluate(const BitBoard& computer, const BitBoard& opponent);

#endif