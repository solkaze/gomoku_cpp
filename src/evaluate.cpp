#include <iomanip>
#include <iostream>

#include "prohibit.hpp"
#include "evaluate.hpp"
#include "csv_data.hpp"

//*====================
//* プロトタイプ宣言
//*====================

bool fiveLow(const BitBoard& bitBoard, const int y, const int x);

GameSet isWin(const BitBoard& computer, const BitBoard& opponent, pair<int, int> put);

Advantage checkAdvantage(int board[][BOARD_SIZE], int comStone, int oppStone, int& putY, int& putX);

bool isOpenSequence(int board[][BOARD_SIZE], int y, int x, int dy, int dx, int stone, const vector<RowData>& masks);

//*====================
//* グローバル変数
//*====================

// CSVデータクラスの定義
CSVData fiveLowMASK("data/five_mask.csv");
CSVData fourOpenMask("data/four_open_mask.csv");
CSVData fourJumpMask("data/four_jump_mask.csv");
CSVData fourCloseMask("data/four_close_mask.csv");
CSVData threeOpenMask("data/three_open_mask.csv");
CSVData threeJumpMask("data/three_jump_mask.csv");
CSVData threeCloseMask("data/three_close_mask.csv");
CSVData twoOpenMask("data/two_open_mask.csv");
CSVData skipMask("data/skip_mask.csv");

const auto FIVE_LOW_MASK    = fiveLowMASK.getData();
const auto FOUR_OPEN_MASK   = fourOpenMask.getData();
const auto FOUR_JUMP_MASK   = fourJumpMask.getData();
const auto FOUR_CLOSE_MASK  = fourCloseMask.getData();
const auto THREE_OPEN_MASK  = threeOpenMask.getData();
const auto THREE_JUMP_MASK  = threeJumpMask.getData();
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
        if (computer.getStone() == STONE_BLACK && isProhibited(computer, y, x)) return GameSet::LOSE;
        if (fiveLow(computer, y, x)) return GameSet::WIN;

    } else if (opponent.checkBit(y, x)) {
        if (opponent.getStone() == STONE_BLACK && isProhibited(opponent, y, x)) return GameSet::WIN;
        if (fiveLow(opponent, y, x)) return GameSet::LOSE;
    }

    return GameSet::CONTINUE;
}

