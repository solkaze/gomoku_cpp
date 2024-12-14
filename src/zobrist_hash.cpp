#include <random>

#include "zobrist_hash.hpp"

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
    return false; // 条件に合致するエントリが見つからない
}

void TransportationTable::mergeTo(TransportationTable& globalTT) const {
    for (const auto& [key, entry] : table) {
        auto it = globalTT.table.find(key);
        if (it == globalTT.table.end() || it->second.depth < entry.depth) {
            globalTT.table[key] = entry;
        }
    }
}