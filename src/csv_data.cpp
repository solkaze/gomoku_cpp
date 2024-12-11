#include <fstream>
#include <iostream>
#include <sstream>

#include "common.hpp"
#include "csv_data.hpp"

CSVData::CSVData(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string cell;
        vector<int> row;

        while (getline(ss, cell, ',')) {
            if (cell.find_first_not_of("01") != string::npos) {
                cerr << "Invalid binary value in CSV: " << cell << std::endl;
                continue;
            }
            row.push_back(stoi(cell, nullptr, 2));
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