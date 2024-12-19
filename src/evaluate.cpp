#include <iomanip>
#include <iostream>

#include "prohibit.hpp"
#include "evaluate.hpp"
#include "csv_data.hpp"

CSVData fiveLowMASK("data/five_mask.csv");
CSVData fourOpenMask("data/four_open_mask.csv");
CSVData fourCloseMask("data/four_close_mask.csv");
CSVData threeOpenMask("data/three_open_mask.csv");
CSVData threeCloseMask("data/three_close_mask.csv");
CSVData twoOpenMask("data/two_open_mask.csv");
CSVData skipMask("data/skip_mask.csv");

const auto FIVE_LOW_MASK    = fiveLowMASK.getData();
const auto FOUR_OPEN_MASK   = fourOpenMask.getData();
const auto FOUR_CLOSE_MASK  = fourCloseMask.getData();
const auto THREE_OPEN_MASK  = threeOpenMask.getData();
const auto THREE_CLOSE_MASK = threeCloseMask.getData();
const auto TWO_OPEN_MASK    = twoOpenMask.getData();
const auto SKIP_MASK        = skipMask.getData();

bool fiveLow(const BitBoard& bitBoard, const int y, const int x) {

    for(const auto& [dy, dx] : DIRECTIONS) {
        auto [line, _ ] = bitBoard.putOutBitLine(y, x, dy, dx, -4, 4);

        for (const auto& mask : FIVE_LOW_MASK) {
            uint32_t sarchLine = line & mask.range;

            if (sarchLine == mask.stones) return true;
        }
    }
    return false;
}

GameSet isWin(const BitBoard& computer, const BitBoard& opponent, pair<int, int> put) {
    int y = put.first;
    int x = put.second;

    if (computer.checkBit(y, x)) {
        if (fiveLow(computer, y, x)) return GameSet::WIN;
        if (computer.getStone() == STONE_BLACK && isProhibited(computer, y, x)) return GameSet::LOSE;

    } else if (opponent.checkBit(y, x)) {
        if (fiveLow(opponent, y, x)) return GameSet::LOSE;
        if (opponent.getStone() == STONE_BLACK && isProhibited(opponent, y, x)) return GameSet::WIN;
    }

    return GameSet::CONTINUE;
}

bool checkThreat(int board[][BOARD_SIZE], int comStone, int oppStone);

bool isOpenSequence(int board[][BOARD_SIZE], int y, int x, int dy, int dx, int stone, const vector<RowData>& masks);

bool checkThreat(int board[][BOARD_SIZE], int comStone, int oppStone) {
    bool comChanceThree = false;
    bool comChanceFour = false;
    bool oppChanceThree = false;
    bool oppChanceFour = false;
    for (int x = 0; x < BOARD_SIZE; ++x) {
        for (int y = 0; y < BOARD_SIZE; ++y) {
            // 空きセルスキップ
            if (board[y][x] == comStone) {
                for (const auto& [dy, dx] : DIRECTIONS) {
                    if (isOpenSequence(board, y, x, dy, dx, comStone, FOUR_OPEN_MASK)) comChanceFour = true;
                    if (comChanceFour) break;
                    if (isOpenSequence(board, y, x, dy, dx, comStone, FOUR_CLOSE_MASK)) comChanceFour = true;
                    if (comChanceFour) break;
                    if (isOpenSequence(board, y, x, dy, dx, comStone, THREE_OPEN_MASK)) comChanceThree = true;
                    if (comChanceThree) break;
                }
            } else if (board[y][x] == oppStone) {
                for (const auto& [dy, dx] : DIRECTIONS) {
                    if (isOpenSequence(board, y, x, dy, dx, oppStone, FOUR_OPEN_MASK)) oppChanceFour = true;
                    if (oppChanceFour) break;
                    if (isOpenSequence(board, y, x, dy, dx, oppStone, FOUR_CLOSE_MASK)) oppChanceFour = true;
                    if (oppChanceFour) break;
                    if (isOpenSequence(board, y, x, dy, dx, oppStone, THREE_OPEN_MASK)) oppChanceThree = true;
                    if (oppChanceThree) break;
                }
            }
        }
    }
    if (comChanceFour) return false;
    else if (oppChanceFour) return true;
    else if (comChanceThree) return false;
    else if (oppChanceThree) return true;

    return false;
}

bool isOpenSequence(int board[][BOARD_SIZE], int y, int x, int dy, int dx, int stone, const vector<RowData>& masks) {
    uint8_t line = 0;
    uint8_t empty = 0;
    for (int step = -2; step <= 5; ++step) {
        int ny = y + step * dy;
        int nx = x + step * dx;

        if (ny < 0 || ny >= BOARD_SIZE || nx < 0 || nx >= BOARD_SIZE) continue;

        if (board[ny][nx] == stone) {
            line |= (1 << (step + 2));
        } else if (board[ny][nx] == STONE_SPACE) {
            empty |= (1 << (step + 2));
        }
    }

    for (const auto& mask : masks) {
        uint32_t filteredLine = line & mask.range;
        if (filteredLine != mask.stones) continue;  // 条件を満たさない場合はスキップ

        uint32_t filteredEmpty = empty & mask.range;
        if (filteredEmpty == mask.empty) {
            return true;}
    }
    return false;
}

