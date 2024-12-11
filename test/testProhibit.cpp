#include "testClass.hpp"
#include "testCsv.hpp"

CSVData globalCSV("../data/threeMask.csv");

const auto MASK_LINE = globalCSV.getData();

bool isProhibitedThreeThree(const BitBoard& bitBoard, int y, int x) {
    int threeCount = 0;

    for (const auto& [dy, dx] : DIRECTIONS) {
        auto [line, empty] = bitBoard.putOutBitLine(y, x, dy, dx, -4, 4);

        for (const auto& mask : MASK_LINE) {
            int sarchBitLine = line & mask[2];
            int emptyBitLine = empty & mask[2];

            if (sarchBitLine == mask[0] && emptyBitLine == mask[1]) ++threeCount;
        }
    }
    if (threeCount >= 2) return true;
    return false;
}