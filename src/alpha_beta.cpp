#include <iostream>
#include <iomanip>
#include <vector>
#include <future>
#include <algorithm>

#include "alpha_beta.hpp"
#include "evaluate.hpp"

bool Survey = false;

shared_mutex globalTTMutex;

void testPrintBoard(const BitBoard& com, const BitBoard& opp) {
    cout << "   ";
    for(int i = 0; i < BOARD_SIZE; i++) {
        if(i / 10)
            printf("%d ", i / 10);
        else
            printf("  ");
    }
    cout << endl;
    cout << "   ";
    for(int i = 0; i < BOARD_SIZE; i++) {
        cout << i % 10 << " ";
    }
    cout << endl;
    for (int i = 0; i < BOARD_SIZE; i++) {
        cout << setw(2) << i << " ";
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (com.checkBit(i, j)) {
                if (com.getStone() == STONE_BLACK)
                    cout << "● ";
                else cout << "○ ";
            } else if (opp.checkBit(i, j)) {
                if (opp.getStone() == STONE_BLACK)
                    cout << "● ";
                else cout << "○ ";
            } else {
                cout << "・";
            }
        }
        cout << endl;
    }
    cout << endl;
}

int alphaBeta(BitBoard& computer, BitBoard& opponent,
            int depth, int alpha, int beta, TransportationTable& localTT, bool isMaximizingPlayer, pair<int, int> put) {

    int cachedEval;
    if (localTT.retrieveEntry(depth, alpha, beta, cachedEval, isMaximizingPlayer)) {
        return cachedEval;
    }

    switch(isWin(computer, opponent, put)) {
        case GameSet::WIN:
            return  SCORE_FIVE;
            break;
        case GameSet::LOSE:
            return -SCORE_FIVE;
            break;
        case GameSet::CONTINUE:
            break;
    }

    if (depth == 0) {
        return evaluate(computer, opponent);
    }

    if (isMaximizingPlayer) {
        int maxEval = -INF;

        for (const auto& [y, x] : SPIRAL_MOVES) {
            if (computer.checkEmptyBit(y, x)) {

                computer.setBit(y, x);
                localTT.updateHashKey(computer.getStone(), y, x);

                int eval = alphaBeta(computer, opponent, depth - 1, alpha, beta, localTT, false, make_pair(y, x));

                computer.removeBit(y, x);
                localTT.updateHashKey(computer.getStone(), y, x);

                maxEval = std::max(maxEval, eval);
                alpha   = std::max(alpha, eval);

                if (beta <= alpha) { // ベータカット
                    localTT.storeEntry(depth, maxEval, BoundType::LOWER_BOUND);
                    return maxEval;
                }
            }
        }

        localTT.storeEntry(depth, maxEval, BoundType::EXACT);

        return maxEval;
    } else {
        int minEval = INF;

        for (const auto& [y, x] : SPIRAL_MOVES) {
            if (opponent.checkEmptyBit(y, x)) {

                opponent.setBit(y, x);
                localTT.updateHashKey(opponent.getStone(), y, x);

                int eval = alphaBeta(computer, opponent, depth - 1, alpha, beta, localTT, true, make_pair(y, x));

                opponent.removeBit(y, x);
                localTT.updateHashKey(opponent.getStone(), y, x);

                minEval = std::min(minEval, eval);
                beta    = std::min(beta, eval);

                if (beta <= alpha) { // アルファカット
                    localTT.storeEntry(depth, minEval, BoundType::UPPER_BOUND);
                    return minEval;
                }
            }
        }

        localTT.storeEntry(depth, minEval, BoundType::EXACT);

        return minEval;
    }
}

// ハッシュ関数を定義（pairをキーにする場合に必要）
struct pair_hash {
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2>& p) const {
        return std::hash<T1>()(p.first) ^ std::hash<T2>()(p.second);
    }
};

std::unordered_map<std::pair<int, int>, int, pair_hash> historyHeuristic;

