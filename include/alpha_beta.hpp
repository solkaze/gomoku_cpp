#ifndef ALPHA_BETA_HPP
#define ALPHA_BETA_HPP

#include "common.hpp"

int alphaBeta(BitBoard& computer, BitBoard& opponent, 
            int depth, int alpha, int beta, bool isMaximizingPlayer);

#endif // ALPHA_BETA_HPP