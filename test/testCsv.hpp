#ifndef TESTCSV_HPP
#define TESTCSV_HPP

#include <vector>
#include <array>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

struct RowData {
    int stones;
    int empty;
    int range;
};

class CSVData {

    private:
        vector<RowData> data;

    public:
        explicit CSVData(const string& filename);

        // データを取得する
        const vector<RowData>& getData() const {
            return data;
        }

        // データを出力する
        void printData() const;

};

#endif // TESTCSV_HPP