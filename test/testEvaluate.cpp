#include "testClass.hpp"
#include "testCsv.hpp"
#include "testEvaluate.hpp"
#include <iostream>
#include <bitset>
CSVData fiveLowMASK("../data/five_mask.csv");
CSVData fourOpenMask("../data/four_open_mask.csv");
CSVData fourCloseMask("../data/four_close_mask.csv");
CSVData threeOpenMask("../data/three_open_mask.csv");
CSVData threeCloseMask("../data/three_close_mask.csv");
CSVData twoOpenMask("../data/two_open_mask.csv");

const auto FIVE_LOW_MASK    = fiveLowMASK.getData();
const auto FOUR_OPEN_MASK   = fourOpenMask.getData();
const auto FOUR_CLOSE_MASK  = fourCloseMask.getData();
const auto THREE_OPEN_MASK  = threeOpenMask.getData();
const auto THREE_CLOSE_MASK = threeCloseMask.getData();
const auto TWO_OPEN_MASK    = twoOpenMask.getData();

int evaluateLineScore(int line, int empty, const vector<RowData>& masks, const int score) {
    int totalScore = 0;
    for (const auto& mask : masks) {
        int filteredLine = line & mask.range;
        if (filteredLine != mask.stones) continue;  // 条件を満たさない場合はスキップ

        int filteredEmpty = empty & mask.range;
        if (filteredEmpty == mask.empty) {
            totalScore += score;
        }
    }
    if (totalScore > 0) {
        cout << "line: " << bitset<6>(line) << endl;
        cout << "empty: " << bitset<6>(empty) << endl;
        cout << "score: " << totalScore << endl;
    }
    return totalScore;
}



int evaluate(const BitBoard& computer, const BitBoard& opponent) {
    int score = 0;

    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            if (computer.checkBit(y, x)) {

                for (const auto& [dy, dx] : DIRECTIONS) {
                    auto [line, empty] = computer.putOutBitLine(y, x, dy, dx, -1, 5);
                    // ４連両端空き
                    score += evaluateLineScore(line, empty, FOUR_OPEN_MASK,   SCORE_OPEN_FOUR);
                    // 4連片側空き
                    score += evaluateLineScore(line, empty, FOUR_CLOSE_MASK,  SCORE_CLOSE_FOUR);
                    // 3連両端空き
                    score += evaluateLineScore(line, empty, THREE_OPEN_MASK,  SCORE_OPEN_THREE);
                    // 3連片側空き
                    score += evaluateLineScore(line, empty, THREE_CLOSE_MASK, SCORE_CLOSE_THREE);
                    // 2連両端空き
                    score += evaluateLineScore(line, empty, TWO_OPEN_MASK,    SCORE_OPEN_TWO);
                }
            } else if (opponent.checkBit(y, x)) {

                for (const auto& [dy, dx] : DIRECTIONS) {
                    auto [line, empty] = opponent.putOutBitLine(y, x, dy, dx, -1, 5);
                    // ４連両端空き
                    score -= evaluateLineScore(line, empty, FOUR_OPEN_MASK,   SCORE_OPEN_FOUR);
                    // 4連片側空き
                    score -= evaluateLineScore(line, empty, FOUR_CLOSE_MASK,  SCORE_CLOSE_FOUR);
                    // 3連両端空き
                    score -= evaluateLineScore(line, empty, THREE_OPEN_MASK,  SCORE_OPEN_THREE);
                    // 3連片側空き
                    score -= evaluateLineScore(line, empty, THREE_CLOSE_MASK, SCORE_CLOSE_THREE);
                    // 2連両端空き
                    score -= evaluateLineScore(line, empty, TWO_OPEN_MASK,    SCORE_OPEN_TWO);
                }
            }
        }
    }
    return score;
}