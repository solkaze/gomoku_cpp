#include <iostream>
#include <bitset>

#include "testClass.hpp"


int main() {
    int board[BOARD_SIZE][BOARD_SIZE] = {0};

    board[0][0] = 1;
    board[1][1] = 1;
    board[2][2] = 1;
    board[3][3] = 1;

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
        auto tmp = bitBoard.putOutBitLine(y, x, -1, -1, -1, 4);
        int result = tmp.first;
        int empty = tmp.second;

        cout << bitset<5>(result) << endl;
        cout << bitset<5>(empty) << endl;

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