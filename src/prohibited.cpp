#include <array>

#include "common.hpp"
#include "prohibited.hpp"

constexpr array<uint8_t, 3> BASE_3MASK = {
    0b01110,
    0b010110,
    0b011010,
};

constexpr array<uint8_t, 4> BASE_4MASK = {
    0b011110,
    0b0101110,
    0b0110110,
    0b0111010,
};

// 33禁判定
bool isProhibitedThreeThree(const BitBoard& bitBoard, int y, int x) {
    return false;
}

// 44禁判定
bool isProhibitedFourFour(const BitBoard& bitBoard, int y, int x) {
    int fourCount = 0;

    for (const auto& [dy, dx] : DIRECTIONS) {
        int count = 1;
        int openCount = 0;

        // 正方向
        for (int step = 1; step < 5; ++step) {
            int ny = y + step * dy;
            int nx = x + step * dx;

            if (bitBoard.isInBounds(y, x)) break; // 画面外

            if (BitBoard::checkEmptyBit(y, x)) { // 空白

            } else if (bitBoard.checkBit(y, x)) { // 自分
                ++count;
            } else break; // 相手
        }

        // 逆方向
        for (int step = 1; step < 5; ++step) {
            int ny = y - step * dy;
            int nx = x - step * dx;

            if (bitBoard.isInBounds(y, x)) break; // 画面外

            if (BitBoard::checkEmptyBit(y, x)) { // 空白

            } else if (bitBoard.checkBit(y, x)) { // 自分
                ++count;
            } else break; // 相手

            ++count;
        }

        if (count == 4) ++fourCount;
    }

    if (fourCount >= 2) return true;
    else return false;
}

// 長連禁判定
bool isProhibitedLongLens(const BitBoard& bitBoard, int y, int x) {
    for (const auto& [dy, dx] : DIRECTIONS) {
        int longCount = 1;

        // 正方向
        for (int step = 1; step < 6; ++step) {
            int ny = y + step * dy;
            int nx = x + step * dx;

            if (bitBoard.isInBounds(y, x)) break;

            if (BitBoard::checkEmptyBit(y, x)) break;       // 空白
            else if (bitBoard.checkBit(y, x)) ++longCount;  // 自分
            else break;                                     // 相手
        }

        // 負方向
        for (int step = 1; step < 6; ++step) {
            int ny = y - step * dy;
            int nx = x - step * dx;

            if (bitBoard.isInBounds(y, x)) break;

            if (BitBoard::checkEmptyBit(y, x)) break;       // 空白
            else if (bitBoard.checkBit(y, x)) ++longCount;  // 自分
            else break;                                     // 相手
        }

        if (longCount >= 6) return true;
    }

    return false;
}

bool isProhibited(const BitBoard& bitBoard, int y, int x) {
    if (PROHIBITED_THREE_THREE && isProhibitedThreeThree(bitBoard, y, x)) return true;
    if (PROHIBITED_FOUR_FOUR && isProhibitedFourFour(bitBoard, y, x)) return true;
    if (PROHIBITED_LONG_LENS && isProhibitedLongLens(bitBoard, y, x)) return true;

    return false;
}