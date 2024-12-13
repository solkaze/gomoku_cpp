#include "common.hpp"
#include "prohibit.hpp"
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
    int y = put.first;
    int x = put.second;

    if (stoneType == computer.getStone()) {
        if (fiveLow(computer, y, x))      return GameSet::WIN;
        if (isProhibited(computer, y, x)) return GameSet::LOSE;

    } else if (stoneType == opponent.getStone()) {
        if (fiveLow(opponent, y, x))      return GameSet::LOSE;
        if (isProhibited(opponent, y, x)) return GameSet::WIN;
    }

    return GameSet::CONTINUE;
}

int evaluate(const BitBoard& computer, const BitBoard& opponent) {
    int score = 0;

    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            if (computer.checkBit(y, x)) {

                for (const auto& [dy, dx] : DIRECTIONS) {
                    auto [line, empty] = computer.putOutBitLine(y, x, dy, dx, -1, 5);
                }

            } else if (opponent.checkBit(y, x)) {

                for (const auto& [dy, dx] : DIRECTIONS) {
                    auto [line, empty] = opponent.putOutBitLine(y, x, dy, dx, -1, 5);
                }

            }
        }
    }
    return score;
}