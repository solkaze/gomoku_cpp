#ifndef GOMOKU_AI
#define GOMOKU_AI

using namespace std;

#include <numeric>

#include "gomoku.hpp"

constexpr int INF = numeric_limits<int>::max();

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

// 評価関数用方向
constexpr array<array<int, 2>, 4> DIRECTIONS = {{
    {0,  1},
    {1,  0},
    {1,  1},
    {1, -1}
}};

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

#endif