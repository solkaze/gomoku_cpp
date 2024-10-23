#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <ncurses.h>
#include <unistd.h>
#include <locale.h>

void initModule() {
	setlocale(LC_ALL, "");  // 文字コードをUTF-8に設定
	initscr();              // スクリーンを初期化
    keypad(stdscr, TRUE);
	cbreak();               // 入力バッファを使用しない(Enterを押さない)
	noecho();               // 入力文字を表示しない
	curs_set(0);            // カーソルを表示しない
	start_color();          // 色を利用する
}

// 色の初期設定
void initColor() {
        init_pair(1, COLOR_BLACK, COLOR_YELLOW); // 碁盤の色
        init_pair(2, COLOR_WHITE, COLOR_BLACK);  // 黒石の色
        init_pair(3, COLOR_BLACK, COLOR_WHITE);  // 白石の色
        init_pair(4, COLOR_BLACK, COLOR_RED);   // カーソルの色
	return;
}

void printField() {
    //　フィールド表示
    int delay = 3;
    attron(COLOR_PAIR(1)); // 碁盤の色を設定
    for (int i = 0; i < field.size(); ++i) mvprintw(delay + i, delay, "%s", field[i].c_str());
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
    mvprintw(delay + pointer[cur.y][cur.x].second, delay + pointer[cur.y][cur.x].first, "%s", "_");
    attroff(COLOR_PAIR(4));

    refresh();
}

int main() {
    int ch;
    cur.x = 0;
    cur.y = 0;
    initModule();
    initColor();
    printField();
    int player_turn = 1;
    while (1) { 
        ch = getch();
        if (ch == 'q') break;
        if (ch == KEY_UP) {
            cur.y = (cur.y - 1 + BORD_HEIGHT) % BORD_HEIGHT; // y座標を1減らす
        } else if (ch == KEY_DOWN) {
            cur.y = (cur.y + 1) % BORD_HEIGHT; // y座標を1増やす
        } else if (ch == KEY_LEFT) {
            cur.x = (cur.x - 1 + BORD_WIDTH) % BORD_WIDTH; // x座標を1減らす
        } else if (ch == KEY_RIGHT) {
            cur.x = (cur.x + 1) % BORD_WIDTH; // x座標を1増やす
        } else if (ch == '\n' && point[cur.y][cur.x] == 0) {
            if (player_turn == BLACK_STONE) point[cur.y][cur.x] = BLACK_STONE;
            else if (player_turn == WHITE_STONE) point[cur.y][cur.x] = WHITE_STONE;

            player_turn *= -1;
        }
        printField();
    }

    endwin();

    return 0;
}