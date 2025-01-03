#ifndef ZOBRIST_HASH_HPP
#define ZOBRIST_HASH_HPP

#include <array>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

#include "gomoku_ai.hpp"

using namespace std;

enum BoundType { EXACT, LOWER_BOUND, UPPER_BOUND };

struct TTEntry {
    int score;       // 評価スコア
    int depth;       // 探索の深さ
    BoundType flag;  // エントリのフラグ
};

class TransportationTable {
   private:
    static array<array<array<uint64_t, 2>, BOARD_SIZE>, BOARD_SIZE>
        zobristTable;  // Zobristハッシュテーブル

    static unordered_map<uint64_t, TTEntry> globalTable;
    static shared_mutex globalTableMutex;  // 読み取り用のロック

    static void initializeZobristTable();

    unordered_map<uint64_t, TTEntry> table;

    uint64_t currentHashKey;

   public:
    TransportationTable() = delete;

    TransportationTable(int board[][BOARD_SIZE]) : table{}, currentHashKey(0) {
        static once_flag flag;
        call_once(flag, initializeZobristTable);

        // ハッシュキーの初期化
        for (int y = 0; y < BOARD_SIZE; ++y) {
            for (int x = 0; x < BOARD_SIZE; ++x) {
                int stone = board[y][x];  // 石の種類（0: 空, 1: 黒, 2: 白）
                if (stone != 0) {
                    currentHashKey ^= zobristTable[y][x][stone - 1];
                }
            }
        }
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

    bool retrieveEntry(int depth, int& alpha, int& beta, int& score,
                       bool isMaximizingPlayer) const;

    bool retrieveEntryFromGlobal(int depth, int& alpha, int& beta, int& score,
                                 bool isMaximizingPlayer) const;

    void mergeTo() const;

    void clear() {
        table.clear();
    }

    int getTableSize() const {
        return table.size();
    }

    static int getGlobalTableSize() {
        return globalTable.size();
    }
};

#endif