#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>


#include "testCsv.hpp"

using namespace std;

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
                cerr << "Too many columns in CSV row: " << line << endl;
                break;
            }

            if (cell.find_first_not_of("01") != string::npos) {
                cerr << "Invalid binary value in CSV: " << cell << endl;
                continue;
            }

            row[colIndex] = std::stoi(cell, nullptr, 2);
            ++colIndex;
        }

        if (colIndex != 3) {
            cerr << "Row does not have exactly 3 columns: " << line << endl;
            continue;
        }

        data.push_back(row);
    }

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