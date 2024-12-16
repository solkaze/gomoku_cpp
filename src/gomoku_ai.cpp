#include <numeric>
#include <limits>
#include <random>
#include <chrono>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <future>
#include <memory>
#include <tuple>

#include "common.hpp"
#include "prohibit.hpp"
#include "evaluate.hpp"
#include "alpha_beta.hpp"
#include "zobrist_hash.hpp"

//*==================================================
//*    関数実装
//*==================================================

pair<pair<int, int>, int> searchBestMoveAtDepth(
    int board[][BOARD_SIZE],
    int comStone,
    int oppStone,
    const vector<pair<int, int>>& moves,
    int depth,
    int previousBestVal,
    pair<int, int> previousBestMove) {

    pair<int, int> bestMove = previousBestMove;
    int bestVal = previousBestVal;
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
                    cout << pos.first << ", " << pos.second << ": " << moveVal << endl;
                    if (moveVal > bestVal) {
                        bestVal = moveVal;
                        bestMove = pos;
                    }
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
        cout << pos.first << ", " << pos.second << ": " << moveVal << endl;
        if (moveVal > bestVal) {
            bestVal = moveVal;
            bestMove = pos;
        }
    }

    // 全ローカルTTをグローバルTTにマージ
    for (const auto& localTT : localTTs) {
        localTT.mergeTo();
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
                int distA = abs(a.first - bestMove.first) + abs(a.second - bestMove.second);
                int distB = abs(b.first - bestMove.first) + abs(b.second - bestMove.second);
                return distA < distB;
            });
        }

        // 深さごとの最適手を探索
        tie(bestMove, bestVal) = searchBestMoveAtDepth(board, comStone, oppStone, sortedMoves, depth, bestVal, bestMove);

        // 深さごとの結果を表示（デバッグ用）
        cout << "深さ " << depth << " の最適手: " << bestMove.second << ", " << bestMove.first << endl;
        cout << "評価値: " << bestVal << endl;
    }

    return {bestMove, bestVal};
}

// 最適解探索
pair<int, int> findBestMove(int board[][BOARD_SIZE], int comStone, int oppStone) {
    // 反復深化探索を呼び出して最適手を取得
    auto [bestMove, bestVal] = iterativeDeepening(board, comStone, oppStone, MAX_DEPTH);
    cout << "----------\n";
    cout << "最終的な最適手: " << bestMove.second << ", " << bestMove.first << endl;
    cout << "評価値: " << bestVal << endl;
    cout << "----------\n";
    return bestMove;
}

int calcPutPos(int board[][BOARD_SIZE], int com, int *pos_x, int *pos_y) {
    static bool first = true;

    // 序盤処理の設定
    if (com == STONE_BLACK && first) {
        first = false;
        // 初手として中央を指定
        *pos_y = BOARD_SIZE / 2;
        *pos_x = BOARD_SIZE / 2;
        return 0;
    }

    auto start = chrono::high_resolution_clock::now();

    static int comStone = com;
    static int oppStone = com == STONE_BLACK ? STONE_WHITE : STONE_BLACK;

    // 配置処理
    pair<int, int> bestMove = findBestMove(board, comStone, oppStone);

    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << "----------\n";
    cout << "処理時間: " << duration.count() << " ms\n";
    cout << "----------\n";


    *pos_y = bestMove.first;
    *pos_x = bestMove.second;
    cout << "\n==========\n";
    cout << "置いた位置:( " << *pos_x << ", " << *pos_y << " )\n";
    cout << "==========" << endl;
    return 0;
}
