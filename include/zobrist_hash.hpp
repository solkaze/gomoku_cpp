#ifndef ZOBRIST_HASH_HPP
#define ZOBRIST_HASH_HPP

#include "common.hpp"
#include <unordered_map>

enum BoundType {
    EXACT,
    LOWER_BOUND,
    UPPER_BOUND
};

struct TTEntry {
    int score; // 評価スコア
    int depth; // 探索の深さ
    BoundType flag; // エントリのフラグ
};

class TransportationTable {
    private:

        array<array<array<uint64_t, 2>, BOARD_SIZE>, BOARD_SIZE> zobristTable; // Zobristハッシュテーブル

        unordered_map<uint64_t, TTEntry> table;

        uint64_t currentHashKey;

        void initializeZobristTable();

    public:
        TransportationTable() : zobristTable{}, table{}, currentHashKey(0) {
            initializeZobristTable();
        }

        void updateHashKey(int player, const int y, const int x) {
            currentHashKey ^= zobristTable[y][x][player - 1];
        }

        uint64_t getHashKey() const {
            return currentHashKey;
        }

        void storeEntry(int dipth, int score, BoundType flag) {
            table[currentHashKey] = {score, dipth, flag};
        }

        bool retrieveEntry(int depth, int& alpha, int& beta, int& score, bool isMaximizingPlayer) const;

        void mergeTo(TransportationTable& globalTT) const;

        void clear() {
            table.clear();
        }
};

#endif