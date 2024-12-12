#include "bit_board.hpp"
#include "gomoku.hpp"
#include "gomoku_ai.hpp"

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

pair<int, int> BitBoard::putOutBitLine(const int y, const int x, const int dy, const int dx,
                                        const int start, const int end) const {
    if (start > end) return {-1, -1};

    int bitLine = 0;
    int emptyLine = 0;

    for (int step = start; step <= end; ++step) {
        int ny = y + dy * step;
        int nx = x + dx * step;

        // 範囲外チェックを条件分岐内で行う
        if (ny < 0 || ny >= BOARD_SIZE || nx < 0 || nx >= BOARD_SIZE) continue;

        // ビットボードのチェック（インライン化されたヘルパーを使用）
        int index = (ny * K_BOARD_SIZE + nx) / SEGMENT_SIZE;
        int shift = (ny * K_BOARD_SIZE + nx) % SEGMENT_SIZE;
        uint64_t mask = (1ULL << shift);

        if (bitBoard[index] & mask) {
            bitLine |= (1 << (step - start));
        } else if ((*emptyBoard)[index] & mask) {
            emptyLine |= (1 << (step - start));
        }
    }

    return {bitLine, emptyLine};
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