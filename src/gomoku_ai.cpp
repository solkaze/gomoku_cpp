#include <iostream>
#include <algorithm>
#include <vector>
#include <utility>
#include <numeric>
#include <chrono>
#include <thread>
#include <array>
#include <future>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <shared_mutex>
#include <random>

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
//*     関数宣言
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
int evaluateLine(int board[][BOARD_SIZE], int stone);

// 石ごとの評価関数
int evaluateStone(int stone, int consecutive, int openEnds);

// アルファ・ベータ法
int alphaBeta(int board[][BOARD_SIZE], int depth, int alpha, int beta, bool maximizingPlayer);

// 最適解探索
std::pair<int, int> findBestMove(int board[][BOARD_SIZE]);

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
std::array<std::array<uint64_t, 3>, BOARD_SIZE * BOARD_SIZE> zobristTable;

// トランスポジションテーブルの定義
std::unordered_map<uint64_t, TranspositionEntry> transpositionTable;

// 共有ロック
std::shared_mutex tableMutex;

// ミューテックスの定義
std::shared_mutex transTableMutex;

// 参照
thread_local uint64_t currentHashKey = 0;


//* ==================================================
//*     定数
//* ==================================================

// int型の最大数
constexpr int INF = std::numeric_limits<int>::max();

// スコアの設定
constexpr int SCORE_FIVE_IN_A_ROW = 1000000;
constexpr int SCORE_OPEN_FOUR     = 10000;
constexpr int SCORE_CLOSED_FOUR   = 5000;
constexpr int SCORE_OPEN_THREE    = 1000;
constexpr int SCORE_CLOSED_THREE  = 500;

// 方向の決定
constexpr std::array<std::array<int, 2>, 8> DIRECTIONS_WIN = {{
    {-1, 0},
    {0, 1},
    {1, 0},
    {0, -1},
    {-1, -1},
    {-1, 1},
    {1, -1},
    {1, 1}
}};

// 評価関数用方向
constexpr std::array<std::array<int, 2>, 4> DIRECTIONS_EVA = {{
    {0, 1},
    {1, 0},
    {1, 1},
    {1, -1}
}};

// アルファ・ベータ法最大深度
constexpr int MAX_DEPTH = 4;

// コンパイル時初期化のための定数 15マス
constexpr int kBoardSize = BOARD_SIZE;

// すべてのマスの数 225マス
constexpr int TOTAL_CELLS = kBoardSize * kBoardSize;

// コンパイル時に自動生成
// これにより実行時間の効率化を図る
constexpr std::array<std::pair<int, int>, TOTAL_CELLS> generateSpiralMoves() {
    std::array<std::pair<int, int>, TOTAL_CELLS> moves{};
    int cx = kBoardSize / 2, cy = kBoardSize / 2;
    moves[0] = {cx, cy};

    const int directions[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
    int steps = 1;
    int index = 1;

    while (index < TOTAL_CELLS) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < steps && index < TOTAL_CELLS; ++j) {
                cx += directions[i][0];
                cy += directions[i][1];
                if (cx >= 0 && cx < kBoardSize && cy >= 0 && cy < kBoardSize) {
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
    std::mt19937_64 rng(std::random_device{}());
    for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; ++i) {
        zobristTable[i][0] = rng();
        zobristTable[i][1] = rng();
        zobristTable[i][2] = rng();
    }
}

// トランスポジションテーブルを参照する関数（ロックを追加）
bool probeTranspositionTable(uint64_t hashKey, int depth, int alpha, int beta, int& outValue) {
    std::shared_lock lock(tableMutex); // 読み込み時は共有ロック
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
    std::unique_lock lock(tableMutex); // 書き込み時は排他ロック
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

    for (const auto& [y, x] : SpiralMoves) {
        if (board[y][x] == stone) {
            for (const auto& [dy, dx] : DIRECTIONS_WIN) {
                int count = 1;
                for (int i = 1; i < 5; ++i) {
                    int ny = y + i * dy, nx = x + i * dx;
                    if (isOutOfRange(nx, ny)) break;
                    ++count;
                }
                if (count == 5) return true;
            }
        }
    } // (for const auto& [y, x] : SpiralMoves)

    return false;
}

// ストーンごとのスコアを取得する関数
int getScoreForStone(int stone, int consecutive, int openEnds) {
    if (stone == comStone) {
        if (consecutive >= 5) return SCORE_FIVE_IN_A_ROW;
        else if (consecutive == 4) return (openEnds == 2) ? SCORE_OPEN_FOUR : SCORE_CLOSED_FOUR;
        else if (consecutive == 3) return (openEnds == 2) ? SCORE_OPEN_THREE : SCORE_CLOSED_THREE;
    } else if (stone == playerStone) {
        // 相手の手の場合はスコアをマイナスにする
        if (consecutive >= 5) return -SCORE_FIVE_IN_A_ROW;
        else if (consecutive == 4) return (openEnds == 2) ? -SCORE_OPEN_FOUR : -SCORE_CLOSED_FOUR;
        else if (consecutive == 3) return (openEnds == 2) ? -SCORE_OPEN_THREE : -SCORE_CLOSED_THREE;
    }
    return 0;  // それ以外の場合
}

// 石の並びを評価する関数
int evaluateLine(const std::vector<int>& line) {
    int score = 0;
    int consecutive = 0;
    int openEnds = 0;

    for (size_t i = 0; i < line.size(); i++) {
        if (line[i] != STONE_SPACE) {
            int stone = line[i];
            consecutive = 1;
            openEnds = 0;

            // 前方に空きマスがあれば1加算
            if (i > 0 && line[i - 1] == STONE_SPACE) openEnds++;

            // 同じ色の石がどれだけ続くかをカウント
            while (i + consecutive < line.size() && line[i + consecutive] == stone) {
                consecutive++;
            }

            // 後方に空きマスがあれば1加算
            if (i + consecutive < line.size() && line[i + consecutive] == STONE_SPACE) openEnds++;

            // 評価
            score += getScoreForStone(stone, consecutive, openEnds);

            // 評価済み部分を飛ばす
            i += consecutive - 1;
        }
    }

    return score;
}


// 各方向での評価を行う
int evaluateBoard(int board[][BOARD_SIZE]) {
    int totalScore = 0;

    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {

            if (board[row][col] != STONE_SPACE) {
                for (const auto& direction : DIRECTIONS_EVA) {
                    std::vector<int> line;
                    int x = row, y = col;

                    // 連続する石の並びを取得
                    while (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE) {
                        line.push_back(board[x][y]);
                        x += direction[0];
                        y += direction[1];
                    }

                    // ラインを評価
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

    // トランスポーテーションテーブルの確認
    // if (probeTranspositionTable(currentHashKey, depth, alpha, beta, eval)) {
    //     return eval;
    // }

    // 探索の末端のとき
    if (depth == MAX_DEPTH || isFull(board) || isWin(board, stone)) {
        eval = evaluateBoard(board);
        // storeTransposition(currentHashKey, depth, eval, BoundType::EXACT);
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

                maxEval = std::max(maxEval, eval);
                alpha = std::max(alpha, eval);
                if (beta <= alpha) break; // Beta cut-off
            }
        }
        // BoundType boundType = (maxEval <= alpha) ? BoundType::UPPER_BOUND : ((maxEval >= beta) ? BoundType::LOWER_BOUND : BoundType::EXACT);
        // storeTransposition(currentHashKey, depth, maxEval, boundType);
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

                minEval = std::min(minEval, eval);
                beta = std::min(beta, eval);
                if (beta <= alpha) break; // Alpha cut-off
            }
        }
        
        // BoundType boundType = (minEval <= alpha) ? BoundType::UPPER_BOUND : ((minEval >= beta) ? BoundType::LOWER_BOUND : BoundType::EXACT);
        // storeTransposition(currentHashKey, depth, minEval, boundType);
        return minEval;
    }
}

