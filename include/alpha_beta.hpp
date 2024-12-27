#ifndef ALPHA_BETA_HPP
#define ALPHA_BETA_HPP

#include <utility>

#include "zobrist_hash.hpp"

using namespace std;

pair<pair<int, int>, int> iterativeDeepening(int board[][BOARD_SIZE], int comStone, int oppStone, int maxDepth);
#endif  // ALPHA_BETA_HPP