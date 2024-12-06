#include <iostream>
#include <numeric>
#include <array>
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
#include <immintrin.h>

#include "gomoku.hpp"

using namespace std;

//*==================================================
//*    定数
//*==================================================

// int型の最大数
constexpr int INF = numeric_limits<int>::max();

// スコア
constexpr int SCORE_FIVE          = 1000000;
constexpr int SCORE_OPEN_FOUR     = 10000;
constexpr int SCORE_CLOSED_FOUR   = 5000;
constexpr int SCORE_OPEN_THREE    = 1000;
constexpr int SCORE_CLOSED_THREE  = 500;
constexpr int SCORE_OPEN_TWO      = 100;
constexpr int SCORE_CLOSED_TWO    = 50;

// 禁じ手
constexpr bool PROHIBITED_THREE_THREE  = true;
constexpr bool PROHIBITED_FOUR_FOUR    = true;
constexpr bool PROHIBITED_LONG_LONG    = false;

// 評価関数用方向
constexpr array<array<int, 2>, 4> DIRECTIONS = {{
    {0,  1},
    {1,  0},
    {1,  1},
    {1, -1}
}};

// アルファ・ベータ法最大深度
constexpr int MAX_DEPTH = 4;

// スレッドの最大数
constexpr int MAX_THREADS = 8;

// コンパイル時初期化のための定数 15マス
constexpr int K_BOARD_SIZE = BOARD_SIZE;

// すべてのマスの数 225マス
constexpr int TOTAL_CELLS = K_BOARD_SIZE * K_BOARD_SIZE;

// 必要なuint64_tの数
constexpr int BITBOARD_PARTS = (TOTAL_CELLS + 63) / 64;

// ビット列の最大サイズ
constexpr int SEGMENT_SIZE = 64;

// コンパイル時に自動生成
constexpr array<pair<int, int>, TOTAL_CELLS> generateSpiralMoves() {
    array<pair<int, int>, TOTAL_CELLS> moves{};
    int cy = K_BOARD_SIZE / 2, cx = K_BOARD_SIZE / 2;
    moves[0] = {cy, cx};

    const int directions[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
    int steps = 1;
    int index = 1;

    while (index < TOTAL_CELLS) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < steps && index < TOTAL_CELLS; ++j) {
                cy += directions[i][0];
                cx += directions[i][1];
                if (cy >= 0 && cy < K_BOARD_SIZE && cx >= 0 && cx < K_BOARD_SIZE) {
                    moves[index++] = {cy, cx};
                }
            }
            if (i % 2 == 1) ++steps;
        }
    }
    return moves;
}

// グローバル変数として初期化
constexpr auto SPIRAL_MOVES = generateSpiralMoves();



//* ビット列をbitBoardとして表す
using BitBoard = array<uint64_t, BITBOARD_PARTS>;


//*==================================================
//*    構造体, クラス, 列挙
//*==================================================

enum class GameSet {
    WIN,
    LOSE,
    PROHIBITED,
    CONTINUE
};


//*==================================================
//*    グローバル変数
//*==================================================

// CPUの石
int ComStone;

// 対戦相手の石
int OppStone;

// 共有ロック
shared_mutex TableMutex;

// 履歴スタック
thread_local stack<pair<pair<int, int>, int>> History;

// ビットボード
BitBoard ComputerBitboard{};
BitBoard OpponentBitboard{};


//*==================================================
//*    プロトタイプ関数宣言
//*==================================================


// 範囲外判定
bool isOutOfRange(int x, int y);
// 満杯判定
bool isBoardFull(const BitBoard& computerBitboard, const BitBoard& opponentBitboard);
// 勝利判定
GameSet isWin(BitBoard& computerBitboard,
                BitBoard& opponentBitboard,
                    stack<pair<pair<int, int>, int>>& History);
// 特定の方向で勝利判定（前述の関数を利用）
bool isWinDirection(int y, int x, const BitBoard& bitboard);


