#include "testClass.hpp"
#include "testCsv.hpp"
#include "testEvaluate.hpp"
#include <iostream>
#include <bitset>
#include <iomanip>
using namespace std;
CSVData fiveLowMASK("../data/five_mask.csv");
CSVData fourOpenMask("../data/four_open_mask.csv");
CSVData fourCloseMask("../data/four_close_mask.csv");
CSVData threeOpenMask("../data/three_open_mask.csv");
CSVData threeCloseMask("../data/three_close_mask.csv");
CSVData twoOpenMask("../data/two_open_mask.csv");
CSVData skipMask("../data/skip_mask.csv");

const auto FIVE_LOW_MASK    = fiveLowMASK.getData();
const auto FOUR_OPEN_MASK   = fourOpenMask.getData();
const auto FOUR_CLOSE_MASK  = fourCloseMask.getData();
const auto THREE_OPEN_MASK  = threeOpenMask.getData();
const auto THREE_CLOSE_MASK = threeCloseMask.getData();
const auto TWO_OPEN_MASK    = twoOpenMask.getData();
const auto SKIP_MASK        = skipMask.getData();

void testPrintBoard(const BitBoard& com,const BitBoard& opp, int y, int x) {
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
            if (i == y && j == x) {
                cout << "\033[33m";
            }
            if (com.checkBit(i, j)) {
                cout << "● ";
            } else if (opp.checkBit(i, j)) {
                cout << "○ ";
            } else {
                cout << "・";
            }
            cout << "\033[0m";

        }
        cout << endl;
    }
    cout << endl;
}

int evaluateLineScore(int line, int empty, const vector<RowData>& masks, const int score) {
    int totalScore = 0;
    for (const auto& mask : masks) {
        uint32_t filteredLine = line & mask.range;
        if (filteredLine != mask.stones) continue;  // 条件を満たさない場合はスキップ

        uint32_t filteredEmpty = empty & mask.range;
        if (filteredEmpty == mask.empty) {
            totalScore += score;
        }
    }
    if (totalScore > 0) {
        cout << "石の列:\t" << bitset<6>(line) << endl;
        cout << "空白列:\t" << bitset<6>(empty) << endl;
        cout << "スコア:\t" << totalScore << endl;
    }
    return totalScore;
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

    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            if (computer.checkBit(y, x)) {
                cout << "==========\n";
                cout << "石の色\t" << (computer.getStone() == 1 ? "黒" : "白") << endl;
                cout << "y: " << y << " x: " << x << endl;
                testPrintBoard(computer, opponent, y, x);

                for (const auto& [dy, dx] : DIRECTIONS) {
                    auto [line, empty] = computer.putOutBitLine(y, x, dy, dx, -2, 5);
                    int preScore = score;
                    cout << "-----\n";
                    cout << "向き: ";
                    if (dy == 0 && dx == 1) cout << "右" << endl;
                    else if (dy == 1 && dx == 0) cout << "下" << endl;
                    else if (dy == 1 && dx == 1) cout << "右下" << endl;
                    else if (dy == 1 && dx == -1) cout << "左下" << endl;
                    cout << endl;
                    if (evaluateLineScore(line, empty, SKIP_MASK)) continue;
                    // 2連両端空き
                    score += evaluateLineScore(line, empty, TWO_OPEN_MASK, SCORE_OPEN_TWO);
                    if (preScore != score) continue;
                    // 3連片側空き
                    score += evaluateLineScore(line, empty, THREE_CLOSE_MASK, SCORE_CLOSE_THREE);
                    if (preScore != score) continue;
                    // 3連両端空き
                    score += evaluateLineScore(line, empty, THREE_OPEN_MASK, SCORE_OPEN_THREE);
                    if (preScore != score) continue;
                    // 4連片側空き
                    score += evaluateLineScore(line, empty, FOUR_CLOSE_MASK, SCORE_CLOSE_FOUR);
                    if (preScore != score) continue;
                    // 4連両端空き
                    score += evaluateLineScore(line, empty, FOUR_OPEN_MASK, SCORE_OPEN_FOUR);
                }
                cout << "\n全体スコア:\t" << score << endl;
                cout << "==========\n";
            } else if (opponent.checkBit(y, x)) {
                cout << "==========\n";
                cout << "石の色: " << (opponent.getStone() == 1 ? "黒" : "白") << endl;
                cout << "y: " << y << " x: " << x << endl;
                testPrintBoard(computer, opponent, y, x);

                for (const auto& [dy, dx] : DIRECTIONS) {
                    auto [line, empty] = opponent.putOutBitLine(y, x, dy, dx, -2, 5);
                    int preScore = score;
                    cout << "-----\n";
                    cout << "向き: ";
                    if (dy == 0 && dx == 1) cout << "右" << endl;
                    else if (dy == 1 && dx == 0) cout << "下" << endl;
                    else if (dy == 1 && dx == 1) cout << "右下" << endl;
                    else if (dy == 1 && dx == -1) cout << "左下" << endl;
                    cout << endl;
                    if (evaluateLineScore(line, empty, SKIP_MASK)) continue;
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
                    // 4連両端空き
                    score -= evaluateLineScore(line, empty, FOUR_OPEN_MASK, SCORE_OPEN_FOUR);
                }
                cout << "\n全体スコア:\t" << score << endl;
                cout << "==========\n";
            }
        }
    }
    return score;
}

bool isOpenSequence(int board[][BOARD_SIZE], int y, int x, int dy, int dx, int stone, const vector<RowData>& masks);

bool checkChance(int board[][BOARD_SIZE], int comStone, int oppStone) {
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
    cout << "comChanceFour: " << comChanceFour << endl;
    cout << "oppChanceFour: " << oppChanceFour << endl;
    cout << "comChanceThree: " << comChanceThree << endl;
    cout << "oppChanceThree: " << oppChanceThree << endl;
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
        } else if (board[ny][nx] == 0) {
            empty |= (1 << (step + 2));
        }
    }
    cout << "y: " << y << " x: " << x << endl;
    cout << "line:  " << bitset<6>(line) << endl;
    cout << "empty: " << bitset<6>(empty) << endl;

    for (const auto& mask : masks) {
        uint32_t filteredLine = line & mask.range;
        if (filteredLine != mask.stones) continue;  // 条件を満たさない場合はスキップ

        uint32_t filteredEmpty = empty & mask.range;
        if (filteredEmpty == mask.empty) return true;
    }
    return false;
}