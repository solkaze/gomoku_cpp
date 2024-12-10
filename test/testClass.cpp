#include <array>
#include <iostream>
#include <cstdint>
#include <bitset>

#include "testClass.hpp"

using namespace std;

BitLine BitBoard::emptyBoard = []() {
    BitLine board = {};
    board.fill(0xFFFFFFFFFFFFFFFF);
    return board;
}();

void BitBoard::convertToBitboards(int board[][BOARD_SIZE]) {
    for (int y = 0; y < K_BOARD_SIZE; ++y) {
        for (int x = 0; x < K_BOARD_SIZE; ++x) {
            if (board[y][x] == stone) {
                setBit(y, x);
            }
        }
    }
}

void BitBoard::testPrintBoard() const {
    for (int row = 0; row < K_BOARD_SIZE; ++row) {
        for (int col = 0; col < K_BOARD_SIZE; ++col) {
            cout << (checkBit(row, col) ? 'X' : '*');
        }
        cout << endl;
    }
}

void BitBoard::testPrintEmptyBoard() {
    for (int row = 0; row < K_BOARD_SIZE; ++row) {
        for (int col = 0; col < K_BOARD_SIZE; ++col) {
            cout << (checkEmptyBit(row, col) ? 'X' : '*');
        }
        cout << endl;
    }
}

int BitBoard::putOutBitLine(const int y, const int x ,const int dy, const int dx,
                                            const int start, const int end) const {
    if (start > end) return 0;
    for (int step = start; step <= end; ++step) {
        int ny = y + step * dy;
        int nx = x + step * dx;

        if (isInBounds(ny, nx)) return 0;

        if (checkBit(ny, nx)) {
            return step;
        }
    }
}