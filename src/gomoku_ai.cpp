#include <bits/stdc++.h>

#include "gomoku.hpp"

using namespace std;

//* ==================================================
//*     定数
//* ==================================================

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

// 評価関数用方向
constexpr array<array<int, 2>, 4> DIRECTIONS = {{
    {0, 1},
    {1, 0},
    {1, 1},
    {1, -1}
}};

// アルファ・ベータ法最大深度
constexpr int MAX_DEPTH = 3;

// スレッドの最大数
constexpr int MAX_THREADS = 8;

// コンパイル時初期化のための定数 15マス
constexpr int K_BOARD_SIZE = BOARD_SIZE;

// すべてのマスの数 225マス
constexpr int TOTAL_CELLS = K_BOARD_SIZE * K_BOARD_SIZE;

// 必要なuint64_tの数
constexpr int BITBOARD_PARTS = (TOTAL_CELLS + 63) / 64;

// コンパイル時に自動生成
constexpr array<pair<int, int>, TOTAL_CELLS> generateSpiralMoves() {
    array<pair<int, int>, TOTAL_CELLS> moves{};
    int cx = K_BOARD_SIZE / 2, cy = K_BOARD_SIZE / 2;
    moves[0] = {cx, cy};

    const int directions[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
    int steps = 1;
    int index = 1;

    while (index < TOTAL_CELLS) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < steps && index < TOTAL_CELLS; ++j) {
                cx += directions[i][0];
                cy += directions[i][1];
                if (cx >= 0 && cx < K_BOARD_SIZE && cy >= 0 && cy < K_BOARD_SIZE) {
                    moves[index++] = {cx, cy};
                }
            }
            if (i % 2 == 1) ++steps;
        }
    }
    return moves;
}

// グローバル変数として初期化
constexpr auto SPIRAL_MOVES = generateSpiralMoves();


//* ==================================================
//*     構造体, クラス, 列挙
//* ==================================================

enum class BoundType {
    EXACT,
    UPPER_BOUND,
    LOWER_BOUND
};

enum GameSet {
    CONTINUE,
    WIN,
    LOSE,
};

struct TranspositionEntry {
    int value;              // 評価値
    int depth;              // 探索した深さ
    BoundType boundType;    // 何の条件で保存したか (Exact, Upper Bound, Lower Bound)
};


//* ==================================================
//*     グローバル変数
//* ==================================================

// コンピュータの石
int ComStone;

// プレイヤーの石
int OpponentStone;

// 禁じ手
bool gProhibitedThreeThree = false;
bool gProhibitedFourFour = false;
bool gProhibitedLongLen = false;

// zobristハッシュ関数の定義
array<array<uint64_t, 3>, TOTAL_CELLS> ZobristTable;

// トランスポジションテーブルの定義
unordered_map<uint64_t, TranspositionEntry> TranspositionTable;

// 共有ロック
shared_mutex TableMutex;

// ミューテックスの定義
shared_mutex TransTableMutex;

// 履歴スタック
thread_local deque<pair<pair<int, int>, int>> History;

// 全体スコア
thread_local int BoardScore = 0;

// スレッド別ハッシュキー
thread_local uint64_t CurrentHashKey = 0;

// ビットボード（黒と白用）
array<uint64_t, 4> ComputerBitboard{};
array<uint64_t, 4> OpponentBitboard{};


//* ==================================================
//*     プロトタイプ関数宣言
//* ==================================================

// Zobristハッシュの初期化
void initZobristTable();

// ハッシュキーの更新
void updateHashKey(int y, int x, int stone);

// テーブル参照関数
bool probeTranspositionTable(uint64_t hashKey, int depth, int alpha, int beta, int& outValue);

// テーブル保存関数
void storeTransposition(uint64_t hashKey, int depth, int value, BoundType boundType);

// ボード評価関数
int evaluateBoard(int board[][BOARD_SIZE]);

// 周辺の評価
int evaluateStoneRange(int board[][BOARD_SIZE], const int row, const int col);

// 一列の評価関数
int evaluateLine(const vector<pair<int, int>>& line);

