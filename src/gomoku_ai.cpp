#include <numeric>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <limits>
#include <random>
#include <chrono>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <stack>
#include <atomic>
#include <future>

#include "common.hpp"
#include "prohibited.hpp"
#include "evaluate.hpp"
#include "alpha_beta.hpp"

using namespace std;

//*==================================================
//*    定数
//*==================================================
//* ビット列をbitBoardとして表す
using BitLine = array<uint64_t, BITBOARD_PARTS>;


//*==================================================
//*    構造体, クラス, 列挙
//*==================================================


//*==================================================
//*    グローバル変数
//*==================================================

// CPUの石
int ComStone;

// 対戦相手の石
int OppStone;

// ビットボード
BitLine ComputerBitboard = {0, 0, 0, 0};
BitLine OpponentBitboard = {0, 0, 0, 0};


//*==================================================
//*    プロトタイプ関数宣言
//*==================================================


//*==================================================
//*    関数実装
//*==================================================


//* 判定関数

// 範囲外判定
bool isOutOfRange(int x, int y) {
    return x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE;
}

// 満杯判定
bool isBoardFull(const BitLine& computerBitboard, const BitLine& opponentBitboard) {
    BitLine combined = computerBitboard | opponentBitboard;

    // 全ビットが埋まっている（= 全てのビットが 1）場合
    for (int i = 0; i < 3; i++) {
        if (combined[i] != ~0ULL) {  // 64ビットが全て1ではない
            return false;
        }
    }

    // 最後のビットボードの残り部分を確認（225ビット目以降は無視）
    int remainingBits = 225 - 64 * 3;
    uint64_t mask = (1ULL << remainingBits) - 1;  // 下位 "remainingBits" ビットのみを 1 にするマスク
    if ((combined[3] & mask) != mask) {
        return false;
    }

    return true;  // 全てのビットが埋まっている
}

//!----------
//! 禁じ手処理
//!----------

// 33禁判定
bool isProhibitedThreeThree(const BitLine& computer, const BitLine& opponent, int y, int x, int stone) {
    int threeCount = 0;
    const BitLine bitBoard = stone == ComStone ? computer : opponent;
    const BitLine empty = ~(computer | opponent);

    for (const auto& [dy, dx] : DIRECTIONS) {
        int count = 1;
        bool frontEndOpen = false;
        bool backEndOpen  = false;

        // 正方向
        for (int step = 1; step < 5; ++step) {
            int ny = y + step * dy;
            int nx = x + step * dx;
            int openCount = 0;

            if (isOutOfRange(nx, ny)) break;

            // 石の判定
            if (checkBit(empty, ny, nx)) { // 空白
                // 最後尾か否か
                if ((openCount == 0 && step == 3) || (openCount == 1 && step == 4) || checkBit(empty, ny - dy, nx - dx)) {
                    frontEndOpen = true;
                    break;
                } else if (openCount == 0) {
                    ++openCount;
                }
            } else if (checkBit(bitBoard, ny, nx)) ++count; // 自分の石
            else break; // 相手の石の場合は切り捨て
        }

        // 逆方向
        for (int step = 1; step < 5; ++step) {
            int ny = y - step * dy;
            int nx = x - step * dx;
            int openCount = 0;

            if (isOutOfRange(nx, ny)) break;

            if (checkBit(empty, ny, nx)) { // 空白
                if (openCount == 0) { // 飛び石の考慮
                    ++openCount;
                } else if (step == 3 || checkBit(empty, ny - dy, nx - dx)) {
                    backEndOpen = true;
                }
            } else if (checkBit(bitBoard, ny, nx)) ++count; // 自分の石
        }

        if (count == 3 && frontEndOpen && backEndOpen) ++threeCount;
    }

    if (threeCount >= 2) return true;

    return false;
}

