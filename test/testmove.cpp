#include <iostream>
#include <bitset>
#include <iomanip>
#include <thread>

#include "testClass.hpp"
#include "testCsv.hpp"
#include "testProhibit.hpp"
#include "testEvaluate.hpp"

void testPrintBoard(BitBoard& com, BitBoard& opp) {
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
                cout << "● ";
            } else if (opp.checkBit(i, j)) {
                cout << "○ ";
            } else {
                cout << "・";
            }
        }
        cout << endl;
    }
    cout << endl;
}

// constexpr int BOARD_SIZE = 15;
// constexpr int DIRECTIONS[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}}; // 横、縦、斜め（2方向）

constexpr int LIMIT_SEARCH_MOVE = 81;

array<pair<int, int>, LIMIT_SEARCH_MOVE> generateLimitMoves(int y, int x) {
    array<pair<int, int>, LIMIT_SEARCH_MOVE> moves{};
    int cy = y, cx = x;
    moves[0] = {cy, cx};

    const int directions[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
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


int main() {
    // 15x15の盤面を表す2次元配列
    int board[15][15] = {
    //   0  1  2  3  4  5  6  7  8  9 10 11 12 13 14
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 0行目
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 1行目
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 2行目
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 3行目
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 4行目
        {0, 0, 0, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 5行目
        {0, 0, 0, 0, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0}, // 6行目
        {0, 0, 0, 0, 1, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0}, // 7行目
        {0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0}, // 8行目
        {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0}, // 9行目
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 10行目
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 11行目
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 12行目
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 13行目
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 14行目
    };
    auto emptyBoard = make_shared<BitLine>();
    fill(emptyBoard->begin(), emptyBoard->end(), 0xFFFFFFFFFFFFFFFF);
    BitBoard bitBoard(1, board, emptyBoard);
    BitBoard subBoard(2, board, emptyBoard);
    // cout << endl;
    // cout << "自分用ボード" << endl;
    // bitBoard.testPrintBoard();
    // cout << endl;
    // cout << "相手用ボード" << endl;
    // subBoard.testPrintBoard();
    testPrintBoard(bitBoard, subBoard);
    evaluate(bitBoard, subBoard);
    // int a, b;
    // switch (checkAdvantage(board, 1, 2,a,b)) {
    //     case Advantage::COM:
    //         cout << "脅威なし" << endl;
    //         break;
    //     case Advantage::OPP:
    //         cout << "脅威あり" << endl;
    //         break;
    //     case Advantage::DRAW:
    //         cout << "脅威なし" << endl;
    //         break;
    // }
    // cout << "(y x)に石を置きます" << endl;

    return 0;
}