#include "common.hpp"
#include "prohibit.hpp"
#include "evaluate.hpp"
#include "csv_data.hpp"

thread_local stack<pair<int, pair<int, int>>> History;

CSVData fiveLowMASK("data/five_mask.csv");
CSVData fourOpenMask("data/four_open_mask.csv");
CSVData fourCloseMask("data/four_close_mask.csv");
CSVData threeOpenMask("data/three_open_mask.csv");
CSVData threeCloseMask("data/three_close_mask.csv");

const auto FIVE_LOW_MASK = fiveLowMASK.getData();
const auto FOUR_OPEN_MASK = fourOpenMask.getData();
const auto FOUR_CLOSE_MASK = fourCloseMask.getData();
const auto THREE_OPEN_MASK = threeOpenMask.getData();
const auto THREE_CLOSE_MASK = threeCloseMask.getData();

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

                    for (const auto& mask : FOUR_OPEN_MASK) {
                        int sarchLine = line & mask[2];
                        int emptyLine = empty & mask[2];

                        if (sarchLine == mask[0] && emptyLine == mask[1]) score += SCORE_OPEN_FOUR;
                    }

                    for (const auto& mask : FOUR_CLOSE_MASK) {
                        int sarchLine = line & mask[2];
                        int emptyLine = empty & mask[2];

                        if (sarchLine == mask[0] && emptyLine == mask[1]) score += SCORE_CLOSE_FOUR;
                    }

                    for (const auto& mask : THREE_OPEN_MASK) {
                        int sarchLine = line & mask[2];
                        int emptyLine = empty & mask[2];

                        if (sarchLine == mask[0] && emptyLine == mask[1]) score += SCORE_OPEN_THREE;
                    }

                    for (const auto& mask : THREE_CLOSE_MASK) {
                        int sarchLine = line & mask[2];
                        int emptyLine = empty & mask[2];

                        if (sarchLine == mask[0] && emptyLine == mask[1]) score += SCORE_CLOSE_THREE;
                    }
                }

            } else if (opponent.checkBit(y, x)) {

                for (const auto& [dy, dx] : DIRECTIONS) {
                    auto [line, empty] = opponent.putOutBitLine(y, x, dy, dx, -1, 5);

                    for (const auto& mask : FOUR_OPEN_MASK) {
                        int sarchLine = line & mask[2];
                        int emptyLine = empty & mask[2];

                        if (sarchLine == mask[0] && emptyLine == mask[1]) score -= SCORE_OPEN_FOUR;
                    }

                    for (const auto& mask : FOUR_CLOSE_MASK) {
                        int sarchLine = line & mask[2];
                        int emptyLine = empty & mask[2];

                        if (sarchLine == mask[0] && emptyLine == mask[1]) score -= SCORE_CLOSE_FOUR;
                    }

                    for (const auto& mask : THREE_OPEN_MASK) {
                        int sarchLine = line & mask[2];
                        int emptyLine = empty & mask[2];

                        if (sarchLine == mask[0] && emptyLine == mask[1]) score -= SCORE_OPEN_THREE;
                    }

                    for (const auto& mask : THREE_CLOSE_MASK) {
                        int sarchLine = line & mask[2];
                        int emptyLine = empty & mask[2];

                        if (sarchLine == mask[0] && emptyLine == mask[1]) score -= SCORE_CLOSE_THREE;
                    }
                }

            }
        }
    }
    return score;
}