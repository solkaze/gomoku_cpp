#include <iostream>

#include "prohibit.hpp"
#include "evaluate.hpp"
#include "alpha_beta.hpp"
#include "zobrist_hash.hpp"

//*==================================================
//*    関数実装
//*==================================================

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
    if (first) {
        first = false;
        if (com == STONE_BLACK) {
            // 初手として中央を指定
            *pos_y = BOARD_SIZE / 2;
            *pos_x = BOARD_SIZE / 2;

            cout << "==========\n";
            cout << "置いた位置:( " << *pos_x << ", " << *pos_y << " )\n";
            cout << "==========" << endl;
            return 0;
        }

    }

    auto start = chrono::high_resolution_clock::now();

    static int comStone = com;
    static int oppStone = com == STONE_BLACK ? STONE_WHITE : STONE_BLACK;

    // 配置処理
    pair<int, int> bestMove = findBestMove(board, comStone, oppStone);

    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    cout << "----------\n";
    cout << "処理時間: " << duration << "ミリ秒\n";
    cout << "----------\n";


    *pos_y = bestMove.first;
    *pos_x = bestMove.second;
    cout << "==========\n";
    cout << "置いた位置:( " << *pos_x << ", " << *pos_y << " )\n";
    cout << "==========" << endl;
    return 0;
}
