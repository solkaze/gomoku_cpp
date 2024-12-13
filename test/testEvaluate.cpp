#include "testClass.hpp"
#include "testCsv.hpp"
#include <iostream>
CSVData fiveLowMASK("../data/five_mask.csv");

const auto FIVE_LOW_MASK = fiveLowMASK.getData();
bool fiveLow(const BitBoard& bitBoard, const int y, const int x) {

    for(const auto& [dy, dx] : DIRECTIONS) {
        auto [line, _ ] = bitBoard.putOutBitLine(y, x, dy, dx, -4, 4);

        for (const auto& mask : FIVE_LOW_MASK) {
            int sarchLine = line & mask[2];

            if (sarchLine == mask[0]) return true;
        }
    }
    return false;
}