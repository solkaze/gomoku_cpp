#ifndef Gomoku_CPP
#define Gomoku_CPP

#include <vector>
#include <string>
#include <utility>

#define BLACK_STONE (1)
#define WHITE_STONE (-1)
class Gomoku {
public:
    Gomoku():
        point(grid_size, std::vector<int>(grid_size, 0)),
        x(0),
        y(0)
    {}

    void put_stone(int x, int y, int color);
    void print_field();

private:

    const int grid_size = 15;
    int x;
    int y;

    std::vector<std::string> field = {
        "+---+---+---+---+---+---+---+---+---+---+---+---+---+---+",
        "|   |   |   |   |   |   |   |   |   |   |   |   |   |   |",
        "+---+---+---+---+---+---+---+---+---+---+---+---+---+---+",
        "|   |   |   |   |   |   |   |   |   |   |   |   |   |   |",
        "+---+---+---+---+---+---+---+---+---+---+---+---+---+---+",
        "|   |   |   |   |   |   |   |   |   |   |   |   |   |   |",
        "+---+---+---+---+---+---+---+---+---+---+---+---+---+---+",
        "|   |   |   |   |   |   |   |   |   |   |   |   |   |   |",
        "+---+---+---+---+---+---+---+---+---+---+---+---+---+---+",
        "|   |   |   |   |   |   |   |   |   |   |   |   |   |   |",
        "+---+---+---+---+---+---+---+---+---+---+---+---+---+---+",
        "|   |   |   |   |   |   |   |   |   |   |   |   |   |   |",
        "+---+---+---+---+---+---+---+---+---+---+---+---+---+---+",
        "|   |   |   |   |   |   |   |   |   |   |   |   |   |   |",
        "+---+---+---+---+---+---+---+---+---+---+---+---+---+---+",
        "|   |   |   |   |   |   |   |   |   |   |   |   |   |   |",
        "+---+---+---+---+---+---+---+---+---+---+---+---+---+---+",
        "|   |   |   |   |   |   |   |   |   |   |   |   |   |   |",
        "+---+---+---+---+---+---+---+---+---+---+---+---+---+---+",
        "|   |   |   |   |   |   |   |   |   |   |   |   |   |   |",
        "+---+---+---+---+---+---+---+---+---+---+---+---+---+---+",
        "|   |   |   |   |   |   |   |   |   |   |   |   |   |   |",
        "+---+---+---+---+---+---+---+---+---+---+---+---+---+---+",
        "|   |   |   |   |   |   |   |   |   |   |   |   |   |   |",
        "+---+---+---+---+---+---+---+---+---+---+---+---+---+---+",
        "|   |   |   |   |   |   |   |   |   |   |   |   |   |   |",
        "+---+---+---+---+---+---+---+---+---+---+---+---+---+---+",
        "|   |   |   |   |   |   |   |   |   |   |   |   |   |   |",
        "+---+---+---+---+---+---+---+---+---+---+---+---+---+---+"
    };

    std::vector<std::vector<std::pair<int, int>>> pointer = {
        {{0, 0}, {4, 0}, {8, 0}, {12, 0}, {16, 0}, {20, 0}, {24, 0}, {28, 0}, {32, 0}, {36, 0}, {40, 0}, {44, 0}, {48, 0}, {52, 0}, {56, 0}},
        {{0, 2}, {4, 2}, {8, 2}, {12, 2}, {16, 2}, {20, 2}, {24, 2}, {28, 2}, {32, 2}, {36, 2}, {40, 2}, {44, 2}, {48, 2}, {52, 2}, {56, 2}},
        {{0, 4}, {4, 4}, {8, 4}, {12, 4}, {16, 4}, {20, 4}, {24, 4}, {28, 4}, {32, 4}, {36, 4}, {40, 4}, {44, 4}, {48, 4}, {52, 4}, {56, 4}},
        {{0, 6}, {4, 6}, {8, 6}, {12, 6}, {16, 6}, {20, 6}, {24, 6}, {28, 6}, {32, 6}, {36, 6}, {40, 6}, {44, 6}, {48, 6}, {52, 6}, {56, 6}},
        {{0, 8}, {4, 8}, {8, 8}, {12, 8}, {16, 8}, {20, 8}, {24, 8}, {28, 8}, {32, 8}, {36, 8}, {40, 8}, {44, 8}, {48, 8}, {52, 8}, {56, 8}},
        {{0, 10}, {4, 10}, {8, 10}, {12, 10}, {16, 10}, {20, 10}, {24, 10}, {28, 10}, {32, 10}, {36, 10}, {40, 10}, {44, 10}, {48, 10}, {52, 10}, {56, 10}},
        {{0, 12}, {4, 12}, {8, 12}, {12, 12}, {16, 12}, {20, 12}, {24, 12}, {28, 12}, {32, 12}, {36, 12}, {40, 12}, {44, 12}, {48, 12}, {52, 12}, {56, 12}},
        {{0, 14}, {4, 14}, {8, 14}, {12, 14}, {16, 14}, {20, 14}, {24, 14}, {28, 14}, {32, 14}, {36, 14}, {40, 14}, {44, 14}, {48, 14}, {52, 14}, {56, 14}},
        {{0, 16}, {4, 16}, {8, 16}, {12, 16}, {16, 16}, {20, 16}, {24, 16}, {28, 16}, {32, 16}, {36, 16}, {40, 16}, {44, 16}, {48, 16}, {52, 16}, {56, 16}},
        {{0, 18}, {4, 18}, {8, 18}, {12, 18}, {16, 18}, {20, 18}, {24, 18}, {28, 18}, {32, 18}, {36, 18}, {40, 18}, {44, 18}, {48, 18}, {52, 18}, {56, 18}},
        {{0, 20}, {4, 20}, {8, 20}, {12, 20}, {16, 20}, {20, 20}, {24, 20}, {28, 20}, {32, 20}, {36, 20}, {40, 20}, {44, 20}, {48, 20}, {52, 20}, {56, 20}},
        {{0, 22}, {4, 22}, {8, 22}, {12, 22}, {16, 22}, {20, 22}, {24, 22}, {28, 22}, {32, 22}, {36, 22}, {40, 22}, {44, 22}, {48, 22}, {52, 22}, {56, 22}},
        {{0, 24}, {4, 24}, {8, 24}, {12, 24}, {16, 24}, {20, 24}, {24, 24}, {28, 24}, {32, 24}, {36, 24}, {40, 24}, {44, 24}, {48, 24}, {52, 24}, {56, 24}},
        {{0, 26}, {4, 26}, {8, 26}, {12, 26}, {16, 26}, {20, 26}, {24, 26}, {28, 26}, {32, 26}, {36, 26}, {40, 26}, {44, 26}, {48, 26}, {52, 26}, {56, 26}},
        {{0, 28}, {4, 28}, {8, 28}, {12, 28}, {16, 28}, {20, 28}, {24, 28}, {28, 28}, {32, 28}, {36, 28}, {40, 28}, {44, 28}, {48, 28}, {52, 28}, {56, 28}}
    };

    std::vector<std::vector<int>> point;
};

#endif