int evaluateLineScore(int line, int empty, const vector<RowData>& masks, const int score) {
    for (const auto& mask : masks) {
        uint32_t filteredLine = line & mask.range;
        if (filteredLine != mask.stones) continue;  // 条件を満たさない場合はスキップ

        uint32_t filteredEmpty = empty & mask.range;
        if (filteredEmpty == mask.empty) return score;
    }
    return 0;
}

bool evaluateLineScore(int line, int empty, const vector<RowData>& masks) {
    for (const auto& mask : masks) {
        uint32_t filteredLine = line & mask.range;
        if (filteredLine != mask.stones) continue;  // 条件を満たさない場合はスキップ

        uint32_t filteredEmpty = empty & mask.range;
        if (filteredEmpty == mask.empty) return true;
    }
    return false;
}



int evaluate(const BitBoard& computer, const BitBoard& opponent) {
    int score = 0;
    uint8_t countComOpenFour = 0;
    uint8_t countComOpenThree = 0;

    uint8_t countOppOpenFour = 0;
    uint8_t countOppOpenThree = 0;

    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            if (computer.checkBit(y, x)) {

                for (const auto& [dy, dx] : DIRECTIONS) {
                    auto [line, empty] = computer.putOutBitLine(y, x, dy, dx, -2, 5);

                    if (evaluateLineScore(line, empty, SKIP_MASK)) continue;

                    int preScore = score;
                    // ４連両端空き
                    score += evaluateLineScore(line, empty, FOUR_OPEN_MASK,   SCORE_OPEN_FOUR);
                    if (preScore != score) {
                        countComOpenFour++;
                        if (countComOpenFour >= 2 || (countComOpenFour >= 1 && countComOpenThree >= 1)) return SCORE_NEAR_WIN;
                        else continue;
                    }
                    // 4連片側空き
                    score += evaluateLineScore(line, empty, FOUR_CLOSE_MASK,  SCORE_CLOSE_FOUR);
                    if (preScore != score) {
                        countComOpenFour++;
                        if (countComOpenFour >= 2 || (countComOpenFour >= 1 && countComOpenThree >= 1)) return SCORE_NEAR_WIN;
                        else continue;
                    }
                    // 3連両端空き
                    score += evaluateLineScore(line, empty, THREE_OPEN_MASK,  SCORE_OPEN_THREE);
                    if (preScore != score) {
                        countComOpenThree++;
                        if (countComOpenThree >= 2 || (countComOpenThree >= 1 && countComOpenFour >= 1)) return SCORE_NEAR_WIN;
                        else continue;
                    }
                    // 3連片側空き
                    score += evaluateLineScore(line, empty, THREE_CLOSE_MASK, SCORE_CLOSE_THREE);
                    if (preScore != score) continue;
                    // 2連両端空き
                    score += evaluateLineScore(line, empty, TWO_OPEN_MASK,    SCORE_OPEN_TWO);



                }
            } else if (opponent.checkBit(y, x)) {

                for (const auto& [dy, dx] : DIRECTIONS) {
                    auto [line, empty] = opponent.putOutBitLine(y, x, dy, dx, -2, 5);

                    if (evaluateLineScore(line, empty, SKIP_MASK)) continue;

                    int preScore = score;
                    // ４連両端空き
                    score -= evaluateLineScore(line, empty, FOUR_OPEN_MASK, SCORE_OPEN_FOUR);
                    if (preScore != score) {
                        countOppOpenFour++;
                        if (countOppOpenFour >= 2 || (countOppOpenFour >= 1 && countOppOpenThree >= 1)) return -SCORE_NEAR_WIN;
                        else continue;
                    }
                    // 4連片側空き
                    score -= evaluateLineScore(line, empty, FOUR_CLOSE_MASK, SCORE_CLOSE_FOUR);
                    if (preScore != score) {
                        countOppOpenFour++;
                        if (countOppOpenFour >= 2 || (countOppOpenFour >= 1 && countOppOpenThree >= 1)) return -SCORE_NEAR_WIN;
                        else continue;
                    }
                    // 3連両端空き
                    score -= evaluateLineScore(line, empty, THREE_OPEN_MASK, SCORE_OPEN_THREE);
                    if (preScore != score) {
                        countOppOpenThree++;
                        if (countOppOpenThree >= 2 || (countOppOpenThree >= 1 && countOppOpenFour >= 1)) return -SCORE_NEAR_WIN;
                        else continue;
                    }
                    // 3連片側空き
                    score -= evaluateLineScore(line, empty, THREE_CLOSE_MASK, SCORE_CLOSE_THREE);
                    if (preScore != score) continue;
                    // 2連両端空き
                    score -= evaluateLineScore(line, empty, TWO_OPEN_MASK, SCORE_OPEN_TWO);
                }
            }
        }
    }
    return score;
}