// 33禁判定
bool isProhibitedThreeThree(const BitBoard& computer, const BitBoard& opponent, int y, int x, int stone);
// 44禁判定
bool isProhibitedFourFour(const BitBoard& computer, const BitBoard& opponent, int y, int x, int stone);
// 長連禁判定
bool isProhibitedLongLens(const BitBoard& bitboard, int y, int x);


// 指定位置を1にする
inline void setBit(BitBoard& bitboard, int y, int x);
// 指定位置を0にする
inline void clearBit(BitBoard& bitboard, int y, int x);
// 指定位置のビットが1か0か
inline bool checkBit(const BitBoard& bitboard, int y, int x);
// 配列をビット列に変換
void convertToBitboards(int board[][BOARD_SIZE]);


// AND 演算
BitBoard operator&(const BitBoard& lhs, const BitBoard& rhs);
// OR 演算
BitBoard operator|(const BitBoard& lhs, const BitBoard& rhs);
// XOR 演算
BitBoard operator^(const BitBoard& lhs, const BitBoard& rhs);
// NOT 演算
BitBoard operator~(const BitBoard& bitboard);


// ボード全体の評価
int evaluateBoard(const BitBoard& comBitboard, const BitBoard& oppBitboard);
// アルファ・ベータ法
int alphaBeta(BitBoard& computer, BitBoard& opponent,
                int depth, int alpha, int beta, bool isMaximizingPlayer);
// 最適解探索
pair<int, int> findBestMove(int pos_x, int pos_y);
// コマ配置
int calcPutPos(int board[][BOARD_SIZE], int com, int *pos_x, int *pos_y);


//*==================================================
//*    関数実装
//*==================================================


//* 判定関数

// 範囲外判定
bool isOutOfRange(int x, int y) {
    return x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE;
}

// 満杯判定
bool isBoardFull(const BitBoard& computerBitboard, const BitBoard& opponentBitboard) {
    BitBoard combined = computerBitboard | opponentBitboard;

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
bool isProhibitedThreeThree(const BitBoard& computer, const BitBoard& opponent, int y, int x, int stone) {
    int threeCount = 0;
    const BitBoard bitBoard = stone == ComStone ? computer : opponent;
    const BitBoard empty = ~(computer | opponent);

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

        if (count == 3 && openCount != 0) ++threeCount;
    }

    if (threeCount >= 2) return true;

    return false;
}

