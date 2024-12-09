#include "common.hpp"
#include "prohibited.hpp"
#include "evaluate.hpp"

bool fiveLow(const BitBoard& bitBoard, int y, int x) {
}

GameSet isWin(const BitBoard& computer, const BitBoard& opponent) {
    auto [stoneType, put] = History.top();

    if (stoneType == computer.getStone()) {
        if (fiveLow(computer, put.first, put.second)) return GameSet::WIN;
        if (isProhibited(computer,put.first, put.second)) return GameSet::LOSE;

    } else if (stoneType == opponent.getStone()) {
        if (fiveLow(opponent, put.first, put.second)) return GameSet::LOSE;
        if (isProhibited(opponent, put.first, put.second)) return GameSet::WIN;
    }


    return GameSet::CONTINUE;
}