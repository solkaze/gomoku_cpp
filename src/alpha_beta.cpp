#include "alpha_beta.hpp"

#include <algorithm>
#include <future>
#include <iomanip>
#include <iostream>
#include <vector>

#include "evaluate.hpp"

//*====================
//* 構造体定義
//*====================
struct pair_hash {
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2>& p) const {
        return std::hash<T1>()(p.first) ^ std::hash<T2>()(p.second);
    }
};

//*====================
//* プロトタイプ宣言
//*====================

array<pair<int, int>, LIMIT_SEARCH_MOVE> generateSearchMoves(int y, int x);

void testPrintBoard(const BitBoard& com, const BitBoard& opp);

int alphaBeta(BitBoard& computer, BitBoard& opponent, int depth, int alpha,
              int beta, bool isMaximizingPlayer, pair<int, int> put,
              TransportationTable& localTT);

//*====================
//* グローバル変数
//*====================

// 履歴ヒューリスティック
unordered_map<pair<int, int>, int, pair_hash> historyHeuristic;

// スレッドの共有ロック
shared_mutex globalTTMutex;

// 探索の配列
array<pair<int, int>, LIMIT_SEARCH_MOVE> SearchMoves =
    generateSearchMoves(K_BOARD_SIZE / 2, K_BOARD_SIZE / 2);

//*====================
//* 関数実装
//*====================

array<pair<int, int>, LIMIT_SEARCH_MOVE> generateSearchMoves(int y, int x) {
    array<pair<int, int>, LIMIT_SEARCH_MOVE> moves{};
    int cy = y, cx = x;
    moves[0] = {cy, cx};

    constexpr int directions[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};

    int steps = 1;
    int index = 1;

    while (index < LIMIT_SEARCH_MOVE) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < steps && index < LIMIT_SEARCH_MOVE; ++j) {
                cy += directions[i][0];
                cx += directions[i][1];
                if (cy >= 0 && cy < K_BOARD_SIZE && cx >= 0 &&
                    cx < K_BOARD_SIZE) {
                    moves[index++] = {cy, cx};
                }
            }
            if (i % 2 == 1) ++steps;
        }
    }
    return moves;
}

void testPrintHistoryHeuristic(vector<pair<int, int>>& moves) {
    for (const auto& [y, x] : moves) {
        cout << "(" << y << ", " << x << ")->\t" << historyHeuristic[{y, x}]
             << endl;
    }
    this_thread::sleep_for(chrono::seconds(1));
}

void testPrintBoard(const BitBoard& com, const BitBoard& opp) {
    cout << "   ";
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (i / 10)
            printf("%d ", i / 10);
        else
            printf("  ");
    }
    cout << endl;
    cout << "   ";
    for (int i = 0; i < BOARD_SIZE; i++) {
        cout << i % 10 << " ";
    }
    cout << endl;
    for (int i = 0; i < BOARD_SIZE; i++) {
        cout << setw(2) << i << " ";
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (com.checkBit(i, j)) {
                if (com.getStone() == STONE_BLACK)
                    cout << "● ";
                else
                    cout << "○ ";
            } else if (opp.checkBit(i, j)) {
                if (opp.getStone() == STONE_BLACK)
                    cout << "● ";
                else
                    cout << "○ ";
            } else {
                cout << "・";
            }
        }
        cout << endl;
    }
    cout << endl;
}

