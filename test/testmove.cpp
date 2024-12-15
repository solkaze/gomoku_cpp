#include <iostream>
#include <bitset>

#include "testClass.hpp"
#include "testCsv.hpp"
#include "testProhibit.hpp"
#include "testEvaluate.hpp"

int main() {
    int board[BOARD_SIZE][BOARD_SIZE] = {0};

    // board[7][3] = 1;
    // board[7][4] = 1;
    // board[7][5] = 1;
    // board[7][6] = 1;
    // board[7][8] = 1;
    // board[7][9] = 1;
    // board[7][10] = 1;
    // board[7][11] = 1;

    // board[3][3] = 1;
    // board[4][4] = 1;
    // board[5][5] = 1;
    // board[6][6] = 1;
    // board[8][8] = 1;
    // board[9][9] = 1;
    // board[10][10] = 1;
    // board[11][11] = 1;

    // board[3][7] = 1;
    // board[4][7] = 1;
    // board[5][7] = 1;
    // board[6][7] = 1;
    // board[8][7] = 1;
    // board[9][7] = 1;
    // board[10][7] = 1;
    // board[11][7] = 1;

    board[7][7] = 1;
    board[7][8] = 1;
    board[8][5] = 1;
    board[8][6] = 1;
    board[8][7] = 1;
    board[8][8] = 1;
    board[9][6] = 1;

    board[5][5] = 2;
    board[6][4] = 2;
    board[6][5] = 2;
    board[7][5] = 2;
    board[8][4] = 2;
    board[8][9] = 2;
    board[10][5] = 2;
    // board[9][5] = 1;
    // board[10][4] = 1;
    // board[11][3] = 1;
    auto emptyBoard = make_shared<BitLine>();
    fill(emptyBoard->begin(), emptyBoard->end(), 0xFFFFFFFFFFFFFFFF);
    BitBoard bitBoard(1, board, emptyBoard);
    BitBoard subBoard(2, board, emptyBoard);
    cout << endl;
    cout << "自分用ボード" << endl;
    bitBoard.testPrintBoard();
    cout << endl;
    cout << "相手用ボード" << endl;
    subBoard.testPrintBoard();
    int x, y;
    cout << "(y x)に石を置きます" << endl;
    cin >> y >> x;
    bitBoard.setBit(y, x);
    if (isProhibitedThreeThree(bitBoard, y, x)) {
        cout << "33禁です" << endl;
    } else {
        cout << "33禁ではありません" << endl;
    }

    if (isProhibitedFourFour(bitBoard, y, x)) {
        cout << "44禁です" << endl;
    } else {
        cout << "44禁ではありません" << endl;
    }

    if (fiveLow(bitBoard, y, x)) {
        cout << "5連続です" << endl;
    } else {
        cout << "5連続ではありません" << endl;
    }

    bitBoard.testPrintBoard();

    return 0;
}