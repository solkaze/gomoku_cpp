#include <bits/stdc++.h>

using namespace std;

#include "gomoku.hpp"

//* ==================================================
//*     構造体、クラス
//* ==================================================

enum class BoundType {
    EXACT,
    UPPER_BOUND,
    LOWER_BOUND
};

struct TranspositionEntry {
    int value;              // 評価値
    int depth;              // 探索した深さ
    BoundType boundType;    // 何の条件で保存したか (Exact, Upper Bound, Lower Bound)
};

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

// 一列の評価関数
int evaluateLine(const vector<int>& line);

// アルファ・ベータ法
int alphaBeta(int board[][BOARD_SIZE], int depth, int alpha, int beta, bool maximizingPlayer);

// 最適解探索
pair<int, int> findBestMove(int board[][BOARD_SIZE]);

// gomoku.cppに配置を返す
int calcPutPos(int board[][BOARD_SIZE], int com, int *pos_x, int *pos_y);

// 満杯確認
bool isFull(int board[][BOARD_SIZE]);

// 勝利確認
bool isWin(int board[][BOARD_SIZE], int stone);

// 範囲外確認
bool isOutOfRange(int x, int y);


//* ==================================================
//*     グローバル変数
//* ==================================================

// コンピュータの石
int comStone;

// プレイヤーの石
int playerStone;

// zobristハッシュ関数の定義
array<array<uint64_t, 3>, BOARD_SIZE * BOARD_SIZE> zobristTable;

// トランスポジションテーブルの定義
unordered_map<uint64_t, TranspositionEntry> transpositionTable;

// 共有ロック
shared_mutex tableMutex;

// ミューテックスの定義
shared_mutex transTableMutex;

// 参照
thread_local uint64_t currentHashKey = 0;


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

// コンパイル時初期化のための定数 15マス
constexpr int K_BOARD_SIZE = BOARD_SIZE;

// すべてのマスの数 225マス
constexpr int TOTAL_CELLS = K_BOARD_SIZE * K_BOARD_SIZE;

// コンパイル時に自動生成
// これにより実行時間の効率化を図る
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
// これもコンパイル時に自動生成
constexpr auto SpiralMoves = generateSpiralMoves();


//* ==================================================
//*     関数実装
//* ==================================================

// Zobristハッシュの初期化
void initZobristTable() {
    mt19937_64 rng(random_device{}());
    for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; ++i) {
        zobristTable[i][0] = rng();
        zobristTable[i][1] = rng();
        zobristTable[i][2] = rng();
    }
}

// トランスポジションテーブルを参照する関数（ロックを追加）
bool probeTranspositionTable(uint64_t hashKey, int depth, int alpha, int beta, int& outValue) {
    shared_lock lock(tableMutex); // 読み込み時は共有ロック
    auto it = transpositionTable.find(hashKey);
    if (it != transpositionTable.end()) {
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
    unique_lock lock(tableMutex); // 書き込み時は排他ロック
    transpositionTable[hashKey] = {depth, value, boundType};
}

// ハッシュキーの更新
void updateHashKey(int y, int x, int stone) {
    currentHashKey ^= zobristTable[y * BOARD_SIZE + x][stone];
}

// 範囲外確認
bool isOutOfRange(int x, int y) {
    return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
}

// 満杯確認
bool isFull(int board[][BOARD_SIZE]) {
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            if (board[i][j] == STONE_SPACE) return false;
        }
    }
    return true;
}

// 勝利確認
bool isWin(int board[][BOARD_SIZE], int stone) {
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            if (board[row][col] == stone) {
                // 8方向の探索
                for (const auto& [dy, dx] : DIRECTIONS) {
                    int count = 1;
                    for (int i = 1; i < 5; i++) {
                        int y = row + i * dy;
                        int x = col + i * dx;
                        if (isOutOfRange(x, y) && board[y][x] == stone) {
                            count++;
                        } else {
                            break;
                        }
                    }
                    if (count == 5) return true;
                }
            }
        }
    }
    return false;
}

// 石の並びを評価する関数
int evaluateLine(const vector<int>& line) {
    int score = 0;



    return score;
}


// 各方向での評価を行う
int evaluateBoard(int board[][BOARD_SIZE]) {
    int totalScore = 0;

    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            if (board[row][col] != STONE_SPACE) {
                // 8方向の探索
                for (const auto& [dy, dx] : DIRECTIONS) {
                    vector<int> line;
                    for (int i = -1; i < 5; i++) {
                        int y = row + i * dy;
                        int x = col + i * dx;
                        if (isOutOfRange(x, y)) {
                            line.emplace_back(board[y][x]);
                        } else {
                            break;
                        }
                    }

                    totalScore += evaluateLine(line);
                }
            }
        }
    }

    return totalScore;
}

