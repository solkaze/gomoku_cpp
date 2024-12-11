#include <iostream>
#include <bitset>

#include "testClass.hpp"
#include "testCsv.hpp"
#include "testProhibit.hpp"

int main() {
    int board[BOARD_SIZE][BOARD_SIZE] = {0};

    board[2][2] = 1;
    board[5][5] = 1;
    board[4][2] = 1;
    board[5][1] = 1;

    BitBoard bitBoard(1);
    BitBoard subBoard(2);
    bitBoard.convertToBitboards(board);
    subBoard.convertToBitboards(board);
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

    return 0;
}