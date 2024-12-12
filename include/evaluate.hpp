#ifndef EVALUATE_HPP
#define EVALUATE_HPP

#include <thread>
#include <stack>
#include "common.hpp"

enum class GameSet {
    WIN,
    LOSE,
    PROHIBITED,
    CONTINUE
};

// 履歴用スタック
extern thread_local stack<pair<int, pair<int, int>>> History;

GameSet isWin(const BitBoard& computer, const BitBoard& opponent);

int evaluate(const BitBoard& computer, const BitBoard& opponent);

#endif