pair<pair<int, int>, int> searchBestMoveAtDepth(
    int board[][BOARD_SIZE],
    int comStone,
    int oppStone,
    const vector<pair<int, int>>& moves,
    int depth) {

    pair<int, int> bestMove = {-1, -1};
    int bestVal = -INF;
    vector<future<pair<int, pair<int, int>>>> futures;
    mutex mtx; // 排他制御用

    vector<TransportationTable> localTTs; // ローカルTTのリスト
    localTTs.reserve(moves.size());

    for (const auto& [y, x] : moves) {
        if (board[y][x] == STONE_SPACE) { // 空白確認
            if (futures.size() >= MAX_THREADS) {
                // スレッドの結果を収集
                for (auto& fut : futures) {
                    auto [moveVal, pos] = fut.get();
                    lock_guard<mutex> lock(mtx);
                    // cout << "moveVal:\t" << moveVal << " pos:\t" << pos.first << ", " << pos.second << endl;
                    if (moveVal > bestVal) {
                        bestVal = moveVal;
                        bestMove = pos;
                    }
                    historyHeuristic[pos] = moveVal;
                }
                futures.clear();
            }

            // 非同期タスクで評価を実行
            futures.emplace_back(async(launch::async, [=, &bestVal, &localTTs, &mtx]() {
                auto emptyBoard = make_shared<BitLine>();
                fill(emptyBoard->begin(), emptyBoard->end(), 0xFFFFFFFFFFFFFFFF);

                BitBoard localCom(comStone, board, emptyBoard);
                BitBoard localOpp(oppStone, board, emptyBoard);

                TransportationTable localTT(board); // 各スレッドで独自のTTを作成

                // ビットボードに現在の手を設定
                localCom.setBit(y, x);
                localTT.updateHashKey(localCom.getStone(), y, x);

                // アルファ・ベータ探索を実行
                int moveVal = alphaBeta(localCom, localOpp, depth, bestVal, INF, localTT, false, make_pair(y, x));


                // ローカルTTをリストに保存
                {
                    lock_guard<mutex> lock(mtx);
                    localTTs.push_back(move(localTT));
                }

                return make_pair(moveVal, make_pair(y, x));
            }));
        }
    }

    // 残りのスレッド結果を収集
    for (auto& fut : futures) {
        auto [moveVal, pos] = fut.get();
        lock_guard<mutex> lock(mtx);
        // cout << "moveVal:\t" << moveVal << " pos:\t" << pos.first << ", " << pos.second << endl;
        if (moveVal > bestVal) {
            bestVal = moveVal;
            bestMove = pos;
        }
        historyHeuristic[pos] = moveVal;
    }

    // 全ローカルTTをグローバルTTにマージ
    for (const auto& localTT : localTTs) {
        localTT.mergeTo();
    }

    return {bestMove, bestVal};
}

// スレッドなしバージョン
pair<pair<int, int>, int> searchBestMoveAtDepthNoThread(
    int board[][BOARD_SIZE],
    int comStone,
    int oppStone,
    const vector<pair<int, int>>& moves,
    int depth) {

    pair<int, int> bestMove = {-1, -1};
    int bestVal = -INF;

    for (const auto& [y, x] : moves) {
        if (board[y][x] == STONE_SPACE) { // 空白確認
            auto emptyBoard = make_shared<BitLine>();
            fill(emptyBoard->begin(), emptyBoard->end(), 0xFFFFFFFFFFFFFFFF);

            BitBoard localCom(comStone, board, emptyBoard);
            BitBoard localOpp(oppStone, board, emptyBoard);

            TransportationTable localTT(board); // 各スレッドで独自のTTを作成

            // ビットボードに現在の手を設定
            localCom.setBit(y, x);
            localTT.updateHashKey(localCom.getStone(), y, x);
            // アルファ・ベータ探索を実行
            int moveVal = alphaBeta(localCom, localOpp, depth, bestVal, INF, localTT, false, make_pair(y, x));

            if (moveVal > bestVal) {
                bestVal = moveVal;
                bestMove = make_pair(y, x);
            }

            localTT.mergeTo();
        }
    }

    return {bestMove, bestVal};
}

// 反復深化探索
pair<pair<int, int>, int> iterativeDeepening(
    int board[][BOARD_SIZE], int comStone, int oppStone, int maxDepth) {
    pair<int, int> bestMove = {-1, -1}; // 最適手
    int bestVal = -INF;                // 初期評価値

    // 動的にソート可能なコピーを作成
    vector<pair<int, int>> sortedMoves(SPIRAL_MOVES.begin(), SPIRAL_MOVES.end());

    // 反復深化探索
    for (int depth = 1; depth <= maxDepth; ++depth) {
        cout << "探索深さ: " << depth << endl;

        // 前回の最適手を基にソート（初回はそのまま）
        if (bestMove.first != -1 && bestMove.second != -1) {
            sort(sortedMoves.begin(), sortedMoves.end(), [&](const pair<int, int>& a, const pair<int, int>& b) {
                return historyHeuristic[a] > historyHeuristic[b];
            });
            historyHeuristic.clear();
        }

        // 深さごとの最適手を探索
        tie(bestMove, bestVal) = searchBestMoveAtDepthNoThread(board, comStone, oppStone, sortedMoves, depth);

        // 深さごとの結果を表示（デバッグ用）
        cout << "深さ " << depth << " の最適手: " << bestMove.second << ", " << bestMove.first << endl;
        cout << "評価値: " << bestVal << endl;
        if (bestVal >= SCORE_FIVE && depth == 1) break;
    }

    return {bestMove, bestVal};
}