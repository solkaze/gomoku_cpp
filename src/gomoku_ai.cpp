#include <iostream>
#include <random>
#include <ctime>
#include <algorithm>
#include <vector>
#include <cmath>
#include <utility>
#include <numeric>
#include <chrono>
#include <thread>
#include <array>

#include "gomoku.hpp"

//* ==========
//* 関数宣言
//* ==========

// 評価関数
int evaluate(int board[][BOARD_SIZE]);

// アルファ・ベータ法
int alphaBeta(int board[][BOARD_SIZE], int depth, int alpha, int beta, bool maximizingPlayer);

// 最適な動きの導出
std::pair<int, int> findBestMove(int board[][BOARD_SIZE]);

// gomoku.cppに配置を返す
int calcPutPos(int board[][BOARD_SIZE], int com, int *pos_x, int *pos_y);

// 満杯確認
bool isFull(int board[][BOARD_SIZE]);

// 勝利確認
bool isWin(int board[][BOARD_SIZE], int stone);

//* ==========
//* グローバル変数
//* ==========

// コンピュータの石
int comStone;

// プレイヤーの石
int playerStone;

// int型の最大数
const int INF = std::numeric_limits<int>::max();

// アルファ・ベータ法最大深度
const int MAX_DEPTH = 4;

// コンパイル時初期化のための定数 15マス
constexpr int kBoardSize = BOARD_SIZE;

// すべてのマスの数 225マス
constexpr int TOTAL_CELLS = kBoardSize * kBoardSize;

//* ==========
//* 定数
//* ==========

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

//* ==========
//* 主要関数実装
//* ==========

// 満杯確認
bool isFull(int board[][BOARD_SIZE]) {
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            if (board[i][j] == STONE_SPACE) return false;
        }
    }
    return true;
}

// アルファ・ベータ法
int alphaBeta(int board[][BOARD_SIZE], int depth, int alpha, int beta, bool isMaximizingPlayer) {
    // これ以上探索しないとき
    if (depth == 0 || isFull(board)) {
        return evaluate(board);
    }

    if (isMaximizingPlayer) {
        int maxEval = -INF;
        for (const auto& [x, y] : SpiralMoves) {
            if (board[y][x] == STONE_SPACE) {
                board[y][x] = comStone;
                int eval = alphaBeta(board, depth - 1, alpha, beta, false);
                board[y][x] = STONE_SPACE;
                maxEval = std::max(maxEval, eval);
                alpha = std::max(alpha, eval);
                if (beta <= alpha) break;
            }
        }
        return maxEval;
    } else {
        int minEval = INF;
        for (const auto& [x, y] : SpiralMoves) {
            if (board[y][x] == STONE_SPACE) {
                board[y][x] = playerStone;
                int eval = alphaBeta(board, depth - 1, alpha, beta, true);
                board[y][x] = STONE_SPACE;
                minEval = std::min(minEval, eval);
                beta = std::min(beta, eval);
                if (beta <= alpha) break;
            }
        }
        return minEval;
    }
}

// コマ配置
int calcPutPos(int board[][BOARD_SIZE], int com, int *pos_x, int *pos_y) {

    comStone = com;
    playerStone = comStone == STONE_BLACK ? STONE_WHITE : STONE_BLACK;

    std::pair<int, int> bestMove = findBestMove(board);
    *pos_y = bestMove.first;
    *pos_x = bestMove.second;
    return 0;
}