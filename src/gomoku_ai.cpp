#include <numeric>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <limits>
#include <random>
#include <chrono>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <stack>
#include <atomic>
#include <future>

#include "common.hpp"
#include "prohibited.hpp"
#include "evaluate.hpp"
#include "alpha_beta.hpp"

//*==================================================
//*    関数実装
//*==================================================


//* 判定関数

// 範囲外判定
bool isOutOfRange(int x, int y) {
    return x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE;
}

// 満杯判定
bool isBoardFull(const BitLine& computerBitboard, const BitLine& opponentBitboard) {
    BitLine combined = computerBitboard | opponentBitboard;

    // 全ビットが埋まっている（= 全てのビットが 1）場合
    for (int i = 0; i < 3; i++) {
        if (combined[i] != ~0ULL) {  // 64ビットが全て1ではない
            return false;
        }
    }

    // 最後のビットボードの残り部分を確認（225ビット目以降は無視）
    int remainingBits = 225 - 64 * 3;
    uint64_t mask = (1ULL << remainingBits) - 1;  // 下位 "remainingBits" ビットのみを 1 にするマスク
    if ((combined[3] & mask) != mask) {
        return false;
    }

    return true;  // 全てのビットが埋まっている
}

std::pair<int, int> iterativeDeepening(BitBoard& computer, BitBoard& opponent,
                                        int maxDepth, int timeLimit) {
    std::pair<int, int> bestMove = {-1, -1};
    int bestEval = -INF;

    auto startTime = std::chrono::steady_clock::now();

    // 浅い深さから順に探索
    for (int depth = 1; depth <= maxDepth; ++depth) {
        int alpha = -INF, beta = INF;

        // 現在の深さでアルファ・ベータ法を実行
        std::pair<int, int> currentBestMove = {-1, -1};
        int eval = alphaBeta(computer, opponent, depth, alpha, beta, true, currentBestMove);

        // 時間制限の確認
        auto currentTime = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() > timeLimit) {
            break; // 時間切れなら探索を終了
        }

        // 現在の深さで得た最善手と評価値を保存
        bestEval = eval;
        bestMove = currentBestMove;
    }

    return bestMove;
}


// 最適解探索
pair<int, int> findBestMove(BitBoard& computer, BitBoard& opponent, int pos_y, int pos_x) {
    int bestVal = -INF;
    pair<int, int> bestMove = {-1, -1};
    vector<future<pair<int, pair<int, int>>>> futures;
    mutex mtx; // 排他制御用
    atomic<int> threadCount(0); // 実行スレッド数

    if (isOppFour(ComputerBitboard, OpponentBitboard, pos_y, pos_x) && !isComFour(ComputerBitboard, OpponentBitboard)) {
        return make_pair(pos_y, pos_x);
    }

    for (int depth = 1; depth <= MAX_DEPTH; ++depth) {
        int alpha = -INF, beta = INF;

        // 現在の深さでアルファ・ベータ法を実行
        std::pair<int, int> currentBestMove = {-1, -1};
        int eval = alphaBeta(computer, opponent, depth, alpha, beta, true, currentBestMove);

        // 現在の深さで得た最善手と評価値を保存
        bestVal = eval;
        bestMove = currentBestMove;
    }

    // 各手を分割して並行処理
    for (const auto& [dy, dx] : SPIRAL_MOVES) {

        if (!computer.checkBit(dy, dx) && !opponent.checkBit(dy, dx)) { // 空白確認
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
            futures.emplace_back(async(launch::async, [=, &bestVal, &threadCount]() {
                BitBoard localCom = computer;
                BitBoard localOpp = opponent;

                // ビットボードに現在の手を設定
                localCom.setBit(dy, dx);
                History.push({localCom.getStone(), {dy, dx}});

                // アルファ・ベータ探索を実行
                int moveVal = alphaBeta(localCom, localOpp, 0, bestVal, INF, false);

                History.pop();

                // スレッド数をカウント
                threadCount++;
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

    // スレッド数を出力
    cout << "実行されたスレッド数: " << threadCount.load() << endl;
    cout << bestVal << endl;
    return bestMove;
}

int calcPutPos(int board[][BOARD_SIZE], int com, int *pos_x, int *pos_y) {
    static BitBoard computerBitboard(com);
    static BitBoard opponentBitboard(com == STONE_BLACK ? STONE_WHITE : STONE_BLACK);

    // 序盤処理の設定
    if (com == STONE_BLACK && *pos_x == -1 && *pos_y == -1) {
        // 初手として中央を指定
        *pos_y = BOARD_SIZE / 2;
        *pos_x = BOARD_SIZE / 2;
        return 0;
    }

    // ビットボード変換
    computerBitboard.convertToBitboards(board);
    opponentBitboard.convertToBitboards(board);

    // 配置処理
    pair<int, int> bestMove = findBestMove(computerBitboard, opponentBitboard, *pos_y, *pos_x);

    *pos_y = bestMove.first;
    *pos_x = bestMove.second;

    cout << "置いた位置:( " << *pos_x << ", " << *pos_y << " )" << endl;
    return 0;
}
