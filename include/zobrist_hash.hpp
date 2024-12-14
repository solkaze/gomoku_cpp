#ifndef ZOBRIST_HASH_HPP
#define ZOBRIST_HASH_HPP

#include "common.hpp"
#include <unordered_map>

enum TranspositionTableEntryFlag {
    EXACT,
    LOWERBOUND,
    UPPERBOUND
};

class TranspositionTable {
    private:
        unordered_map<uint64_t, TranspositionTableEntryFlag> transpositionTable;
        uint64_t currentHashKey;
};

#endif