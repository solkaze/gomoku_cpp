#ifndef CSV_DATA_HPP
#define CSV_DATA_HPP

#include <string>
#include <vector>
#include <fstream>
#include "common.hpp"

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

#endif