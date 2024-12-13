#ifndef TESTCSV_HPP
#define TESTCSV_HPP

#include <vector>
#include <array>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

class CSVData {

    private:
        vector<array<int, 3>> data;

    public:
        explicit CSVData(const string& filename);

        // データを取得する
        const vector<array<int, 3>>& getData() const {
            return data;
        }

        // データを出力する
        void printData() const;

};

#endif // TESTCSV_HPP