Advantage checkAdvantage(int board[][BOARD_SIZE], int comStone, int oppStone, int& putY, int& putX) {
    bool comChanceThree = false;
    bool comChanceFour  = false;
    bool oppChanceThree = false;
    bool oppChanceFour  = false;
    int comStoreX = -1, comStoreY = -1;
    int oppStoreX = -1, oppStoreY = -1;

    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            // 空きセルスキップ
            if (board[y][x] == comStone) {
                for (const auto& [dy, dx] : DIRECTIONS) {
                    if (!comChanceFour && isOpenSequence(board, y, x, dy, dx, comStone, FOUR_OPEN_MASK)) {
                        comChanceFour = true;
                        comStoreY = y + dy;
                        comStoreX = x + dx;
                        break;
                    } else if (!comChanceFour && isOpenSequence(board, y, x, dy, dx, comStone, FOUR_JUMP_MASK)) {
                        comChanceFour = true;
                        comStoreY = y + dy;
                        comStoreX = x + dx;
                        break;
                    } else if (!comChanceFour && isOpenSequence(board, y, x, dy, dx, comStone, FOUR_CLOSE_MASK)) {
                        comChanceFour = true;
                        comStoreY = y + dy;
                        comStoreX = x + dx;
                        break;
                    } else if (!comChanceFour && !comChanceThree &&
                                isOpenSequence(board, y, x, dy, dx, comStone, THREE_OPEN_MASK)) {
                        comChanceThree = true;
                        comStoreY = y + dy;
                        comStoreX = x + dx;
                        break;
                    } else if (!comChanceFour && !comChanceThree &&
                                isOpenSequence(board, y, x, dy, dx, comStone, THREE_JUMP_MASK)) {
                        comChanceThree = true;
                        comStoreY = y + dy;
                        comStoreX = x + dx;
                        break;
                    }
                }
            } else if (board[y][x] == oppStone) {
                for (const auto& [dy, dx] : DIRECTIONS) {
                    if (!oppChanceFour && isOpenSequence(board, y, x, dy, dx, oppStone, FOUR_OPEN_MASK)) {
                        oppChanceFour = true;
                        oppStoreY = y + dy;
                        oppStoreX = x + dx;
                        break;
                    } else if (!oppChanceFour && isOpenSequence(board, y, x, dy, dx, oppStone, FOUR_JUMP_MASK)) {
                        oppChanceFour = true;
                        oppStoreY = y + dy;
                        oppStoreX = x + dx;
                        break;
                    } else if (!oppChanceFour && isOpenSequence(board, y, x, dy, dx, oppStone, FOUR_CLOSE_MASK)) {
                        oppChanceFour = true;
                        oppStoreY = y + dy;
                        oppStoreX = x + dx;
                        break;
                    } else if (!oppChanceFour && !oppChanceThree &&
                                isOpenSequence(board, y, x, dy, dx, oppStone, THREE_OPEN_MASK)) {
                        oppChanceThree = true;
                        oppStoreY = y + dy;
                        oppStoreX = x + dx;
                        break;
                    } else if (!oppChanceFour && !oppChanceThree &&
                                isOpenSequence(board, y, x, dy, dx, oppStone, THREE_JUMP_MASK)) {
                        oppChanceThree = true;
                        oppStoreY = y + dy;
                        oppStoreX = x + dx;
                        break;
                    }
                }
            }
        }
    }

    if (comChanceFour) {
        putY = comStoreY;
        putX = comStoreX;
        return Advantage::COM;
    } else if (oppChanceFour) {
        putY = oppStoreY;
        putX = oppStoreX;
        return Advantage::OPP;
    } else if (comChanceThree) {
        putY = comStoreY;
        putX = comStoreX;
        return Advantage::COM;
    } else if (oppChanceThree) {
        putY = oppStoreY;
        putX = oppStoreX;
        return Advantage::OPP;
    }

    return Advantage::DRAW;
}

