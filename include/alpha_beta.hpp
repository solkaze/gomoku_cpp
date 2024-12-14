#ifndef ALPHA_BETA_HPP
#define ALPHA_BETA_HPP

#include "common.hpp"
#include "zobrist_hash.hpp"

int alphaBeta(BitBoard& computer, BitBoard& opponent,
            int depth, int alpha, int beta, TransportationTable& localTT, bool isMaximizingPlayer);

#endif // ALPHA_BETA_HPP