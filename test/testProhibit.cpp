#include "testClass.hpp"
#include "testCsv.hpp"

#include <bitset>
#include <iostream>

using namespace std;

CSVData threeOpenCSV("../data/three_prohibit_mask.csv");
CSVData fourOpenCSV("../data/four_prohibit_mask.csv");

const auto THREE_OPEN_MASK = threeOpenCSV.getData();
const auto FOUR_OPEN_MASK = fourOpenCSV.getData();

bool isProhibitedThreeThree(const BitBoard& bitBoard, int y, int x) {
    int threeCount = 0;

    for (const auto& [dy, dx] : DIRECTIONS) {
        auto [line, empty] = bitBoard.putOutBitLine(y, x, dy, dx, -4, 4);
        cout << bitset<9>(line) << " " << bitset<9>(empty) << " " << dy << " " << dx << endl;

        for (const auto& mask : THREE_OPEN_MASK) {
            int sarchBitLine = line & mask[2];
            int emptyBitLine = empty & mask[2];

            if (sarchBitLine == mask[0] && emptyBitLine == mask[1]) ++threeCount;
        }
    }
    if (threeCount >= 2) return true;
    return false;
}

bool isProhibitedFourFour(const BitBoard& bitBoard, int y, int x) {
    int fourCount = 0;

    for (const auto& [dy, dx] : DIRECTIONS) {
        auto [line, empty] = bitBoard.putOutBitLine(y, x, dy, dx, -5, 5);

        for (const auto& mask : FOUR_OPEN_MASK) {
            int sarchBitLine = line & mask[2];
            int emptyBitLine = empty & mask[2];

            if (sarchBitLine == mask[0] && emptyBitLine == mask[1]) ++fourCount;
        }
    }
    if (fourCount >= 2) return true;
    return false;
}