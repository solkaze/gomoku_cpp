#include <array>

#include "prohibit.hpp"
#include "csv_data.hpp"

// 33禁判定
CSVData threePrihibitCSV("data/three_prohibit_mask.csv");
CSVData fourProhibitCSV("data/four_prohibit_mask.csv");

const auto THREE_PROHIBIT_MASK = threePrihibitCSV.getData();
const auto FOUR_PROHIBIT_MASK = fourProhibitCSV.getData();

bool isProhibitedThreeThree(const BitBoard& bitBoard, int y, int x) {
    int threeCount = 0;

    for (const auto& [dy, dx] : DIRECTIONS) {
        auto [line, empty] = bitBoard.putOutBitLine(y, x, dy, dx, -4, 4);

        for (const auto& mask : THREE_PROHIBIT_MASK) {
            uint32_t filteredLine = line & mask.range;
            if (filteredLine != mask.stones) continue;

            uint32_t filteredEmpty = empty & mask.range;
            if (filteredLine == mask.stones && filteredEmpty == mask.empty) ++threeCount;
        }
    }
    if (threeCount >= 2) return true;
    return false;
}

// 44禁判定
bool isProhibitedFourFour(const BitBoard& bitBoard, int y, int x) {
    int fourCount = 0;

    for (const auto& [dy, dx] : DIRECTIONS) {
        auto [line, empty] = bitBoard.putOutBitLine(y, x, dy, dx, -5, 5);

        for (const auto& mask : FOUR_PROHIBIT_MASK) {
            uint32_t filteredLine = line & mask.range;
            if (filteredLine != mask.stones) continue;

            uint32_t filteredEmpty = empty & mask.range;
            if (filteredLine == mask.stones && filteredEmpty == mask.empty) ++fourCount;
        }
    }
    if (fourCount >= 2) return true;
    return false;
}

// 長連禁判定
bool isProhibitedLongLens(const BitBoard& bitBoard, int y, int x) {
    for (const auto& [dy, dx] : DIRECTIONS) {
        int longCount = 1;

        // 正方向
        for (int step = 1; step < 6; ++step) {
            int ny = y + step * dy;
            int nx = x + step * dx;

            if (bitBoard.isInBounds(ny, nx)) break;

            if (bitBoard.checkEmptyBit(ny, nx)) break;        // 空白
            else if (bitBoard.checkBit(ny, nx)) ++longCount;  // 自分
            else break;                                       // 相手
        }

        // 負方向
        for (int step = 1; step < 6; ++step) {
            int ny = y - step * dy;
            int nx = x - step * dx;

            if (bitBoard.isInBounds(ny, nx)) break;

            if (bitBoard.checkEmptyBit(ny, nx)) break;       // 空白
            else if (bitBoard.checkBit(ny, nx)) ++longCount;  // 自分
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