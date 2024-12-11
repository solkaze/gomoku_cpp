#ifndef TESTCLASS_HPP
#define TESTCLASS_HPP

#include <iostream>
#include <array>
#include <cstdint>

using namespace std;


const int BOARD_SIZE = 15;
const int K_BOARD_SIZE = 15;
const int BITBOARD_PARTS = 4;
const int SEGMENT_SIZE = 64;

using BitLine = array<uint64_t, BITBOARD_PARTS>;

class BitBoard {
    private:
        // ビットボード
        BitLine bitBoard;
        // 空のボード
        static BitLine emptyBoard;
        // 石の色
        const int stone;

        // partのヘルパー関数
        static inline int getPart(const int y, const int x) {
            return (y * K_BOARD_SIZE + x) / SEGMENT_SIZE;
        };
        // shiftのヘルパー関数
        static inline int getShift(const int y, const int x) {
            return (y * K_BOARD_SIZE + x) % SEGMENT_SIZE;
        };
        // 空白ボードのコマを置く
        static inline void setEmptyBit(const int y, const int x) {
            if (!isInBounds(y, x)) return;
            emptyBoard[getPart(y, x)] |= 1ULL << getShift(y, x);
        };
        // 空白ボードのコマを消す
        static inline void removeEmptyBit(const int y, const int x) {
            if (!isInBounds(y, x)) return;
            emptyBoard[getPart(y, x)] &= ~(1ULL << getShift(y, x));
        };

    public:
        // コンストラクタ(初期化)
        BitBoard() = delete;

        BitBoard(int stone) : bitBoard({0, 0, 0, 0}), stone(stone) {}

        // 石の色を取得
        inline int getStone() const {
            return stone;
        };

        // 範囲外確認
        static bool isInBounds(const int y, const int x) {
            return y >= 0 && y < BOARD_SIZE && x >= 0 && x < BOARD_SIZE;
        };

        // コマを置く
        inline void setBit(const int y, const int x) {
            if (!isInBounds(y, x) || checkBit(y, x)) return;
            bitBoard[getPart(y, x)] |= 1ULL << getShift(y, x);
            removeEmptyBit(y, x);
        };

        // コマを取る
        inline void removeBit(const int y, const int x) {
            if (!isInBounds(y, x) || !checkBit(y, x)) return;
            bitBoard[getPart(y, x)] &= ~(1ULL << getShift(y, x));
            setEmptyBit(y, x);
        };

        // コマを調べる
        inline bool checkBit(const int y, const int x) const {
            if (!isInBounds(y, x)) return false;
            return bitBoard[getPart(y, x)] & (1ULL << getShift(y, x));
        };

        // 空白を調べる
        static inline bool checkEmptyBit(const int y, const int x) {
            if (!isInBounds(y, x)) return false;
            return emptyBoard[getPart(y, x)] & (1ULL << getShift(y, x));
        };

        // ２次元配列からビットボードへ
        void convertToBitboards(int board[][BOARD_SIZE]);

        // 特定のビット列を抜き出す
        pair<int, int> putOutBitLine(const int y, const int x ,const int dy, const int dx,
                                            const int start, const int end) const;

        // テスト用表示
        void testPrintBoard() const;

        // テスト用空白マス表示
        static void testPrintEmptyBoard();
};

#endif