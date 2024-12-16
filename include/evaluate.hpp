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

constexpr int SCORE_FIVE        = 1000000;
constexpr int SCORE_OPEN_FOUR   = 10000;
constexpr int SCORE_CLOSE_FOUR  = 2000;
constexpr int SCORE_OPEN_THREE  = 5000;
constexpr int SCORE_CLOSE_THREE = 500;
constexpr int SCORE_OPEN_TWO    = 200;


GameSet isWin(const BitBoard& computer, const BitBoard& opponent, pair<int, int> put);

int evaluate(const BitBoard& computer, const BitBoard& opponent);

#endif