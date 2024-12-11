#include <array>
#include <iostream>
#include <cstdint>
#include <bitset>

#include "testClass.hpp"

using namespace std;

thread_local BitLine BitBoard::emptyBoard = []() {
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
            cout << (checkBit(row, col) ? 'X' : '*') << " ";
        }
        cout << endl;
    }
}

void BitBoard::testPrintEmptyBoard() {
    for (int row = 0; row < K_BOARD_SIZE; ++row) {
        for (int col = 0; col < K_BOARD_SIZE; ++col) {
            cout << (checkEmptyBit(row, col) ? 'X' : '*') << " ";
        }
        cout << endl;
    }
}

pair<int, int> BitBoard::putOutBitLine(const int y, const int x ,const int dy, const int dx,
                                            const int start, const int end) const {
    if (start > end) return {-1, -1};

    int bitLine   = 0;
    int emptyLine = 0;

    for (int step = start; step <= end; ++step) {
        int ny = y + dy * step;
        int nx = x + dx * step;

        if (!isInBounds(ny, nx)) continue;

        if (checkBit(ny, nx)) bitLine |= (1 << (step - start));
        else if (checkEmptyBit(ny, nx)) emptyLine |= (1 << (step - start));
    }

    return make_pair(bitLine, emptyLine);
}