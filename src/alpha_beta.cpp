#include <utility>
#include <shared_mutex>

#include "common.hpp"
#include "alpha_beta.hpp"
#include "evaluate.hpp"

shared_mutex globalTTMutex;

int alphaBeta(BitBoard& computer, BitBoard& opponent,
            int depth, int alpha, int beta, TransportationTable& localTT, bool isMaximizingPlayer) {

    int cachedEval;
    if (localTT.retrieveEntry(depth, alpha, beta, cachedEval, isMaximizingPlayer)) {
        return cachedEval;
    }

    switch(isWin(computer, opponent)) {
        case GameSet::WIN:
            return SCORE_FIVE;
            break;
        case GameSet::LOSE:
            return -SCORE_FIVE;
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
                localTT.updateHashKey(computer.getStone(), y, x);
                History.push({computer.getStone(), {y, x}});

                int eval = alphaBeta(computer, opponent, depth - 1, alpha, beta, localTT, false);

                computer.removeBit(y, x);
                localTT.updateHashKey(computer.getStone(), y, x);
                History.pop();

                maxEval = max(maxEval, eval);
                alpha = max(alpha, eval);

                if (beta <= alpha) { // ベータカット
                    localTT.storeEntry(depth, maxEval, BoundType::LOWER_BOUND);
                    return maxEval;
                }
            }
        }

        localTT.storeEntry(depth, maxEval, BoundType::EXACT);

        return maxEval;
    } else {
        int minEval = INF;

        for (const auto& [y, x] : SPIRAL_MOVES) {
            if (opponent.checkEmptyBit(y, x)) {

                opponent.setBit(y, x);
                localTT.updateHashKey(opponent.getStone(), y, x);
                History.push({opponent.getStone(), {y, x}});

                int eval = alphaBeta(computer, opponent, depth - 1, alpha, beta, localTT, true);

                opponent.removeBit(y, x);
                localTT.updateHashKey(opponent.getStone(), y, x);
                History.pop();

                minEval = min(minEval, eval);
                beta = min(beta, eval);

                if (beta <= alpha) { // アルファカット
                    localTT.storeEntry(depth, minEval, BoundType::UPPER_BOUND);
                    return minEval;
                }
            }
        }

        localTT.storeEntry(depth, minEval, BoundType::EXACT);

        return minEval;
    }
}