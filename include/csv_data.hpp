#ifndef CSV_DATA_HPP
#define CSV_DATA_HPP

#include <string>
#include <vector>
#include <fstream>
#include <cstdint>

using namespace std;

struct RowData {
    uint32_t stones;
    uint32_t empty;
    uint32_t range;
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