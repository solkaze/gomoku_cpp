#include <iostream>
#include <bitset>

#include "testClass.hpp"
#include "testCsv.hpp"
#include "testProhibit.hpp"

int main() {
    int board[BOARD_SIZE][BOARD_SIZE] = {0};

    board[1][1] = 1;
    board[1][3] = 1;
    board[1][6] = 1;
    // board[3][4] = 1;

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

    if (isProhibitedFourFour(bitBoard, y, x)) {
        cout << "44禁です" << endl;
    } else {
        cout << "44禁ではありません" << endl;
    }

    bitBoard.testPrintBoard();

    return 0;
}