#include <utility>

#include "common.hpp"
#include "alpha_beta.hpp"
#include "evaluate.hpp"

int alphaBeta(BitBoard& computer, BitBoard& opponent, 
            int depth, int alpha, int beta, bool isMaximizingPlayer) {
    if (depth == 0) {
        return evaluate(computer, opponent);
    }

    if (isMaximizingPlayer) {
        int maxEval = -INF;

        for (const auto& [y, x] : SPIRAL_MOVES) {
            if (!computer.checkBit(y, x) && !opponent.checkBit(y, x)) {

                computer.setBit(y, x);
                History.push({computer.getStone(), {y, x}});

                int eval = alphaBeta(computer, opponent, depth - 1, alpha, beta, false);

                computer.removeBit(y, x);
                History.pop();

                maxEval = max(maxEval, eval);
                alpha = max(alpha, eval);

                if (beta <= alpha) break; // Beta cut-off
            }
        }

        return maxEval;
    } else {
        int minEval = INF;

        for (const auto& [y, x] : SPIRAL_MOVES) {
            if (!computer.checkBit(y, x) && !opponent.checkBit(y, x)) {

                opponent.setBit(y, x);
                History.push({opponent.getStone(), {y, x}});

                int eval = alphaBeta(computer, opponent, depth - 1, alpha, beta, true);

                minEval = min(minEval, eval);
                beta = min(beta, eval);

                if (beta <= alpha) break;
            }
        }

        return minEval;
    }
}