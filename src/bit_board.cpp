#include "bit_board.hpp"
#include "gomoku.hpp"
#include "gomoku_ai.hpp"

BitLine BitBoard::emptyBoard = []() {
    BitLine board = {};
    board.fill(0xFFFFFFFFFFFFFFFF);
    return board;
}();

inline bool BitBoard::isInBounds(int y, int x){
    return y >= 0 && y < BOARD_SIZE && x >= 0 && x < BOARD_SIZE;
}

inline void BitBoard::setBit(int y, int x) {
    if (!isInBounds(y, x) || checkBit(y, x)) return;

    int part = (y * K_BOARD_SIZE + x) / SEGMENT_SIZE;
    int shift = (y * K_BOARD_SIZE + x) % SEGMENT_SIZE;
    bitboard[part] |= 1ULL << shift;
    setEmptyBit(y, x);
}

inline void BitBoard::removeBit(int y, int x) {
    if (!isInBounds(y, x) || !checkBit(y, x)) return;

    int part = (y * K_BOARD_SIZE + x) / SEGMENT_SIZE;
    int shift = (y * K_BOARD_SIZE + x) % SEGMENT_SIZE;
    bitboard[part] &= ~(1ULL << shift);
    removeEmptyBit(y, x);
}

inline bool BitBoard::checkBit(int y, int x) const {
    if (!isInBounds(y, x)) return false;

    int part = (y * K_BOARD_SIZE + x) / SEGMENT_SIZE;
    int shift = (y * K_BOARD_SIZE + x) % SEGMENT_SIZE;
    return (bitboard[part] & (1ULL << shift)) != 0;
}

void BitBoard::convertToBitboards(int board[][BOARD_SIZE]) {
    for (int y = 0; y < K_BOARD_SIZE; ++y) {
        for (int x = 0; x < K_BOARD_SIZE; ++x) {
            if (board[y][x] == stone) {
                setBit(y, x);
            }
        }
    }
}

int BitBoard::getStone() const {
    return stone;
}

void BitBoard::setEmptyBit(int y, int x) {
    if (!isInBounds(y, x)) return;

    int part = (y * K_BOARD_SIZE + x) / SEGMENT_SIZE;
    int shift = (y * K_BOARD_SIZE + x) % SEGMENT_SIZE;
    emptyBoard[part] &= ~(1ULL << shift);
}

void BitBoard::removeEmptyBit(int y, int x) {
    if (!isInBounds(y, x)) return;

    int part = (y * K_BOARD_SIZE + x) / SEGMENT_SIZE;
    int shift = (y * K_BOARD_SIZE + x) % SEGMENT_SIZE;
    emptyBoard[part] |= 1ULL << shift;
}

bool BitBoard::checkEmptyBit(int y, int x) {
    if (!isInBounds(y, x)) return false;

    int part = (y * K_BOARD_SIZE + x) / SEGMENT_SIZE;
    int shift = (y * K_BOARD_SIZE + x) % SEGMENT_SIZE;
    return (emptyBoard[part] & (1ULL << shift)) != 0;
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