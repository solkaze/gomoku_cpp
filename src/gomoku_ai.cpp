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

    for (const auto& [dy, dx] : moves) {
        if (board[dy][dx] == STONE_SPACE) { // 空白確認
            if (futures.size() >= MAX_THREADS) {
                // スレッドの結果を収集
                for (auto& fut : futures) {
                    auto [moveVal, pos] = fut.get();
                    lock_guard<mutex> lock(mtx);
                    if (moveVal > bestVal) {
                        bestVal = moveVal;
                        bestMove = pos;
                    }
                }
                futures.clear();
            }

            // 非同期タスクで評価を実行
            futures.emplace_back(async(launch::async, [=, &bestVal]() {
                auto emptyBoard = make_shared<BitLine>();
                fill(emptyBoard->begin(), emptyBoard->end(), 0xFFFFFFFFFFFFFFFF);

                BitBoard localCom(comStone, emptyBoard);
                BitBoard localOpp(oppStone, emptyBoard);

                localCom.convertToBitboards(board);
                localOpp.convertToBitboards(board);

                // ビットボードに現在の手を設定
                localCom.setBit(dy, dx);
                History.push({localCom.getStone(), {dy, dx}});

                // アルファ・ベータ探索を実行
                int moveVal = alphaBeta(localCom, localOpp, depth, bestVal, INF, false);

                History.pop();

                return make_pair(moveVal, make_pair(dy, dx));
            }));
        }
    }

    // 残りのスレッド結果を収集
    for (auto& fut : futures) {
        auto [moveVal, pos] = fut.get();
        lock_guard<mutex> lock(mtx);
        if (moveVal > bestVal) {
            bestVal = moveVal;
            bestMove = pos;
        }
    }

    return {bestMove, bestVal};
}


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
                return (a == bestMove) > (b == bestMove);
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


pair<int, int> findBestMove(int board[][BOARD_SIZE], int comStone, int oppStone) {
    // 反復深化探索を呼び出して最適手を取得
    auto [bestMove, bestVal] = iterativeDeepening(board, comStone, oppStone, MAX_DEPTH);
    cout << "最終的な最適手: " << bestMove.second << ", " << bestMove.first << endl;
    cout << "評価値: " << bestVal << endl;
    return bestMove;
}

// 最適解探索
pair<int, int> findBestMoveDefault(int board[][BOARD_SIZE], int comStone, int oppStone) {
    int bestVal = -INF;
    pair<int, int> bestMove = {-1, -1};
    vector<future<pair<int, pair<int, int>>>> futures;
    mutex mtx; // 排他制御用

    // if (isOppFour(ComputerBitboard, OpponentBitboard, pos_y, pos_x) && !isComFour(ComputerBitboard, OpponentBitboard)) {
    //     return make_pair(pos_y, pos_x);
    // }
    // 各手を分割して並行処理
    for (const auto& [dy, dx] : SPIRAL_MOVES) {

        if (board[dy][dx] == STONE_SPACE) { // 空白確認
            // スレッドの上限を維持
            if (futures.size() >= MAX_THREADS) {
                for (auto& fut : futures) {
                    auto [moveVal, pos] = fut.get(); // 結果を取得
                    cout << pos.first << ", " << pos.second << ": " << moveVal << endl;
                    lock_guard<mutex> lock(mtx); // 排他制御
                    if (moveVal > bestVal) {
                        bestVal = moveVal;
                        bestMove = pos;
                    }
                }
                futures.clear();
            }

            // 新しいスレッドで評価を非同期実行
            futures.emplace_back(async(launch::async, [=, &bestVal]() {
                auto emptyBoard = make_shared<BitLine>();
                fill(emptyBoard->begin(), emptyBoard->end(), 0xFFFFFFFFFFFFFFFF);

                BitBoard localCom(comStone, emptyBoard);
                BitBoard localOpp(oppStone, emptyBoard);

                localCom.convertToBitboards(board);
                localOpp.convertToBitboards(board);

                // ビットボードに現在の手を設定
                localCom.setBit(dy, dx);
                History.push({localCom.getStone(), {dy, dx}});

                // アルファ・ベータ探索を実行
                int moveVal = alphaBeta(localCom, localOpp, MAX_DEPTH, bestVal, INF, false);

                History.pop();

                return make_pair(moveVal, make_pair(dy, dx));
            }));
        }

    }

    // 残りのスレッド結果を反映
    for (auto& fut : futures) {
        auto [moveVal, pos] = fut.get();
        lock_guard<mutex> lock(mtx);
        if (moveVal > bestVal) {
            bestVal = moveVal;
            bestMove = pos;
        }
    }


    cout << "最適手: " << bestMove.first << ", " << bestMove.second << endl << bestVal << endl;
    return bestMove;
}

// スレッドなしバージョン
pair<int, int> findBestMoveNoThread(int board[][BOARD_SIZE], int comStone, int oppStone) {
    int bestVal = -INF;
    pair<int, int> bestMove = {-1, -1};

    for (const auto& [dy, dx] : SPIRAL_MOVES) {
        if (board[dy][dx] == STONE_SPACE) { // 空白確認
            auto emptyBoard = make_shared<BitLine>();
            fill(emptyBoard->begin(), emptyBoard->end(), 0xFFFFFFFFFFFFFFFF);

            BitBoard localCom(comStone, emptyBoard);
            BitBoard localOpp(oppStone, emptyBoard);

            localCom.convertToBitboards(board);
            localOpp.convertToBitboards(board);

            // ビットボードに現在の手を設定
            localCom.setBit(dy, dx);
            History.push({localCom.getStone(), {dy, dx}});

            // アルファ・ベータ探索を実行
            int moveVal = alphaBeta(localCom, localOpp, MAX_DEPTH, bestVal, INF, false);

            localCom.removeBit(dy, dx);
            History.pop();

            if (moveVal > bestVal) {
                bestVal = moveVal;
                bestMove = make_pair(dy, dx);
            }
        }
    }
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

    static int comStone = com;
    static int oppStone = com == STONE_BLACK ? STONE_WHITE : STONE_BLACK;

    // 配置処理
    pair<int, int> bestMove = findBestMove(board, comStone, oppStone);

    *pos_y = bestMove.first;
    *pos_x = bestMove.second;

    cout << "置いた位置:( " << *pos_x << ", " << *pos_y << " )" << endl;
    return 0;
}
