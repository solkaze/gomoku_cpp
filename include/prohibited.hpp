#ifndef PROHIBITED
#define PROHIBITED

// 禁じ手
constexpr bool PROHIBITED_THREE_THREE  = true;
constexpr bool PROHIBITED_FOUR_FOUR    = true;
constexpr bool PROHIBITED_LONG_LENS    = false;

bool isProhibited(const BitBoard& bitBoard, int y, int x);

#endif