// アルファ・ベータ法
int alphaBeta(array<uint64_t, 4>& computer, array<uint64_t, 4>& opponent, int depth, int alpha, int beta, bool isMaximizingPlayer);

// 最適解探索
pair<int, int> findBestMove();

// gomoku.cppに配置を返す
int calcPutPos(int board[][BOARD_SIZE], int com, int *pos_x, int *pos_y);

// 満杯確認
bool isBoardFull(const array<uint64_t, 4>& computerBitboard, const array<uint64_t, 4>& opponentBitboard);

// 勝利確認
GameSet isWin(int board[][BOARD_SIZE], int stone);

// 範囲外確認
bool isOutOfRange(int x, int y);

// ビットを1に変換
inline void setBit(array<uint64_t, 4>& bitboard, int y, int x);

// ビットを0に変換
inline void clearBit(array<uint64_t, 4>& bitboard, int y, int x);

// 対象位置のビット確認
inline bool checkBit(const array<uint64_t, 4>& bitboard, int y, int x);

//* ==================================================
//*     関数実装
//* ==================================================

// Zobristハッシュの初期化
void initZobristTable() {
    mt19937_64 rng(random_device{}());
    for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; ++i) {
        ZobristTable[i][0] = rng();
        ZobristTable[i][1] = rng();
        ZobristTable[i][2] = rng();
    }
}

// トランスポジションテーブルを参照する関数（ロックを追加）
bool probeTranspositionTable(uint64_t hashKey, int depth, int alpha, int beta, int& outValue) {
    shared_lock lock(TableMutex); // 読み込み時は共有ロック
    auto it = TranspositionTable.find(hashKey);

    if (it != TranspositionTable.end()) {
        const TranspositionEntry& entry = it->second;

        if (entry.depth >= depth) {
            if (entry.boundType == BoundType::EXACT) {
                outValue = entry.value;
                return true;
            } else if (entry.boundType == BoundType::LOWER_BOUND && entry.value >= beta) {
                outValue = entry.value;
                return true;
            } else if (entry.boundType == BoundType::UPPER_BOUND && entry.value <= alpha) {
                outValue = entry.value;
                return true;
            }
        }
    }
    return false;
}

// トランスポジションテーブルに結果を保存する関数（ロックを追加）
void storeTransposition(uint64_t hashKey, int depth, int value, BoundType boundType) {
    unique_lock lock(TableMutex); // 書き込み時は排他ロック
    TranspositionTable[hashKey] = {depth, value, boundType};
}

// ハッシュキーの更新
void updateHashKey(int y, int x, int stone) {
    CurrentHashKey ^= ZobristTable[y * BOARD_SIZE + x][stone];
}

// 範囲外判定
bool isOutOfRange(int x, int y) {
    return x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE;
}

// 満杯確認
bool isBoardFull(const array<uint64_t, 4>& computerBitboard, const array<uint64_t, 4>& opponentBitboard) {
    array<uint64_t, 4> combined;
    for (int i = 0; i < 4; i++) {
        combined[i] = computerBitboard[i] | opponentBitboard[i];  // 黒と白のビットボードを OR 演算
    }

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

// 勝利確認
GameSet isWin(int board[][BOARD_SIZE], int stone) {
    // 4方向の探索
    for (const auto& [dy, dx] : DIRECTIONS) {
        int count = 0;

        for (int i = -4; i < 5; i++) {
            int y = i * dy;
            int x = i * dx;

            if (isOutOfRange(x, y)) continue;

            if (board[y][x] == stone) {
                count++;
                if (count == 5) return GameSet::WIN;
            } else {
                count = 0;
            }
        }
    }

    return GameSet::CONTINUE;
}

// ビットボードを操作する関数
// 指定位置を1にする
inline void setBit(array<uint64_t, 4>& bitboard, int y, int x) {
    int pos = y * BOARD_SIZE + x;
    bitboard[pos / 64] |= (1ULL << (pos % 64));
}

// 指定位置を0にする
inline void clearBit(array<uint64_t, 4>& bitboard, int y, int x) {
    int pos = y * BOARD_SIZE + x;
    bitboard[pos / 64] &= ~(1ULL << (pos % 64));
}

// 指定位置のビットが1か0か
inline bool checkBit(const array<uint64_t, 4>& bitboard, int y, int x) {
    int pos = y * BOARD_SIZE + x;
    return bitboard[pos / 64] & (1ULL << (pos % 64));
}

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
            } else if (board[y][x] == OpponentStone) { // 白石
                OpponentBitboard[part] |= (1ULL << offset);
            }
        }
    }
}

