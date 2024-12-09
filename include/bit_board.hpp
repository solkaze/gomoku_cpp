#ifndef BITBOARD_AI
#define BITBOARD_AI

#include <array>
#include <cstdint>
#include <iostream>

#include "gomoku_ai.hpp"
#include "gomoku.hpp"

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

        // ビットボードを取得
        BitLine getBitboards() const;

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

#endif