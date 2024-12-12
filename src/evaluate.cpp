#include "common.hpp"
#include "prohibited.hpp"
#include "evaluate.hpp"
#include "csv_data.hpp"

thread_local stack<pair<int, pair<int, int>>> History;

CSVData fiveLowMASK("data/five_mask.csv");

const auto FIVE_LOW_MASK = fiveLowMASK.getData();

bool fiveLow(const BitBoard& bitBoard, const int y, const int x) {

    for(const auto& [dy, dx] : DIRECTIONS) {
        auto [line, _ ] = bitBoard.putOutBitLine(y, x, dy, dx, -4, 4);

        for (const auto& mask : FIVE_LOW_MASK) {
            int sarchLine = line & mask[2];

            if (sarchLine == mask[0]) return true;
        }
    }
    return false;
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

int evaluate(const BitBoard& computer, const BitBoard& opponent) {
    return 0;
}