// 石の並びを評価する関数
// pair<個数, 石の色>
int evaluateLine(const vector<pair<int, int>>& line) {
    int score = 0;
    auto front = line.begin();
    auto back = line.end();
    bool openEnd = false;

    for (auto it = front; it != back; ++it) {
        int count = it->first;
        int stone = it->second;
        int stoneSign = (stone == ComStone) ? 1 : -1;

        // 範囲外はスキップ
        if (stone == STONE_OUT) continue;
    }

    return score;
}

// 石の周辺を評価
int evaluateStoneRange(int board[][BOARD_SIZE], const int row, const int col) {
    int score = 0;

    // 4方向の探索
    for (const auto& [dy, dx] : DIRECTIONS) {
        vector<pair<int, int>> line;

        for (int i = -4; i < 5; i++) {
            int y = row + i * dy;
            int x = col + i * dx;

            if (line.empty()) { // 配列が空かどうか
                if (!isOutOfRange(x, y)) line.push_back({1, board[y][x]});
                else                    line.push_back({0, STONE_OUT});

            } else {
                if (isOutOfRange(x, y))                    line.push_back({0, STONE_OUT});
                else if (line.back().second == board[y][x]) line.back().first++;
                else                                        line.push_back({1, board[y][x]});

            }

            // デバッグ用出力
            // if (board[y][x] == STONE_SPACE) {
            //     cout << "空 ";
            // } else if (board[y][x] == STONE_BLACK) {
            //     cout << "黒 ";
            // } else if (board[y][x] == STONE_WHITE) {
            //     cout << "白 ";
            // } else {
            //     cout << "外 ";
            // }
            // if (i == 4) cout << endl;
        }

        // デバッグ用
        // for (const auto& [length, stone] : line) {
        //     cout << "(" << length << ", " << stone << ") ";
        // }
        // cout << endl;
        // this_thread::sleep_for(chrono::milliseconds(25));

        score += evaluateLine(line);
    }

    return score;
}


// ボード全体の評価
int evaluateBoard(int board[][BOARD_SIZE]) {
    int totalScore = 0;

    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {

            if (board[row][col] != STONE_SPACE) totalScore = evaluateStoneRange(board, row, col);
        }
    }

    return totalScore;
}

// アルファ・ベータ法
int alphaBeta(array<uint64_t, 4>& computer, array<uint64_t, 4>& opponent, int depth, int alpha, int beta, bool isMaximizingPlayer) {
    int stone = isMaximizingPlayer ? ComStone : OpponentStone;
    int eval;

    //boardPrint(board);
    // 一秒待機
    // cout << "hashkey: " << CurrentHashKey << endl;
    // this_thread::sleep_for(chrono::milliseconds(25));

    // トランスポーテーションテーブルの確認
    // if (probeTranspositionTable(CurrentHashKey, depth, alpha, beta, eval)) {
    //     return eval;
    // }

    // 勝利判定の確認
    // if (isWin(board, stone)) return isMaximizingPlayer ? INF : -INF;

    // 探索の末端のとき
    if (depth == MAX_DEPTH) {
        // eval = evaluateBoard(board);
        storeTransposition(CurrentHashKey, depth, eval, BoundType::EXACT);
        return eval;
    }

    // アルファ・ベータ法の本編
    if (isMaximizingPlayer) {
        int maxEval = -INF;

        for (const auto& [y, x] : SPIRAL_MOVES) {
            if (!checkBit(computer, y, x) && !checkBit(opponent, y, x)) {
                setBit(computer, y, x);
                updateHashKey(y, x, ComStone);
                History.push_back({{y, x}, ComStone});

                int eval = alphaBeta(computer, opponent, depth + 1, alpha, beta, false);

                clearBit(computer, y, x);
                updateHashKey(y, x, ComStone);
                maxEval = max(maxEval, eval);
                alpha = max(alpha, eval);
                if (beta <= alpha) break; // Beta cut-off
            }
        }
        //BoundType boundType = (maxEval <= alpha) ? BoundType::UPPER_BOUND : ((maxEval >= beta) ? BoundType::LOWER_BOUND : BoundType::EXACT);
        //storeTransposition(CurrentHashKey, depth, maxEval, boundType);
        return maxEval;
    } else {
        int minEval = INF;

        for (const auto& [y, x] : SPIRAL_MOVES) {
            if (!checkBit(computer, y, x) && !checkBit(opponent, y, x)) {
                setBit(opponent, y, x);
                updateHashKey(y, x, OpponentStone);
                History.push_back({{y, x}, OpponentStone});

                int eval = alphaBeta(computer, opponent, depth + 1, alpha, beta, true);

                clearBit(opponent, y, x);
                updateHashKey(y, x, OpponentStone);
                minEval = min(minEval, eval);
                beta = min(beta, eval);
                if (beta <= alpha) break; // Alpha cut-off
            }
        }
        //BoundType boundType = (minEval <= alpha) ? BoundType::UPPER_BOUND : ((minEval >= beta) ? BoundType::LOWER_BOUND : BoundType::EXACT);
        //storeTransposition(CurrentHashKey, depth, minEval, boundType);
        return minEval;
    }
}

