#include "common.hpp"


// 33禁判定
bool isProhibitedThreeThree(const BitLine& computer, const BitLine& opponent, int y, int x, int stone) {
}

// 44禁判定
bool isProhibitedFourFour(const BitLine& computer, const BitLine& opponent, int y, int x, int stone) {
    int fourCount = 0;
    const BitLine bitBoard = stone == ComStone ? computer : opponent;
    const BitLine empty = ~(computer | opponent);

    for (const auto& [dy, dx] : DIRECTIONS) {
        int count = 1;
        int openCount = 0;

        // 正方向
        for (int step = 1; step < 5; ++step) {
            int ny = y + step * dy;
            int nx = x + step * dx;

            if (isOutOfRange(nx, ny)) break;

            if (checkBit(empty, ny, nx)) { // 空白
                if (openCount == 0) { // 飛び石の考慮
                    ++openCount;
                    continue;
                } else {
                    break;
                }
            } else if (!checkBit(bitBoard, ny, nx)) break; // 相手の石

            ++count;
        }

        // 逆方向
        for (int step = 1; step < 5; ++step) {
            int ny = y - step * dy;
            int nx = x - step * dx;

            if (isOutOfRange(nx, ny)) break;

            if (checkBit(empty, ny, nx)) { // 空白
                if (openCount == 0) { // 飛び石の考慮
                    ++openCount;
                    continue;
                } else {
                    break;
                }
            } else if (!checkBit(bitBoard, ny, nx)) break; // 相手の石

            ++count;
        }

        if (count == 4 && openCount != 0) ++fourCount;
    }

    if (fourCount >= 2) return true;

    return false;
}

// 長連禁判定
bool isProhibitedLongLens(const BitLine& bitBoard, int y, int x) {
    for (const auto& [dy, dx] : DIRECTIONS) {
        int longCount = 1;
        // 正方向
        for (int step = 1; step < 5; ++step) {
            int ny = y + step * dy;
            int nx = x + step * dx;

            if (isOutOfRange(nx, ny)) break;
            if (!checkBit(bitBoard, ny, nx)) break;
            ++longCount;
        }

        // 逆方向
        for (int step = 1; step < 5; ++step) {
            int ny = y - step * dy;
            int nx = x - step * dx;

            if (isOutOfRange(nx, ny)) break;
            if (!checkBit(bitBoard, ny, nx)) break;
            ++longCount;
        }

        if (longCount >= 6) return true;
    }

    return false;
}