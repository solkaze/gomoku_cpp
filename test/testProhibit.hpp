#ifndef PROHIBITED_HPP
#define PROHIBITED_HPP

#include "testClass.hpp"
#include "testCsv.hpp"

extern CSVData threeOpenCSV;

bool isProhibitedThreeThree(const BitBoard& bitBoard, int y, int x);

bool isProhibitedFourFour(const BitBoard& bitBoard, int y, int x);

#endif