// 44禁判定
bool isProhibitedFourFour(const BitLine& computer, const BitLine& opponent, int y, int x, int stone) {
    int fourCount = 0;
    const BitLine bitBoard = stone == ComStone ? computer : opponent;
    const BitLine empty = ~(computer | opponent);

    for (const auto& [dy, dx] : DIRECTIONS) {
        int count = 1;
        int openCount = 0;

        // 正方向
        for (int step = 1; step < 5; ++step) {
            int ny = y + step * dy;
            int nx = x + step * dx;

            if (isOutOfRange(nx, ny)) break;

            if (checkBit(empty, ny, nx)) { // 空白
                if (openCount == 0) { // 飛び石の考慮
                    ++openCount;
                    continue;
                } else {
                    break;
                }
            } else if (!checkBit(bitBoard, ny, nx)) break; // 相手の石

            ++count;
        }

        // 逆方向
        for (int step = 1; step < 5; ++step) {
            int ny = y - step * dy;
            int nx = x - step * dx;

            if (isOutOfRange(nx, ny)) break;

            if (checkBit(empty, ny, nx)) { // 空白
                if (openCount == 0) { // 飛び石の考慮
                    ++openCount;
                    continue;
                } else {
                    break;
                }
            } else if (!checkBit(bitBoard, ny, nx)) break; // 相手の石

            ++count;
        }

        if (count == 4 && openCount != 0) ++fourCount;
    }

    if (fourCount >= 2) return true;

    return false;
}

// 長連禁判定
bool isProhibitedLongLens(const BitLine& bitBoard, int y, int x) {
    for (const auto& [dy, dx] : DIRECTIONS) {
        int longCount = 1;
        // 正方向
        for (int step = 1; step < 5; ++step) {
            int ny = y + step * dy;
            int nx = x + step * dx;

            if (isOutOfRange(nx, ny)) break;
            if (!checkBit(bitBoard, ny, nx)) break;
            ++longCount;
        }

        // 逆方向
        for (int step = 1; step < 5; ++step) {
            int ny = y - step * dy;
            int nx = x - step * dx;

            if (isOutOfRange(nx, ny)) break;
            if (!checkBit(bitBoard, ny, nx)) break;
            ++longCount;
        }

        if (longCount >= 6) return true;
    }

    return false;
}

//* 勝利判定

// 特定の方向で勝利判定
bool isWinDirection(int y, int x, const BitLine& bitboard) {

    for (const auto& [dy, dx] : DIRECTIONS) {
        int count = 1;

        // 正方向
        for (int step = 1; step < 5; ++step) {
            int ny = y + step * dy;
            int nx = x + step * dx;

            if (isOutOfRange(nx, ny)) break;
            if (!checkBit(bitboard, ny, nx)) break;
            ++count;
        }

        // 逆方向
        for (int step = 1; step < 5; ++step) {
            int ny = y - step * dy;
            int nx = x - step * dx;

            if (isOutOfRange(nx, ny)) break;
            if (!checkBit(bitboard, ny, nx)) break;
            ++count;
        }

        // 5つ以上連続
        if (count >= 5) return true;
    }

    return false;
}

// 勝利判定
GameSet isWin(BitLine& computer,
                BitLine& opponent,
                    stack<pair<pair<int, int>, int>>& History) {
    // スタックが空の場合は処理をスキップ
    if (History.empty()) return GameSet::CONTINUE;

    // スタックから最後に置かれた手を取り出し
    auto [lastMove, stoneType] = History.top();
    int y = lastMove.first;
    int x = lastMove.second;

    // 禁じ手チェック（先手のみ有効）
    if (stoneType == STONE_BLACK) {
        if (PROHIBITED_THREE_THREE && isProhibitedThreeThree(computer, opponent, y, x, stoneType)) {
            return GameSet::PROHIBITED;
        }
        if (PROHIBITED_FOUR_FOUR && isProhibitedFourFour(computer, opponent, y, x, stoneType)) {
            return GameSet::PROHIBITED;
        }
        if (PROHIBITED_LONG_LONG && isProhibitedLongLens(stoneType == ComStone ? computer : opponent, y, x)) {
            return GameSet::PROHIBITED;
        }
    }

    // 勝利判定を行う
    if (stoneType == ComStone) {
        if (isWinDirection(y, x, computer)) {
            return GameSet::WIN;
        }
    } else if (stoneType == OppStone) {
        if (isWinDirection(y, x, opponent)) {
            return GameSet::LOSE;
        }
    }

    return GameSet::CONTINUE;
}

