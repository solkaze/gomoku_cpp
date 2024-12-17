#include <utility>
#include <shared_mutex>
#include <iomanip>

#include "common.hpp"
#include "alpha_beta.hpp"
#include "evaluate.hpp"

bool Survey = false;

shared_mutex globalTTMutex;

void testPrintBoard(const BitBoard& com, const BitBoard& opp) {
    cout << "   ";
    for(int i = 0; i < BOARD_SIZE; i++) {
        if(i / 10)
            printf("%d ", i / 10);
        else
            printf("  ");
    }
    cout << endl;
    cout << "   ";
    for(int i = 0; i < BOARD_SIZE; i++) {
        cout << i % 10 << " ";
    }
    cout << endl;
    for (int i = 0; i < BOARD_SIZE; i++) {
        cout << setw(2) << i << " ";
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (com.checkBit(i, j)) {
                if (com.getStone() == STONE_BLACK)
                    cout << "● ";
                else cout << "○ ";
            } else if (opp.checkBit(i, j)) {
                if (opp.getStone() == STONE_BLACK)
                    cout << "● ";
                else cout << "○ ";
            } else {
                cout << "・";
            }
        }
        cout << endl;
    }
    cout << endl;
}

int alphaBeta(BitBoard& computer, BitBoard& opponent,
            int depth, int alpha, int beta, TransportationTable& localTT, bool isMaximizingPlayer, pair<int, int> put) {

    int cachedEval;
    if (localTT.retrieveEntry(depth, alpha, beta, cachedEval, isMaximizingPlayer)) {
        return cachedEval;
    }

    switch(isWin(computer, opponent, put)) {
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

                int eval = alphaBeta(computer, opponent, depth - 1, alpha, beta, localTT, false, make_pair(y, x));

                computer.removeBit(y, x);
                localTT.updateHashKey(computer.getStone(), y, x);

                maxEval = std::max(maxEval, eval);
                alpha   = std::max(alpha, eval);

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

                int eval = alphaBeta(computer, opponent, depth - 1, alpha, beta, localTT, true, make_pair(y, x));

                opponent.removeBit(y, x);
                localTT.updateHashKey(opponent.getStone(), y, x);

                minEval = std::min(minEval, eval);
                beta    = std::min(beta, eval);

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