// アルファ・ベータ法
int alphaBeta(int board[][BOARD_SIZE], int depth, int alpha, int beta, bool isMaximizingPlayer) {
    int stone = isMaximizingPlayer ? comStone : playerStone;
    int eval;

    //boardPrint(board);
    // 一秒待機
    // cout << "hashkey: " << currentHashKey << endl;
    // this_thread::sleep_for(chrono::milliseconds(25));

    // トランスポーテーションテーブルの確認
    // if (probeTranspositionTable(currentHashKey, depth, alpha, beta, eval)) {
    //     return eval;
    // }

    // 探索の末端のとき
    if (depth == MAX_DEPTH || isFull(board) || isWin(board, stone)) {
        eval = evaluateBoard(board);
        storeTransposition(currentHashKey, depth, eval, BoundType::EXACT);
        return eval;
    }

    // アルファ・ベータ法の本編
    if (isMaximizingPlayer) {
        int maxEval = -INF;
        for (const auto& [y, x] : SpiralMoves) {
            if (board[y][x] == STONE_SPACE) {
                board[y][x] = comStone;
                updateHashKey(y, x, comStone);

                int eval = alphaBeta(board, depth + 1, alpha, beta, false);
                board[y][x] = STONE_SPACE;
                updateHashKey(y, x, comStone);

                maxEval = max(maxEval, eval);
                alpha = max(alpha, eval);
                if (beta <= alpha) break; // Beta cut-off
            }
        }
        //BoundType boundType = (maxEval <= alpha) ? BoundType::UPPER_BOUND : ((maxEval >= beta) ? BoundType::LOWER_BOUND : BoundType::EXACT);
        //storeTransposition(currentHashKey, depth, maxEval, boundType);
        return maxEval;
    } else {
        int minEval = INF;
        for (const auto& [y, x] : SpiralMoves) {
            if (board[y][x] == STONE_SPACE) {
                board[y][x] = playerStone;
                updateHashKey(y, x, playerStone);

                int eval = alphaBeta(board, depth + 1, alpha, beta, true);
                board[y][x] = STONE_SPACE;
                updateHashKey(y, x, playerStone);

                minEval = min(minEval, eval);
                beta = min(beta, eval);
                if (beta <= alpha) break; // Alpha cut-off
            }
        }
        //BoundType boundType = (minEval <= alpha) ? BoundType::UPPER_BOUND : ((minEval >= beta) ? BoundType::LOWER_BOUND : BoundType::EXACT);
        //storeTransposition(currentHashKey, depth, minEval, boundType);
        return minEval;
    }
}

// 最適解探索
pair<int, int> findBestMove(int board[][BOARD_SIZE]) {
    int bestVal = -INF;
    pair<int, int> bestMove = {-1, -1};
    vector<future<pair<int, pair<int, int>>>> futures;
    mutex mtx; // 排他制御用
    atomic<int> threadCount(0); // 実行スレッド数

    const int max_threads = 8;

    // 各手を分割して並行処理
    for (const auto& [y, x] : SpiralMoves) {
        if (board[y][x] == STONE_SPACE) {
            // スレッドの上限を維持
            if (futures.size() >= max_threads) {
                for (auto& fut : futures) {
                    auto [moveVal, pos] = fut.get(); // 結果を取得
                    lock_guard<mutex> lock(mtx); // 排他制御
                    cout << "座標と結果: y:" << pos.first << " x:" << pos.second << " moveVal:" << moveVal << endl;
                    if (moveVal > bestVal) {
                        bestVal = moveVal;
                        bestMove = pos;
                    }
                }
                futures.clear();
            }

            // 新しいスレッドで評価を非同期実行
            futures.emplace_back(async(launch::async, [=, &board, &mtx, &threadCount]() {
                int localBoard[BOARD_SIZE][BOARD_SIZE];
                copy(&board[0][0], &board[0][0] + BOARD_SIZE * BOARD_SIZE, &localBoard[0][0]);

                localBoard[y][x] = comStone;
                int moveVal = alphaBeta(localBoard, 0, bestVal, INF, false);
                localBoard[y][x] = STONE_SPACE;

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
pair<int, int> findBestMoveSample(int board[][BOARD_SIZE]) {
    int bestVal = -INF;
    pair<int, int> bestMove = {-1, -1};

    for (const auto& [y, x] : SpiralMoves) {
        if (board[y][x] == STONE_SPACE) {
            int localBoard[BOARD_SIZE][BOARD_SIZE];
            copy(&board[0][0], &board[0][0] + BOARD_SIZE * BOARD_SIZE, &localBoard[0][0]);

            localBoard[y][x] = comStone;
            int moveVal = alphaBeta(localBoard, 0, bestVal, INF, false);
            localBoard[y][x] = STONE_SPACE;

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
        comStone = com;
        playerStone = comStone == STONE_BLACK ? STONE_WHITE : STONE_BLACK;

        if (comStone == STONE_BLACK) {
            *pos_y = BOARD_SIZE / 2;
            *pos_x = BOARD_SIZE / 2;
            return 0;
        }
    }

    // 配置処理
    pair<int, int> bestMove = findBestMoveSample(board);
    cout << "おいた場所は( " << bestMove.first << "," << bestMove.second << " )" << endl;
    *pos_y = bestMove.first;
    *pos_x = bestMove.second;
    return 0;
}