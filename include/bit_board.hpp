#ifndef BITBOARD_AI
#define BITBOARD_AI

#include <array>
#include <cstdint>
#include <iostream>
#include <memory>

#include "gomoku_ai.hpp"
#include "gomoku.hpp"

using BitLine = array<uint64_t, BITBOARD_PARTS>;

class BitBoard {
    private:
        // ビットボード
        BitLine bitBoard;
        // 空のボード
        shared_ptr<BitLine> emptyBoard;
        // 石の色
        const int stone;

    public:
        // コンストラクタ(初期化)
        BitBoard() = delete;

        explicit BitBoard(int stone, shared_ptr<BitLine> sharedEmptyBoard) :
            bitBoard({0, 0, 0, 0}),
            emptyBoard(move(sharedEmptyBoard)),
            stone(stone)
        {}

        explicit BitBoard(int stone, int board[][BOARD_SIZE], shared_ptr<BitLine> sharedEmptyBoard) :
            bitBoard({0, 0, 0, 0}),
            emptyBoard(move(sharedEmptyBoard)),
            stone(stone)
        {
            convertToBitboards(board);
        }

        // 石の色を取得
        inline int getStone() const {
            return stone;
        }

        // 範囲外確認
        static bool isInBounds(const int y, const int x) {
            return y >= 0 && y < BOARD_SIZE && x >= 0 && x < BOARD_SIZE;
        }

        // コマを置く
        inline void setBit(const int y, const int x) {
            if (y < 0 || y >= K_BOARD_SIZE || x < 0 || x >= K_BOARD_SIZE || checkBit(y, x)) return;
            int part = (y * K_BOARD_SIZE + x) / SEGMENT_SIZE;
            int shift = (y * K_BOARD_SIZE + x) % SEGMENT_SIZE;

            bitBoard[part] |= 1ULL << shift;
            (*emptyBoard)[part] &= ~(1ULL << shift);
        }

        // コマを取る
        inline void removeBit(const int y, const int x) {
            if (y < 0 || y >= K_BOARD_SIZE || x < 0 || x >= K_BOARD_SIZE || !checkBit(y, x)) return;
            int part = (y * K_BOARD_SIZE + x) / SEGMENT_SIZE;
            int shift = (y * K_BOARD_SIZE + x) % SEGMENT_SIZE;

            bitBoard[part] &= ~(1ULL << shift);
            (*emptyBoard)[part] |= 1ULL << shift;
        }

        // コマを調べる
        inline bool checkBit(const int y, const int x) const {
            if (y < 0 || y >= K_BOARD_SIZE || x < 0 || x >= K_BOARD_SIZE) return false;
            int part = (y * K_BOARD_SIZE + x) / SEGMENT_SIZE;
            int shift = (y * K_BOARD_SIZE + x) % SEGMENT_SIZE;

            return bitBoard[part] & (1ULL << shift);
        }

        // 空白を調べる
        bool checkEmptyBit(const int y, const int x) const {
            if (y < 0 || y >= K_BOARD_SIZE || x < 0 || x >= K_BOARD_SIZE) return false;
            int part = (y * K_BOARD_SIZE + x) / SEGMENT_SIZE;
            int shift = (y * K_BOARD_SIZE + x) % SEGMENT_SIZE;

            return (*emptyBoard)[part] & (1ULL << shift);
        }

        // ２次元配列からビットボードへ
        void convertToBitboards(int board[][BOARD_SIZE]);

        // 特定のビット列を抜き出す
        pair<uint32_t, uint32_t> putOutBitLine(const int y, const int x ,const int dy, const int dx,
                                            const int start, const int end) const;

        // テスト用表示
        void testPrintBoard() const;

        // テスト用空白マス表示
        void testPrintEmptyBoard() const;
};

#endif