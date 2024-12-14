#include <utility>

#include "common.hpp"
#include "alpha_beta.hpp"
#include "evaluate.hpp"

int alphaBeta(BitBoard& computer, BitBoard& opponent,
            int depth, int alpha, int beta, bool isMaximizingPlayer) {

    switch(isWin(computer, opponent)) {
        case GameSet::WIN:
            return SCORE_FIVE;
            break;
        case GameSet::LOSE:
            return -SCORE_FIVE;
            break;
        case GameSet::PROHIBITED:
            return computer.getStone() == STONE_BLACK ? -SCORE_FIVE : SCORE_FIVE;
            break;
        case GameSet::CONTINUE:
            break;
    }

    if (depth == 0) {
        return evaluate(computer, opponent);
    }

    if (isMaximizingPlayer) {
        int maxEval = -INF;

        for (const auto& [y, x] : SPIRAL_MOVES) {
            if (computer.checkEmptyBit(y, x)) {

                computer.setBit(y, x);
                History.push({computer.getStone(), {y, x}});

                int eval = alphaBeta(computer, opponent, depth - 1, alpha, beta, false);

                computer.removeBit(y, x);
                History.pop();

                maxEval = max(maxEval, eval);
                alpha = max(alpha, eval);

                if (beta <= alpha) break;
            }
        }

        return maxEval;
    } else {
        int minEval = INF;

        for (const auto& [y, x] : SPIRAL_MOVES) {
            if (opponent.checkEmptyBit(y, x)) {

                opponent.setBit(y, x);
                History.push({opponent.getStone(), {y, x}});

                int eval = alphaBeta(computer, opponent, depth - 1, alpha, beta, true);

                opponent.removeBit(y, x);
                History.pop();

                minEval = min(minEval, eval);
                beta = min(beta, eval);

                if (beta <= alpha) break;
            }
        }

        return minEval;
    }
}