// 最適解探索
std::pair<int, int> findBestMove(int board[][BOARD_SIZE]) {
    int bestVal = -INF;
    std::pair<int, int> bestMove = {-1, -1};
    std::vector<std::future<std::pair<int, std::pair<int, int>>>> futures;
    std::mutex mtx; // 排他制御用
    std::atomic<int> threadCount(0); // 実行スレッド数

    const int max_threads = 8;

    // 各手を分割して並行処理
    for (const auto& [y, x] : SpiralMoves) {
        if (board[y][x] == STONE_SPACE) {
            // スレッドの上限を維持
            if (futures.size() >= max_threads) {
                for (auto& fut : futures) {
                    auto [moveVal, pos] = fut.get(); // 結果を取得
                    std::lock_guard<std::mutex> lock(mtx); // 排他制御
                    std::cout << "座標と結果: y:" << pos.first << " x:" << pos.second << " moveVal:" << moveVal << std::endl;
                    if (moveVal > bestVal) {
                        bestVal = moveVal;
                        bestMove = pos;
                    }
                }
                futures.clear();
            }

            // 新しいスレッドで評価を非同期実行
            futures.emplace_back(std::async(std::launch::async, [=, &board, &mtx, &threadCount]() {
                int localBoard[BOARD_SIZE][BOARD_SIZE];
                std::copy(&board[0][0], &board[0][0] + BOARD_SIZE * BOARD_SIZE, &localBoard[0][0]);

                localBoard[y][x] = comStone;
                int moveVal = alphaBeta(localBoard, 0, -INF, INF, true);
                localBoard[y][x] = STONE_SPACE;

                // スレッド数をカウント
                threadCount++;
                return std::make_pair(moveVal, std::make_pair(y, x));
            }));
        }
    }

    // 残りのスレッド結果を反映
    for (auto& fut : futures) {
        auto [moveVal, pos] = fut.get();
        std::lock_guard<std::mutex> lock(mtx);
        if (moveVal > bestVal) {
            bestVal = moveVal;
            bestMove = pos;
        }
    }

    // スレッド数を出力
    std::cout << "実行されたスレッド数: " << threadCount.load() << std::endl;
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

    // メイン処理
    std::pair<int, int> bestMove = findBestMove(board);
    std::cout << "おいた場所は( " << bestMove.first << "," << bestMove.second << " )" << std::endl;
    *pos_y = bestMove.first;
    *pos_x = bestMove.second;
    return 0;
}