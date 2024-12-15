#include <random>

#include "zobrist_hash.hpp"

array<array<array<uint64_t, 2>, BOARD_SIZE>, BOARD_SIZE> TransportationTable::zobristTable;

unordered_map<uint64_t, TTEntry> TransportationTable::globalTable;
shared_mutex TransportationTable::globalTableMutex;

void TransportationTable::initializeZobristTable() {
    mt19937_64 rng(random_device{}());
    uniform_int_distribution<uint64_t> dist;

    for (auto& row : zobristTable) {
        for (auto& col : row) {
            for (auto& piece : col) {
                piece = dist(rng);
            }
        }
    }
}

bool TransportationTable::retrieveEntry(int depth, int& alpha, int& beta, int& score, bool isMaximizingPlayer) const {
    auto it = table.find(currentHashKey);
    if (it != table.end() && it->second.depth >= depth) {
        const TTEntry& entry = it->second;

        switch (entry.flag) {
            case EXACT:
                // 正確なスコアが保存されている場合、そのまま返す
                score = entry.score;
                return true;

            case LOWER_BOUND:
                // 下限値が保存されている場合、アルファ値を調整
                if (entry.score > alpha) {
                    alpha = entry.score;
                }
                break;

            case UPPER_BOUND:
                // 上限値が保存されている場合、ベータ値を調整
                if (entry.score < beta) {
                    beta = entry.score;
                }
                break;
        }

        // アルファ値とベータ値が逆転している場合、探索をスキップ
        if (alpha >= beta) {
            score = (isMaximizingPlayer ? alpha : beta);
            return true;
        }
    }
    return retrieveEntryFromGlobal(depth, alpha, beta, score, isMaximizingPlayer); // 条件に合致するエントリが見つからない
}

bool TransportationTable::retrieveEntryFromGlobal(int depth, int& alpha, int& beta, int& score, bool isMaximizingPlayer) const {
    shared_lock<shared_mutex> lock(globalTableMutex); // グローバルテーブルへのアクセスを同期
    auto it = globalTable.find(currentHashKey);
    if (it != globalTable.end() && it->second.depth >= depth) {
        const TTEntry& entry = it->second;

        switch (entry.flag) {
            case EXACT:
                // 正確なスコアが保存されている場合、そのまま返す
                score = entry.score;
                return true;

            case LOWER_BOUND:
                // 下限値が保存されている場合、アルファ値を調整
                if (entry.score > alpha) {
                    alpha = entry.score;
                }
                break;

            case UPPER_BOUND:
                // 上限値が保存されている場合、ベータ値を調整
                if (entry.score < beta) {
                    beta = entry.score;
                }
                break;
        }

        // アルファ値とベータ値が逆転している場合、探索をスキップ
        if (alpha >= beta) {
            score = (isMaximizingPlayer ? alpha : beta);
            return true;
        }
    }
    return false; // 条件に合致するエントリが見つからない
}


void TransportationTable::mergeTo() const {
    unique_lock<shared_mutex> lock(globalTableMutex); // グローバルテーブルへのアクセスを同期
    for (const auto& [key, entry] : table) {
        auto it = globalTable.find(key);
        if (it == globalTable.end() || it->second.depth < entry.depth) {
            // エントリが存在しない、または深さが浅い場合は更新
            globalTable[key] = entry;
        }
    }
}
