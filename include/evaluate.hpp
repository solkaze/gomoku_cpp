#ifndef EVALUATE_HPP
#define EVALUATE_HPP

#include "bit_board.hpp"

enum class GameSet { WIN, LOSE, CONTINUE };

enum class Advantage { COM, OPP, DRAW };

constexpr int SCORE_FIVE        = 1000000000;  // 5連
constexpr int SCORE_FOUR_OPEN   = 800000;      // 4連両端空き
constexpr int SCORE_FOUR_WIN    = 500000;      // 4,4連 4,3連
constexpr int SCORE_THREE_WIN   = 200000;      // 3,3連
constexpr int SCORE_FOUR_JUMP   = 20000;       // 4連飛び
constexpr int SCORE_FOUR_CLOSE  = 10000;       // 4連片端空き
constexpr int SCORE_THREE_OPEN  = 5000;        // 3連両端空き
constexpr int SCORE_THREE_JUMP  = 1000;        // 3連飛び
constexpr int SCORE_THREE_CLOSE = 500;         // 3連片側空き
constexpr int SCORE_TWO_OPEN    = 100;         // 2連両端空き

GameSet isWin(const BitBoard& computer, const BitBoard& opponent,
              pair<int, int> put);

int evaluate(const BitBoard& computer, const BitBoard& opponent);

Advantage checkAdvantage(int board[][BOARD_SIZE], int comStone, int oppStone,
                         int& putY, int& putX);

#endif