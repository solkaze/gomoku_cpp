#include <iomanip>

#include "common.hpp"
#include "prohibit.hpp"
#include "evaluate.hpp"
#include "csv_data.hpp"

CSVData fiveLowMASK("data/five_mask.csv");
CSVData fourOpenMask("data/four_open_mask.csv");
CSVData fourCloseMask("data/four_close_mask.csv");
CSVData threeOpenMask("data/three_open_mask.csv");
CSVData threeCloseMask("data/three_close_mask.csv");
CSVData twoOpenMask("data/two_open_mask.csv");

const auto FIVE_LOW_MASK    = fiveLowMASK.getData();
const auto FOUR_OPEN_MASK   = fourOpenMask.getData();
const auto FOUR_CLOSE_MASK  = fourCloseMask.getData();
const auto THREE_OPEN_MASK  = threeOpenMask.getData();
const auto THREE_CLOSE_MASK = threeCloseMask.getData();
const auto TWO_OPEN_MASK    = twoOpenMask.getData();

bool fiveLow(const BitBoard& bitBoard, const int y, const int x) {

    for(const auto& [dy, dx] : DIRECTIONS) {
        auto [line, _ ] = bitBoard.putOutBitLine(y, x, dy, dx, -4, 4);

        for (const auto& mask : FIVE_LOW_MASK) {
            int sarchLine = line & mask.range;

            if (sarchLine == mask.stones) return true;
        }
    }
    return false;
}

GameSet isWin(const BitBoard& computer, const BitBoard& opponent, pair<int, int> put) {
    int y = put.first;
    int x = put.second;

    if (computer.checkBit(y, x)) {
        if (fiveLow(computer, y, x)) {
            return GameSet::WIN;
        }
        if (computer.getStone() == STONE_BLACK && isProhibited(computer, y, x)) return GameSet::LOSE;

    } else if (opponent.checkBit(y, x)) {
        if (fiveLow(opponent, y, x)) {
            return GameSet::LOSE;
        }
        if (opponent.getStone() == STONE_BLACK && isProhibited(opponent, y, x)) return GameSet::WIN;
    }

    return GameSet::CONTINUE;
}

int evaluateLineScore(int line, int empty, const vector<RowData>& masks, const int score) {
    for (const auto& mask : masks) {
        int filteredLine = line & mask.range;
        if (filteredLine != mask.stones) continue;  // 条件を満たさない場合はスキップ

        int filteredEmpty = empty & mask.range;
        if (filteredEmpty == mask.empty) return score;
    }
    return 0;
}



int evaluate(const BitBoard& computer, const BitBoard& opponent) {
    int score = 0;

    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            if (computer.checkBit(y, x)) {

                for (const auto& [dy, dx] : DIRECTIONS) {
                    auto [line, empty] = computer.putOutBitLine(y, x, dy, dx, -1, 5);
                    int preScore = score;
                    // 2連両端空き
                    score += evaluateLineScore(line, empty, TWO_OPEN_MASK,    SCORE_OPEN_TWO);
                    if (preScore != score) continue;
                    // 3連片側空き
                    score += evaluateLineScore(line, empty, THREE_CLOSE_MASK, SCORE_CLOSE_THREE);
                    if (preScore != score) continue;
                    // 3連両端空き
                    score += evaluateLineScore(line, empty, THREE_OPEN_MASK,  SCORE_OPEN_THREE);
                    if (preScore != score) continue;
                    // 4連片側空き
                    score += evaluateLineScore(line, empty, FOUR_CLOSE_MASK,  SCORE_CLOSE_FOUR);
                    if (preScore != score) continue;
                    // ４連両端空き
                    score += evaluateLineScore(line, empty, FOUR_OPEN_MASK,   SCORE_OPEN_FOUR);
                }
            } else if (opponent.checkBit(y, x)) {

                for (const auto& [dy, dx] : DIRECTIONS) {
                    auto [line, empty] = opponent.putOutBitLine(y, x, dy, dx, -1, 5);
                    int preScore = score;
                    // 2連両端空き
                    score -= evaluateLineScore(line, empty, TWO_OPEN_MASK, SCORE_OPEN_TWO);
                    if (preScore != score) continue;
                    // 3連片側空き
                    score -= evaluateLineScore(line, empty, THREE_CLOSE_MASK, SCORE_CLOSE_THREE);
                    if (preScore != score) continue;
                    // 3連両端空き
                    score -= evaluateLineScore(line, empty, THREE_OPEN_MASK, SCORE_OPEN_THREE);
                    if (preScore != score) continue;
                    // 4連片側空き
                    score -= evaluateLineScore(line, empty, FOUR_CLOSE_MASK, SCORE_CLOSE_FOUR);
                    if (preScore != score) continue;
                    // ４連両端空き
                    score -= evaluateLineScore(line, empty, FOUR_OPEN_MASK, SCORE_OPEN_FOUR);
                }
            }
        }
    }
    return score;
}