bool isOpenSequence(int board[][BOARD_SIZE], int y, int x, int dy, int dx, int stone, const vector<RowData>& masks) {
    uint8_t line = 0;
    uint8_t empty = 0;
    for (int step = -2; step <= 5; ++step) {
        int ny = y + step * dy;
        int nx = x + step * dx;

        if (ny < 0 || ny >= BOARD_SIZE || nx < 0 || nx >= BOARD_SIZE) continue;

        if (board[ny][nx] == stone) {
            line |= (1U << (step + 2));
        } else if (board[ny][nx] == STONE_SPACE) {
            empty |= (1U << (step + 2));
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
    uint8_t countComOpenFour  = 0;
    uint8_t countComOpenThree = 0;

    uint8_t countOppOpenFour  = 0;
    uint8_t countOppOpenThree = 0;

    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            if (computer.checkBit(y, x)) {
                for (const auto& [dy, dx] : DIRECTIONS) {
                    auto [line, empty] = computer.putOutBitLine(y, x, dy, dx, -2, 5);

                    if (evaluateLineScore(line, empty, SKIP_MASK)) continue;

                    int preScore = score;
                    // ４連両端空き
                    score += evaluateLineScore(line, empty, FOUR_OPEN_MASK, SCORE_FOUR_OPEN);
                    if (preScore != score) {
                        countComOpenFour++;
                        if (countComOpenFour >= 2 || (countComOpenFour >= 1 && countComOpenThree >= 1)) score += SCORE_NEAR_WIN;
                        else continue;
                    }
                    // 4連飛び
                    score += evaluateLineScore(line, empty, FOUR_JUMP_MASK, SCORE_FOUR_JUMP);
                    if (preScore != score) {
                        countComOpenFour++;
                        if (countComOpenFour >= 2 || (countComOpenFour >= 1 && countComOpenThree >= 1)) score += SCORE_NEAR_WIN;
                        else continue;
                    }
                    // 4連片側空き
                    score += evaluateLineScore(line, empty, FOUR_CLOSE_MASK,  SCORE_FOUR_CLOSE);
                    if (preScore != score) {
                        countComOpenFour++;
                        if (countComOpenFour >= 2 || (countComOpenFour >= 1 && countComOpenThree >= 1)) score += SCORE_NEAR_WIN;
                        else continue;
                    }
                    // 3連両端空き
                    score += evaluateLineScore(line, empty, THREE_OPEN_MASK,  SCORE_THREE_OPEN);
                    if (preScore != score) {
                        countComOpenThree++;
                        if (countComOpenThree >= 2 || (countComOpenThree >= 1 && countComOpenFour >= 1)) score += SCORE_NEAR_WIN;
                        else continue;
                    }
                    // 3連飛び
                    score += evaluateLineScore(line, empty, THREE_JUMP_MASK, SCORE_THREE_JUMP);
                    if (preScore != score) {
                        countComOpenThree++;
                        if (countComOpenThree >= 2 || (countComOpenThree >= 1 && countComOpenFour >= 1)) score += SCORE_NEAR_WIN;
                        else continue;
                    }
                    // 3連片側空き
                    score += evaluateLineScore(line, empty, THREE_CLOSE_MASK, SCORE_THREE_CLOSE);
                    if (preScore != score) continue;
                    // 2連両端空き
                    score += evaluateLineScore(line, empty, TWO_OPEN_MASK,    SCORE_TWO_OPEN);
                }
            } else if (opponent.checkBit(y, x)) {

                for (const auto& [dy, dx] : DIRECTIONS) {
                    auto [line, empty] = opponent.putOutBitLine(y, x, dy, dx, -2, 5);

                    if (evaluateLineScore(line, empty, SKIP_MASK)) continue;

                    int preScore = score;
                    // ４連両端空き
                    score -= evaluateLineScore(line, empty, FOUR_OPEN_MASK, SCORE_FOUR_OPEN);
                    if (preScore != score) {
                        countOppOpenFour++;
                        if (countOppOpenFour >= 2 || (countOppOpenFour >= 1 && countOppOpenThree >= 1)) score -= SCORE_NEAR_WIN;
                        else continue;
                    }
                    // 4連飛び
                    score -= evaluateLineScore(line, empty, FOUR_JUMP_MASK, SCORE_FOUR_JUMP);
                    if (preScore != score) {
                        countOppOpenFour++;
                        if (countOppOpenFour >= 2 || (countOppOpenFour >= 1 && countOppOpenThree >= 1)) score -= SCORE_NEAR_WIN;
                        else continue;
                    }
                    // 4連片側空き
                    score -= evaluateLineScore(line, empty, FOUR_CLOSE_MASK, SCORE_FOUR_CLOSE);
                    if (preScore != score) {
                        countOppOpenFour++;
                        if (countOppOpenFour >= 2 || (countOppOpenFour >= 1 && countOppOpenThree >= 1)) score -= SCORE_NEAR_WIN;
                        else continue;
                    }
                    // 3連両端空き
                    score -= evaluateLineScore(line, empty, THREE_OPEN_MASK, SCORE_THREE_OPEN);
                    if (preScore != score) {
                        countOppOpenThree++;
                        if (countOppOpenThree >= 2 || (countOppOpenThree >= 1 && countOppOpenFour >= 1)) score -= SCORE_NEAR_WIN;
                        else continue;
                    }
                    // 3連飛び
                    score -= evaluateLineScore(line, empty, THREE_JUMP_MASK, SCORE_THREE_JUMP);
                    if (preScore != score) {
                        countOppOpenThree++;
                        if (countOppOpenThree >= 2 || (countOppOpenThree >= 1 && countOppOpenFour >= 1)) score -= SCORE_NEAR_WIN;
                        else continue;
                    }
                    // 3連片側空き
                    score -= evaluateLineScore(line, empty, THREE_CLOSE_MASK, SCORE_THREE_CLOSE);
                    if (preScore != score) continue;
                    // 2連両端空き
                    score -= evaluateLineScore(line, empty, TWO_OPEN_MASK, SCORE_TWO_OPEN);
                }
            }
        }
    }
    return score;
}