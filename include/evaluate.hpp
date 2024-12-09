#ifndef EVALUATE
#define EVALUATE

#include <thread>
#include <stack>
#include "common.hpp"

enum class GameSet {
    WIN,
    LOSE,
    PROHIBITED,
    CONTINUE,
};

// 履歴用スタック
extern thread_local stack<pair<int, pair<int, int>>> History;

int evaluate(BitBoard& computer, BitBoard& opponent);

#endif