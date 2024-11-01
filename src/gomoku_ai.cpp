//------------------------------------------------------------------------
// 五目ならべAI
//  ここを作る．
//  下記のサンプルは，直前に自分が置いた石の8近傍のどこかにランダムに石を置くだけ
//
//  引数説明
//    board[BOARD_SIZE][BORARD_SIZE]：五目並べの盤
//    com ： コンピュータが白石(STONE_WHITE)か黒石(STONE_BLACK)かを表す．
//    pos_x, pos_y：石を置く場所のx座標，y座標 両方出力用
//------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <vector>
#include <cmath>
#include <numeric>
#include <thread>   // for std::this_thread::sleep_for
#include <chrono>   // for std::chrono::seconds
#include "gomoku.hpp"

// 目標5段
const int max_dipth = 3;
const int INF = std::numeric_limits<int>::max();

const std::pair<int, int> directions[4] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};

enum class Player { EMPTY, PLAYER1, PLAYER2 };
using Board = std::vector<std::vector<Player>>;

// 評価関数
int evaluate(const Board& board, Player currentPlayer) {
    // 五目並べのルールに基づいて評価を実装
    // 実装例: 現状の評価関数の一部
    return 0; // 評価値（暫定）
}

// アルファ・ベータ法
int alpha_beta(int board[][BOARD_SIZE], int dipth, int alpha, int beta, int currentPlayer) {
	const static int com = currentPlayer;
	// 処理
}

// 探索開始
std::pair<int, int> find_best_move(int board[][BOARD_SIZE], int com) {
    int best_score = -INF;
    std::pair<int, int> best_move = {-1, -1};

	// 探索初期位置
	int ai_x = BOARD_SIZE / 2;
	int ai_y = BOARD_SIZE / 2;

	int board_max = BOARD_SIZE * BOARD_SIZE;
	int direct_index = 0;
	int move_length = 1;

	int count = 0;

	// らせん型に探索
	while (true) {
		
		for (int i = 0; i < 2; ++i) {
			int dx = directions[direct_index].first;
			int dy = directions[direct_index].second;

			for (int j = 0; j < move_length; ++j) {
				if (board[ai_x][ai_y] == STONE_SPACE) {
					board[ai_x][ai_y] = com;

					int eval_score = alpha_beta(board, 0, -INF, INF, com);

					board[ai_x][ai_y] = STONE_SPACE;

					if (eval_score > best_score) {
						best_score = eval_score;
						best_move = std::make_pair(ai_x, ai_y);
					}
				}
				// 次の手に備えた処理
				++count;
				if (count >= board_max) goto END_LOOP;

				ai_x += dx;
				ai_y += dy;
			}

			// 方向転換
			direct_index = (direct_index + 1) % 4;
		}
		++move_length;
	}

	END_LOOP:

	return best_move;
}

// コマを置く処理
int calcPutPos(int board[][BOARD_SIZE], int com, int *pos_x, int *pos_y) {
	static bool start_flag = true;

	// コンピュータが初手の場合の処理
	// 先手は黒
	if (start_flag && com == STONE_BLACK) {
		*pos_x = BOARD_SIZE / 2;
		*pos_y = BOARD_SIZE / 2;
		start_flag = false;
		return 0;
	}

	std::pair<int, int> best_move = find_best_move(board, com);

	*pos_x = best_move.first;
	*pos_y = best_move.second;

	return 0;
}