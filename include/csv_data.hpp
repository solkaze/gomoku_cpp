#ifndef CSV_DATA_HPP
#define CSV_DATA_HPP

#include <string>
#include <vector>
#include <fstream>
#include "common.hpp"

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

#endif