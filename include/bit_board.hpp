#ifndef BITBOARD_AI
#define BITBOARD_AI

#include <array>
#include <cstdint>
#include <iostream>

#include "gomoku_ai.hpp"
#include "gomoku.hpp"

using namespace std;

using BitLine = array<uint64_t, BITBOARD_PARTS>;


class BitBoard {
    private:
        BitLine bitboards;
        const int stone;

    public:
        // コンストラクタ(初期化)
        BitBoard() = delete;

        BitBoard(int stone) : bitboards({0, 0, 0, 0}), stone(stone) {}

        // 範囲外確認
        inline bool isInBounds(int y, int x) const;

        // コマを置く
        inline void setBit(int y, int x);

        // コマを取る
        inline void removeBit(int y, int x);

        // コマを調べる
        inline bool checkBit(int y, int x) const;

        //　２次元配列からビットボードへ
        void convertToBitboards(int board[][BOARD_SIZE]);

        // 石の色を取得
        int getStone() const;

        // ビットボードを取得
        array<uint64_t, BITBOARD_PARTS> getBitboards() const;

        // テスト用表示
        void testPrintBoard() const;
};

#endif