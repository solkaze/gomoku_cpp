#include <fstream>
#include <iostream>
#include <sstream>

#include "common.hpp"
#include "csv_data.hpp"

CSVData::CSVData(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Failed to open file: " << filename << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string cell;
        array<int, 3> row = {0, 0, 0}; // 3列固定
        size_t colIndex = 0;

        while (getline(ss, cell, ',')) {
            if (colIndex >= 3) {
                cerr << "列が多すぎます: " << line << endl;
                break;
            }

            if (cell.find_first_not_of("01") != string::npos) {
                cerr << "二進数で読み込めません: " << cell << endl;
                continue;
            }

            row[colIndex] = std::stoi(cell, nullptr, 2);
            ++colIndex;
        }

        if (colIndex != 3) {
            cerr << "行が3列ではないです: " << line << endl;
            continue;
        }

        data.push_back(row);
    }
    cout << filename << "を読み込みました" << endl;
    file.close();
}

void CSVData::printData() const {
    for (const auto& row : data) {
        for (int value : row) {
            cout << value << " ";
        }
        cout << endl;
    }
}