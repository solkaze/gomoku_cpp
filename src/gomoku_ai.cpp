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
#include "gomoku.hpp"

// 目標5段
const int max_dipth = 3;
const int INF = std::numeric_limits<int>::max();

const std::pair<int, int> directions[4] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};

enum class Player { EMPTY, PLAYER1, PLAYER2 };
using Board = std::vector<std::vector<Player>>;

int alpha_beta(int& Board) {

}

std::pair<int, int> find_best_move(int Board[][BOARD_SIZE]) {
    int best_score = -INF;
    std::pair<int, int> best_move = {-1, -1};

	return best_move;
}

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

	std::pair<int, int> best_move = find_best_move(board);

	*pos_x = best_move.first;
	*pos_y = best_move.second;

	return 0;
}