// 44禁判定
bool isProhibitedFourFour(const BitBoard& computer, const BitBoard& opponent, int y, int x, int stone) {
    int fourCount = 0;
    const BitBoard bitBoard = stone == ComStone ? computer : opponent;
    const BitBoard empty = ~(computer | opponent);

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
bool isProhibitedLongLens(const BitBoard& bitBoard, int y, int x) {
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
bool isWinDirection(int y, int x, const BitBoard& bitboard) {

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
GameSet isWin(BitBoard& computer,
                BitBoard& opponent,
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

//* BitBoard操作

// 指定位置を1にする
inline void setBit(BitBoard& bitboard, int y, int x) {
    int pos = y * BOARD_SIZE + x;
    bitboard[pos / 64] |= (1ULL << (pos % 64));
}

// 指定位置を0にする
inline void clearBit(BitBoard& bitboard, int y, int x) {
    int pos = y * BOARD_SIZE + x;
    bitboard[pos / 64] &= ~(1ULL << (pos % 64));
}

// 指定位置のビットが1か0か
inline bool checkBit(const BitBoard& bitboard, int y, int x) {
    int pos = y * BOARD_SIZE + x;
    return bitboard[pos / 64] & (1ULL << (pos % 64));
}

// AND 演算
BitBoard operator&(const BitBoard& lhs, const BitBoard& rhs) {
    BitBoard result{};
    for (size_t i = 0; i < lhs.size(); ++i) {
        result[i] = lhs[i] & rhs[i];
    }
    return result;
}

// OR 演算
BitBoard operator|(const BitBoard& lhs, const BitBoard& rhs) {
    BitBoard result{};
    for (size_t i = 0; i < lhs.size(); ++i) {
        result[i] = lhs[i] | rhs[i];
    }
    return result;
}

// XOR 演算
BitBoard operator^(const BitBoard& lhs, const BitBoard& rhs) {
    BitBoard result{};
    for (size_t i = 0; i < lhs.size(); ++i) {
        result[i] = lhs[i] ^ rhs[i];
    }
    return result;
}

// NOT 演算
BitBoard operator~(const BitBoard& lhs) {
    BitBoard result{};
    for (size_t i = 0; i < lhs.size(); ++i) {
        result[i] = ~lhs[i];
    }
    return result;
}

// 配列をビット列に変換
void convertToBitboards(int board[][BOARD_SIZE]) {
    // 初期化
    ComputerBitboard.fill(0);
    OpponentBitboard.fill(0);

    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            int pos = y * BOARD_SIZE + x; // 総ビット位置
            int part = pos / 64;          // どのuint64_tに対応するか
            int offset = pos % 64;        // そのuint64_t内のビット位置

            if (board[y][x] == ComStone) { // 黒石
                ComputerBitboard[part] |= (1ULL << offset);
            } else if (board[y][x] == OppStone) { // 白石
                OpponentBitboard[part] |= (1ULL << offset);
            }
        }
    }
}

// 4連成立確認
bool isFourInARow(const BitBoard& computer, const BitBoard& opponent, int& y, int& x) {
    const BitBoard empty = ~(computer | opponent);

    for (const auto& [dy, dx] : DIRECTIONS) {
        // 正方向
        int count = 1;
        bool isSpace = false;
        int sy = y, sx = x;

        for (int step = 1; step < 5; ++step) {
            int ny = y + step * dy;
            int nx = x + step * dx;

            if (isOutOfRange(nx, ny)) break;
            if (checkBit(empty, ny, nx)) {
                isSpace = true;
                sy = ny;
                sx = nx;
            }
            if (!checkBit(opponent, ny, nx)) break;
            ++count;
        }

        // 逆方向
        for (int step = 1; step < 5; ++step) {
            int ny = y - step * dy;
            int nx = x - step * dx;

            if (isOutOfRange(nx, ny)) break;
            if (checkBit(empty, ny, nx)) {
                isSpace = true;
                sy = ny;
                sx = nx;
            }
            if (!checkBit(opponent, ny, nx)) break;
            ++count;
        }

        if (count >= 4 && isSpace) {
            y = sy;
            x = sx;
            return true;
        }
    }

    return false;
}

// 評価関数
int evaluateBoard(const BitBoard& computer, const BitBoard& opponent) {
    BitBoard empty_board = ~(computer | opponent);
    int score = 0;

    return score;
}

// アルファ・ベータ法
int alphaBeta(BitBoard& computer, BitBoard& opponent,
                int depth, int alpha, int beta, bool isMaximizingPlayer) {

    // 勝利判定の確認
    switch(isWin(computer, opponent, History)) {
        case GameSet::WIN:
            if (depth == 0) return INF;
            return SCORE_FIVE;
        case GameSet::PROHIBITED:
            if (depth == 0) return ComStone == STONE_BLACK ? -INF + 1 : INF;
            return ComStone == STONE_BLACK ? -SCORE_FIVE : SCORE_FIVE;
        case GameSet::LOSE:
            if (depth == 0) return -INF + 1;
            return -SCORE_FIVE;
        case GameSet::CONTINUE:
            break;
    }

    // 探索の末端のとき
    if (depth == MAX_DEPTH) {
        return evaluateBoard(computer, opponent);
    }

    // アルファ・ベータ法の本編
    if (isMaximizingPlayer) {
        int maxEval = -INF;

        for (const auto& [y, x] : SPIRAL_MOVES) {
            if (!checkBit(computer, y, x) && !checkBit(opponent, y, x)) {

                setBit(computer, y, x);
                History.push({{y, x}, ComStone});

                int eval = alphaBeta(computer, opponent, depth + 1, alpha, beta, false);

                clearBit(computer, y, x);
                History.pop();

                maxEval = max(maxEval, eval);
                alpha = max(alpha, eval);

                if (beta <= alpha) break; // Beta cut-off
            }
        }
        return maxEval;
    } else {
        int minEval = INF;

        for (const auto& [y, x] : SPIRAL_MOVES) {
            if (!checkBit(computer, y, x) && !checkBit(opponent, y, x)) {

                setBit(opponent, y, x);
                History.push({{y, x}, OppStone});

                int eval = alphaBeta(computer, opponent, depth + 1, alpha, beta, true);

                clearBit(opponent, y, x);
                History.pop();

                minEval = min(minEval, eval);
                beta = min(beta, eval);

                if (beta <= alpha) break; // Alpha cut-off
            }
        }
        return minEval;
    }
}

// 最適解探索
pair<int, int> findBestMove(int pos_y, int pos_x) {
    int bestVal = -INF;
    pair<int, int> bestMove = {-1, -1};
    vector<future<pair<int, pair<int, int>>>> futures;
    mutex mtx; // 排他制御用
    atomic<int> threadCount(0); // 実行スレッド数

    if (isFourInARow(ComputerBitboard, OpponentBitboard, pos_y, pos_x)) {
        return make_pair(pos_y, pos_x);
    }

    // 各手を分割して並行処理
    for (const auto& [dy, dx] : SPIRAL_MOVES) {

        if (!checkBit(ComputerBitboard, dy, dx) && !checkBit(OpponentBitboard, dy, dx)) { // 空白確認
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
                BitBoard localCom = ComputerBitboard;
                BitBoard localOpp = OpponentBitboard;

                // ビットボードに現在の手を設定
                setBit(localCom, dy, dx);
                History.push({{dy, dx}, ComStone});

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

// 最適解探索スレッドなし
pair<int, int> findBestMoveSample() {
    int bestVal = -INF;
    pair<int, int> bestMove = {-1, -1};

    for (const auto& [y, x] : SPIRAL_MOVES) {
        if (!checkBit(ComputerBitboard, y, x) && !checkBit(OpponentBitboard, y, x)) {
            BitBoard localCom = ComputerBitboard;
            BitBoard localOpp = OpponentBitboard;

            setBit(localCom, y, x);
            History.push({{y, x}, ComStone});

            int moveVal = alphaBeta(localCom, localOpp, 0, bestVal, INF, false);

            clearBit(localCom, y, x);
            History.pop();
            cout << moveVal << endl;

            if (moveVal > bestVal) {
                bestVal = moveVal;
                bestMove = make_pair(y, x);
            }
        }
    }

    return bestMove;
}
// コマ配置
int calcPutPos(int board[][BOARD_SIZE], int com, int *pos_x, int *pos_y) {
    static bool isFirst = true;

    // 序盤処理の設定
    if (isFirst) {
        isFirst = false;
        ComStone = com;
        OppStone = ComStone == STONE_BLACK ? STONE_WHITE : STONE_BLACK;

        if (ComStone == STONE_BLACK) {
            *pos_y = BOARD_SIZE / 2;
            *pos_x = BOARD_SIZE / 2;
            return 0;
        }
    }

    convertToBitboards(board);

    // 配置処理
    pair<int, int> bestMove = findBestMove(*pos_y, *pos_x);
    *pos_y = bestMove.first;
    *pos_x = bestMove.second;

    cout << "置いた位置:( " << *pos_x << ", " << *pos_y << " )" << endl;
    return 0;
}