// 最適解探索
pair<int, int> findBestMove() {
    int bestVal = -INF;
    pair<int, int> bestMove = {-1, -1};
    vector<future<pair<int, pair<int, int>>>> futures;
    mutex mtx; // 排他制御用
    atomic<int> threadCount(0); // 実行スレッド数

    // 各手を分割して並行処理
    for (const auto& [y, x] : SPIRAL_MOVES) {
        if (!checkBit(ComputerBitboard, y, x) && !checkBit(OpponentBitboard, y, x)) { // 空白確認
            // スレッドの上限を維持
            if (futures.size() >= MAX_THREADS) {
                for (auto& fut : futures) {
                    auto [moveVal, pos] = fut.get(); // 結果を取得
                    lock_guard<mutex> lock(mtx); // 排他制御
                    if (moveVal > bestVal) {
                        bestVal = moveVal;
                        bestMove = pos;
                    }
                }
                futures.clear();
            }

            // 新しいスレッドで評価を非同期実行
            futures.emplace_back(async(launch::async, [=, &threadCount]() {
                array<uint64_t, 4> localCom = ComputerBitboard;
                array<uint64_t, 4> localOpp = OpponentBitboard;

                // ビットボードに現在の手を設定
                setBit(localCom, y, x);

                // アルファ・ベータ探索を実行
                int moveVal = alphaBeta(localCom, localOpp, 0, bestVal, INF, false);

                // スレッド数をカウント
                threadCount++;
                return make_pair(moveVal, make_pair(y, x));
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
    return bestMove;
}

// 最適解探索スレッドなし
pair<int, int> findBestMoveSample() {
    int bestVal = -INF;
    pair<int, int> bestMove = {-1, -1};

    for (const auto& [y, x] : SPIRAL_MOVES) {
        if (!checkBit(ComputerBitboard, y, x) && !checkBit(OpponentBitboard, y, x)) {
            array<uint64_t, 4> localCom = ComputerBitboard;
            array<uint64_t, 4> localOpp = OpponentBitboard;

            setBit(localCom, y, x);

            int moveVal = alphaBeta(localCom, localOpp, 0, bestVal, INF, false);

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
        OpponentStone = ComStone == STONE_BLACK ? STONE_WHITE : STONE_BLACK;

        if (ComStone == STONE_BLACK) {
            *pos_y = BOARD_SIZE / 2;
            *pos_x = BOARD_SIZE / 2;
            return 0;
        }
    }

    // 配置処理
    pair<int, int> bestMove = findBestMoveSample();
    cout << "おいた場所は( " << bestMove.first << "," << bestMove.second << " )" << endl;
    *pos_y = bestMove.first;
    *pos_x = bestMove.second;
    return 0;
}