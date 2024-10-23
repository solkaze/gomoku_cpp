#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <ncurses.h>
#include <unistd.h>
#include <locale.h>

#include "gomoku_ai.hpp"

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

int main() {
    int ch;
    Gomoku gomoku;
    initModule();
    initColor();
    gomoku.print_field();
    int player_turn = 1;
    while (1) { 
        ch = getch();
        if (ch == 'q') break;

        gomoku.move_player(ch, player_turn);
        
        gomoku.print_field();
    }

    endwin();

    return 0;
}