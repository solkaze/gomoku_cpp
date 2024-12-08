#include <array>
#include <iostream>
#include <cstdint>
#include <bitset>


using namespace std;

const int BOARD_SIZE = 15;
const int K_BOARD_SIZE = 15;
const int BITBOARD_PARTS = 4;
const int SEGMENT_SIZE = 64;



class BitBoard {
    private:
        array<uint64_t, BITBOARD_PARTS> bitboards;
        int stone;

    public:
        // コンストラクタ(初期化)
        BitBoard() : bitboards({0, 0, 0, 0}) {}

        BitBoard(int stone) : bitboards({0, 0, 0, 0}), stone(stone) {}

        inline bool isInBounds(int y, int x) const {
            return y >= 0 && y < K_BOARD_SIZE && x >= 0 && x < K_BOARD_SIZE;
        }

        inline void setBit(int y, int x) {
            if (!isInBounds(y, x)) return;

            int part = (y * K_BOARD_SIZE + x) / SEGMENT_SIZE;
            int shift = (y * K_BOARD_SIZE + x) % SEGMENT_SIZE;
            bitboards[part] |= 1ULL << shift;
        }

        inline void clearBit(int y, int x) {
            if (!isInBounds(y, x)) return;

            int part = (y * K_BOARD_SIZE + x) / SEGMENT_SIZE;
            int shift = (y * K_BOARD_SIZE + x) % SEGMENT_SIZE;
            bitboards[part] &= ~(1ULL << shift);
        }

        inline bool checkBit(int y, int x) const {
            if (!isInBounds(y, x)) return false;

            int part = (y * K_BOARD_SIZE + x) / SEGMENT_SIZE;
            int shift = (y * K_BOARD_SIZE + x) % SEGMENT_SIZE;
            return (bitboards[part] & (1ULL << shift)) != 0;
        }

        void convertToBitboards(int board[][BOARD_SIZE]) {
            for (int y = 0; y < K_BOARD_SIZE; ++y) {
                for (int x = 0; x < K_BOARD_SIZE; ++x) {
                    if (board[y][x] == stone) {
                        setBit(y, x);
                    }
                }
            }
        }

        void setStone(int s) {
            stone = s;
        }

        int getStone() const {
            return stone;
        }

        array<uint64_t, BITBOARD_PARTS> getBitboards() const {
            return bitboards;
        }

        void testPrintBoard() const {
            for (int row = 0; row < K_BOARD_SIZE; ++row) {
                for (int col = 0; col < K_BOARD_SIZE; ++col) {
                    cout << (checkBit(row, col) ? 'X' : '*');
                }
                cout << endl;
            }
        }
};

int main() {
    int board[BOARD_SIZE][BOARD_SIZE] = {0};
    board[0][0] = 1;
    board[0][1] = 2;
    board[1][0] = 2;
    board[1][1] = 1;

    BitBoard bitBoard(1);
    bitBoard.convertToBitboards(board);
    bitBoard.setBit(2, 0);
    bitBoard.setBit(2, 2);
    bitBoard.clearBit(1, 1);
    bitBoard.setBit(14, 14);
    bitBoard.testPrintBoard();
    if (bitBoard.checkBit(2, 1)) {
        cout << "true" << endl;
    } else {
        cout << "false" << endl;
    }

    return 0;
}