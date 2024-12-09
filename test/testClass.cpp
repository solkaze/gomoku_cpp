#include <array>
#include <iostream>
#include <cstdint>
#include <bitset>


using namespace std;

const int BOARD_SIZE = 15;
const int K_BOARD_SIZE = 15;
const int BITBOARD_PARTS = 4;
const int SEGMENT_SIZE = 64;

using BitLine = array<uint64_t, BITBOARD_PARTS>;
class BitBoard {
    private:
        BitLine bitboard;
        static BitLine emptyBoard;

        // 石の色
        const int stone;

    public:
        // コンストラクタ(初期化)
        BitBoard() = delete;

        BitBoard(int stone) : bitboard({0, 0, 0, 0}), stone(stone) {}

        // 範囲外確認
        static inline bool isInBounds(int y, int x);

        // コマを置く
        inline void setBit(int y, int x);

        // コマを取る
        inline void removeBit(int y, int x);

        // コマを調べる
        inline bool checkBit(int y, int x) const;

        // ２次元配列からビットボードへ
        void convertToBitboards(int board[][BOARD_SIZE]);

        // 石の色を取得
        int getStone() const;

        // 空白ボードのコマを置く
        static void setEmptyBit(int y, int x);

        // 空白ボードのコマを消す
        static void removeEmptyBit(int y, int x);

        // 空白を調べる
        static inline bool checkEmptyBit(int y, int x);

        // テスト用表示
        void testPrintBoard() const;

        // テスト用空白マス表示
        static void testPrintEmptyBoard();
};

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


int main() {
    int board[BOARD_SIZE][BOARD_SIZE] = {0};
    board[0][0] = 1;
    board[0][1] = 2;
    board[1][0] = 2;
    board[1][1] = 1;

    BitBoard bitBoard(1);
    BitBoard subBoard(2);
    bitBoard.convertToBitboards(board);
    subBoard.convertToBitboards(board);

    bitBoard.setBit(2, 0);
    bitBoard.setBit(2, 2);

    subBoard.setBit(2, 1);
    subBoard.setBit(1, 2);


    bitBoard.setBit(14, 14);
    subBoard.removeBit(1, 1);
    bitBoard.testPrintBoard();
    cout << endl;
    subBoard.testPrintBoard();
    cout << endl;
    BitBoard::testPrintEmptyBoard();
    if (BitBoard::checkEmptyBit(2, 1)) {
        cout << "true" << endl;
    } else {
        cout << "false" << endl;
    }

    return 0;
}