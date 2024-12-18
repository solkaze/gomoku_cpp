#include <fstream>
#include <iostream>
#include <sstream>

#include "common.hpp"
#include "csv_data.hpp"

CSVData::CSVData(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "ファイルを開けませんでした:\t" << filename << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string cell;
        RowData row = {0, 0, 0}; // 構造体の初期化
        size_t colIndex = 0;

        while (getline(ss, cell, ',')) {
            if (colIndex >= 3) {
                cerr << "列が不正です:\t" << line << endl;
                break;
            }

            if (cell.find_first_not_of("01") != string::npos) {
                cerr << "二進数で読み込めません:\t" << cell << endl;
                continue;
            }

            uint32_t value = std::stoi(cell, nullptr, 2);

            if (colIndex == 0) row.stones = value;
            else if (colIndex == 1) row.empty = value;
            else if (colIndex == 2) row.range = value;

            ++colIndex;
        }

        if (colIndex != 3) {
            cerr << "列が不正です:\t" << line << endl;
            continue;
        }

        data.push_back(row);
    }
    cout << "読み込み完了:\t" << filename << endl;
    file.close();
}

void CSVData::printData() const {
    for (const auto& row : data) {
        cout << row.stones << ", " << row.empty << ", " << row.range << endl;
    }
}