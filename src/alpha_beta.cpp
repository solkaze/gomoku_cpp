#include <iostream>
#include <iomanip>
#include <vector>
#include <future>
#include <algorithm>

#include "alpha_beta.hpp"
#include "evaluate.hpp"

vector<pair<int, int>> generateLimitMoves(int y, int x);

shared_mutex globalTTMutex;

vector<pair<int, int>> SearchMoves = generateLimitMoves(K_BOARD_SIZE / 2, K_BOARD_SIZE / 2);

vector<pair<int, int>> generateLimitMoves(int y, int x) {
    vector<pair<int, int>> moves{};
    int cy = y, cx = x;
    moves.push_back({cy, cx});

    constexpr int directions[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
    int steps = 1;
    int index = 1;

    while (index < LIMIT_SEARCH_MOVE) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < steps && index < LIMIT_SEARCH_MOVE; ++j) {
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

        for (const auto& [y, x] : SearchMoves) {
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

        for (const auto& [y, x] : SearchMoves) {
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

            // ビットボードの初期化
            BitBoard localCom(comStone, board, emptyBoard);
            BitBoard localOpp(oppStone, board, emptyBoard);

            TransportationTable localTT(board); // 各スレッドで独自のトランスポーテーションテーブルを作成

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

    // 脅威を検出したら相手の置いた手を中心に探索
    int threatY = -1, threatX = -1;
    if (checkThreat(board, comStone, oppStone, threatY, threatX)) {
        cout << "脅威検出" << endl;
        cout << "脅威の手: " << threatY << ", " << threatX << endl;
        SearchMoves = generateLimitMoves(threatY, threatX);
    }

    // 動的にソート可能なコピーを作成
    vector<pair<int, int>> moves(SearchMoves.begin(), SearchMoves.end());

    // 反復深化探索
    for (int depth = 1; depth <= maxDepth; ++depth) {
        cout << "----------\n";
        cout << "探索深さ: " << depth << endl;

        // 前回の最適手を基にソート（初回はそのまま）
        if (bestMove.first != -1 && bestMove.second != -1) {
            sort(moves.begin(), moves.end(), [&](const pair<int, int>& a, const pair<int, int>& b) {
                return historyHeuristic[a] > historyHeuristic[b];
            });
            historyHeuristic.clear();
        }

        // 深さごとの最適手を探索
        tie(bestMove, bestVal) = searchBestMoveAtDepth(board, comStone, oppStone, moves, depth);

        // 深さごとの結果を表示（デバッグ用）
        cout << "深さ " << depth << " の最適手: " << bestMove.second << ", " << bestMove.first << endl;
        cout << "評価値: " << bestVal << endl;
        if (bestVal >= SCORE_FIVE && depth == 1) break;
    }

    // 探索対象を更新
    SearchMoves = generateLimitMoves(bestMove.first, bestMove.second);

    return {bestMove, bestVal};
}