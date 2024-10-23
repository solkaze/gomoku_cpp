#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <ncurses.h>

#include "gomoku_ai.hpp"

void Gomoku::put_stone(int y, int x, int color) {
        point[y][x] = color;
}

void Gomoku::move_player(int ch, int& turn) {
    // grid_size = 15
    switch (ch) {
        case KEY_UP:
            if (y > 0) y--;
            break;
        case KEY_DOWN:
            if (y < grid_size) y++;
            break;
        case KEY_LEFT:
            if (x > 0) x--;
            break;
        case KEY_RIGHT:
            if (x < grid_size) x++;
            break;
        case ' ':
            if (point[y][x] == 0) {
                put_stone(y, x, turn);
                turn *= -1;
            }
            break;
        default:
            break;
    }
}

void Gomoku::print_field() {
        //　フィールド表示
    int delay = 3;
    attron(COLOR_PAIR(1)); // 碁盤の色を設定
    for (size_t i = 0; i < field.size(); ++i) mvprintw(delay + i, delay, "%s", field[i].c_str());
    attroff(COLOR_PAIR(1)); // 色設定を解除
    int rows = point.size();
    int cols = point.size();

    // 上から石を配置
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (point[i][j] == 1) {
                attron(COLOR_PAIR(2));
                mvprintw(delay + pointer[i][j].second, delay + pointer[i][j].first, "%s", "O");
                attron(COLOR_PAIR(2));
            } else if (point[i][j] == -1) {
                attron(COLOR_PAIR(3));
                mvprintw(delay + pointer[i][j].second, delay + pointer[i][j].first, "%s", "O");
                attroff(COLOR_PAIR(3));
            }
        }
    }
    //　更に自身のカーソルを表示
    attron(COLOR_PAIR(4));
    mvprintw(delay + pointer[y][x].second, delay + pointer[y][x].first, "%s", "_");
    attroff(COLOR_PAIR(4));

    refresh();
}