std::pair<int, int> iterativeDeepening(BitBoard& computer, BitBoard& opponent,
                                        int maxDepth, int timeLimit) {
    std::pair<int, int> bestMove = {-1, -1};
    int bestEval = -INF;

    auto startTime = std::chrono::steady_clock::now();

    // 浅い深さから順に探索
    for (int depth = 1; depth <= maxDepth; ++depth) {
        int alpha = -INF, beta = INF;

        // 現在の深さでアルファ・ベータ法を実行
        std::pair<int, int> currentBestMove = {-1, -1};
        int eval = alphaBeta(computer, opponent, depth, alpha, beta, true, currentBestMove);

        // 時間制限の確認
        auto currentTime = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() > timeLimit) {
            break; // 時間切れなら探索を終了
        }

        // 現在の深さで得た最善手と評価値を保存
        bestEval = eval;
        bestMove = currentBestMove;
    }

    return bestMove;
}


// 最適解探索
pair<int, int> findBestMove(BitBoard& computer, BitBoard& opponent, int pos_y, int pos_x) {
    int bestVal = -INF;
    pair<int, int> bestMove = {-1, -1};
    vector<future<pair<int, pair<int, int>>>> futures;
    mutex mtx; // 排他制御用
    atomic<int> threadCount(0); // 実行スレッド数

    if (isOppFour(ComputerBitboard, OpponentBitboard, pos_y, pos_x) && !isComFour(ComputerBitboard, OpponentBitboard)) {
        return make_pair(pos_y, pos_x);
    }

    for (int depth = 1; depth <= MAX_DEPTH; ++depth) {
        int alpha = -INF, beta = INF;

        // 現在の深さでアルファ・ベータ法を実行
        std::pair<int, int> currentBestMove = {-1, -1};
        int eval = alphaBeta(computer, opponent, depth, alpha, beta, true, currentBestMove);

        // 現在の深さで得た最善手と評価値を保存
        bestVal = eval;
        bestMove = currentBestMove;
    }

    // 各手を分割して並行処理
    for (const auto& [dy, dx] : SPIRAL_MOVES) {

        if (!computer.checkBit(dy, dx) && !opponent.checkBit(dy, dx)) { // 空白確認
            // スレッドの上限を維持
            if (futures.size() >= MAX_THREADS) {
                for (auto& fut : futures) {
                    auto [moveVal, pos] = fut.get(); // 結果を取得
                    cout << pos.first << ", " << pos.second << ": " << moveVal << endl;
                    lock_guard<mutex> lock(mtx); // 排他制御
                    if (moveVal > bestVal) {
                        bestVal = moveVal;
                        bestMove = pos;
                    }
                }
                futures.clear();
            }

            // 新しいスレッドで評価を非同期実行
            futures.emplace_back(async(launch::async, [=, &bestVal, &threadCount]() {
                BitBoard localCom = computer;
                BitBoard localOpp = opponent;

                // ビットボードに現在の手を設定
                localCom.setBit(dy, dx);
                History.push({localCom.getStone(), {dy, dx}});

                // アルファ・ベータ探索を実行
                int moveVal = alphaBeta(localCom, localOpp, 0, bestVal, INF, false);

                History.pop();

                // スレッド数をカウント
                threadCount++;
                return make_pair(moveVal, make_pair(dy, dx));
            }));
        }

    }

    // 残りのスレッド結果を反映
    for (auto& fut : futures) {
        auto [moveVal, pos] = fut.get();
        lock_guard<mutex> lock(mtx);
        if (moveVal > bestVal) {
            bestVal = moveVal;
            bestMove = pos;
        }
    }

    // スレッド数を出力
    cout << "実行されたスレッド数: " << threadCount.load() << endl;
    cout << bestVal << endl;
    return bestMove;
}

int calcPutPos(int board[][BOARD_SIZE], int com, int *pos_x, int *pos_y) {
    static BitBoard computerBitboard(com);
    static BitBoard opponentBitboard(com == STONE_BLACK ? STONE_WHITE : STONE_BLACK);

    // 序盤処理の設定
    if (com == STONE_BLACK && *pos_x == -1 && *pos_y == -1) {
        // 初手として中央を指定
        *pos_y = BOARD_SIZE / 2;
        *pos_x = BOARD_SIZE / 2;
        return 0;
    }

    // ビットボード変換
    computerBitboard.convertToBitboards(board);
    opponentBitboard.convertToBitboards(board);

    // 配置処理
    pair<int, int> bestMove = findBestMove(computerBitboard, opponentBitboard, *pos_y, *pos_x);

    *pos_y = bestMove.first;
    *pos_x = bestMove.second;

    cout << "置いた位置:( " << *pos_x << ", " << *pos_y << " )" << endl;
    return 0;
}
