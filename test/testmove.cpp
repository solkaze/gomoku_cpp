#include <iostream>

#include "testClass.hpp"


int main() {
    int board[BOARD_SIZE][BOARD_SIZE] = {0};

    BitBoard bitBoard(1);
    BitBoard subBoard(2);
    bitBoard.convertToBitboards(board);
    subBoard.convertToBitboards(board);

    while (true) {
        static int turn = 1;
        int x, y;
        cin >> y >> x;
        if (x == -1) {
            break;
        }
        if (turn == 1) {
            bitBoard.setBit(y, x);
        } else {
            subBoard.setBit(y, x);
        }

        bitBoard.testPrintBoard();
        cout << endl;
        subBoard.testPrintBoard();
        cout << endl;
        BitBoard::testPrintEmptyBoard();
        cout << endl;

        turn *= -1;
    }

    return 0;
}