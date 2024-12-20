#ifndef EVALUATE_HPP
#define EVALUATE_HPP

#include "bit_board.hpp"

enum class GameSet {
    WIN,
    LOSE,
    CONTINUE
};

constexpr int SCORE_FIVE        = 1000000;
constexpr int SCORE_NEAR_WIN    = 80000;
constexpr int SCORE_OPEN_FOUR   = 90000;
constexpr int SCORE_CLOSE_FOUR  = 10000;
constexpr int SCORE_OPEN_THREE  = 6000;
constexpr int SCORE_CLOSE_THREE = 500;
constexpr int SCORE_OPEN_TWO    = 100;


GameSet isWin(const BitBoard& computer, const BitBoard& opponent, pair<int, int> put);

int evaluate(const BitBoard& computer, const BitBoard& opponent);

bool checkThreat(int board[][BOARD_SIZE], int comStone, int oppStone, int& putY, int& putX);;

#endif