int alphaBeta(BitBoard& computer, BitBoard& opponent, int depth, int alpha,
              int beta, bool isMaximizingPlayer, pair<int, int> put,
              TransportationTable& localTT) {
    int cachedEval;
    if (localTT.retrieveEntry(depth, alpha, beta, cachedEval,
                              isMaximizingPlayer)) {
        return cachedEval;
    }

    switch (isWin(computer, opponent, put)) {
        case GameSet::WIN:
            return SCORE_FIVE;
            break;
        case GameSet::LOSE:
            return -SCORE_FIVE;
            break;
        case GameSet::CONTINUE:
            break;
    }

    if (depth == 0) return evaluate(computer, opponent);

    if (isMaximizingPlayer) {
        int maxEval = -INF;

        for (const auto& [y, x] : SearchMoves) {
            if (computer.checkEmptyBit(y, x)) {
                computer.setBit(y, x);
                localTT.updateHashKey(computer.getStone(), y, x);

                int eval = alphaBeta(computer, opponent, depth - 1, alpha, beta,
                                     false, make_pair(y, x), localTT);

                computer.removeBit(y, x);
                localTT.updateHashKey(computer.getStone(), y, x);

                maxEval = std::max(maxEval, eval);
                alpha   = std::max(alpha, eval);

                if (beta <= alpha) {  // ベータカット
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

                int eval = alphaBeta(computer, opponent, depth - 1, alpha, beta,
                                     true, make_pair(y, x), localTT);

                opponent.removeBit(y, x);
                localTT.updateHashKey(opponent.getStone(), y, x);

                minEval = std::min(minEval, eval);
                beta    = std::min(beta, eval);

                if (beta <= alpha) {  // アルファカット
                    localTT.storeEntry(depth, minEval, BoundType::UPPER_BOUND);
                    return minEval;
                }
            }
        }

        localTT.storeEntry(depth, minEval, BoundType::EXACT);

        return minEval;
    }
}

pair<pair<int, int>, int> searchBestMoveAtDepth(
    int board[][BOARD_SIZE], int comStone, int oppStone,
    const vector<pair<int, int>>& moves, int depth) {
    pair<int, int> bestMove = {-1, -1};
    int bestVal             = -INF;
    vector<future<pair<int, pair<int, int>>>> futures;
    mutex mtx;  // 排他制御用

    vector<TransportationTable> localTTs;  // ローカルTTのリスト
    localTTs.reserve(moves.size());

    for (const auto& [y, x] : moves) {
        if (board[y][x] == STONE_SPACE) {  // 空白確認
            if (futures.size() >= MAX_THREADS) {
                // スレッドの結果を収集
                for (auto& fut : futures) {
                    auto [moveVal, pos] = fut.get();
                    lock_guard<mutex> lock(mtx);
                    // cout << "moveVal:\t" << moveVal << " pos:\t" << pos.first
                    // << ", " << pos.second << endl;
                    if (moveVal > bestVal) {
                        bestVal  = moveVal;
                        bestMove = pos;
                    }
                    historyHeuristic[pos] = moveVal;
                }
                futures.clear();
            }

            // 非同期タスクで評価を実行
            futures.emplace_back(async(launch::async, [=, &bestVal, &localTTs,
                                                       &mtx]() {
                auto emptyBoard = make_shared<BitLine>();
                fill(emptyBoard->begin(), emptyBoard->end(),
                     0xFFFFFFFFFFFFFFFF);

                BitBoard localCom(comStone, board, emptyBoard);
                BitBoard localOpp(oppStone, board, emptyBoard);

                TransportationTable localTT(
                    board);  // 各スレッドで独自のTTを作成

                // ビットボードに現在の手を設定
                localCom.setBit(y, x);
                localTT.updateHashKey(localCom.getStone(), y, x);

                // アルファ・ベータ探索を実行
                int moveVal = alphaBeta(localCom, localOpp, depth, bestVal, INF,
                                        false, make_pair(y, x), localTT);

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
        // cout << "moveVal:\t" << moveVal << " pos:\t" << pos.first << ", " <<
        // pos.second << endl;
        if (moveVal > bestVal) {
            bestVal  = moveVal;
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
    int board[][BOARD_SIZE], int comStone, int oppStone,
    const vector<pair<int, int>>& moves, int depth) {
    pair<int, int> bestMove = {-1, -1};
    int bestVal             = -INF;

    for (const auto& [y, x] : moves) {
        if (board[y][x] == STONE_SPACE) {  // 空白確認
            auto emptyBoard = make_shared<BitLine>();
            fill(emptyBoard->begin(), emptyBoard->end(), 0xFFFFFFFFFFFFFFFF);

            // ビットボードの初期化
            BitBoard localCom(comStone, board, emptyBoard);
            BitBoard localOpp(oppStone, board, emptyBoard);

            TransportationTable localTT(
                board);  // 各スレッドで独自のトランスポーテーションテーブルを作成

            // ビットボードに現在の手を設定
            localCom.setBit(y, x);
            localTT.updateHashKey(localCom.getStone(), y, x);

            // アルファ・ベータ探索を実行
            int moveVal = alphaBeta(localCom, localOpp, depth, bestVal, INF,
                                    false, make_pair(y, x), localTT);

            if (moveVal > bestVal) {
                bestVal  = moveVal;
                bestMove = make_pair(y, x);
            }

            localTT.mergeTo();
        }
    }

    return {bestMove, bestVal};
}
// 反復深化探索
pair<pair<int, int>, int> iterativeDeepening(int board[][BOARD_SIZE],
                                             int comStone, int oppStone,
                                             int maxDepth) {
    pair<int, int> bestMove = {-1, -1};  // 最適手
    int bestVal             = -INF;      // 初期評価値

    // 動的にソート可能なコピーを作成
    vector<pair<int, int>> moves(SPIRAL_MOVES.begin(), SPIRAL_MOVES.end());

    // 反復深化探索
    for (int depth = 0; depth < maxDepth; ++depth) {
        cout << "----------\n";
        cout << "探索深度: " << depth + 1 << endl;
        cout << "探索サイズ" << moves.size() << endl;

        // 前回の最適手を基にソート（初回はそのまま）
        if (bestMove.first != -1 && bestMove.second != -1) {
            sort(moves.begin(), moves.end(),
                 [&](const pair<int, int>& a, const pair<int, int>& b) {
                     if (a == bestMove) return true;
                     if (b == bestMove) return false;
                     return historyHeuristic[a] > historyHeuristic[b];
                 });
            historyHeuristic.clear();
        }

        // 深さごとの最適手を探索
        tie(bestMove, bestVal) =
            searchBestMoveAtDepth(board, comStone, oppStone, moves, depth);

        // 探索対象を更新
        SearchMoves = generateSearchMoves(bestMove.first, bestMove.second);
        moves.assign(SearchMoves.begin(), SearchMoves.end());

        // 深さごとの結果を表示（デバッグ用）
        cout << "深度 " << depth + 1 << " の最適手: " << bestMove.second << ", "
             << bestMove.first << endl;
        cout << "評価値: " << bestVal << endl;
        cout << "----------\n";

        // 勝利が確定するならすぐ処理を止める
        if (bestVal >= SCORE_FIVE) break;
    }

    return {bestMove, bestVal};
}