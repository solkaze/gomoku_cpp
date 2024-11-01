//------------------------------------------------------------------------
// 五目ならべAI
//  ここを作る．
//  下記のサンプルは，直前に自分が置いた石の8近傍のどこかにランダムに石を置くだけ
//
//  引数説明
//    board[BOARD_SIZE][BORARD_SIZE]：五目並べの盤
//    com ： コンピュータが白石(STONE_WHITE)か黒石(STONE_BLACK)かを表す．
//    pos_x, pos_y：石を置く場所のx座標，y座標 両方出力用
//------------------------------------------------------------------------
#include <vector>
#include <cmath>
#include <numeric>
#include <thread>   // for std::this_thread::sleep_for
#include <chrono>   // for std::chrono::seconds
#include <future>
#include <mutex>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <shared_mutex>
#include <string>

#include "gomoku.hpp"
// 目標5段
const int max_dipth = 4;
const int INF = std::numeric_limits<int>::max();

const std::pair<int, int> directions[4] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};

// トランスポジションテーブル
std::unordered_map<uint64_t, int> transposition_table;
std::shared_mutex table_mutex;

// トランスポジションテーブルをファイルに保存する関数
void save_transposition_table(const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    for (const auto& entry : transposition_table) {
        out.write(reinterpret_cast<const char*>(&entry.first), sizeof(entry.first));
        out.write(reinterpret_cast<const char*>(&entry.second), sizeof(entry.second));
    }
}

// トランスポジションテーブルをファイルから読み込む関数
void load_transposition_table(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    uint64_t key;
    int value;
    while (in.read(reinterpret_cast<char*>(&key), sizeof(key))) {
        in.read(reinterpret_cast<char*>(&value), sizeof(value));
        transposition_table[key] = value;
    }
}

// Zobristハッシュを使って盤面をハッシュ化する関数（例）
uint64_t zobrist_hash(const int board[][BOARD_SIZE]) {
    uint64_t hash = 0;
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            if (board[i][j] != STONE_SPACE) {
                hash ^= zobrist_table[i][j][board[i][j] == STONE_BLACK ? 0 : 1];
            }
        }
    }
    return hash;
}

bool is_valid(int x_min, int y_min, int x_max, int y_max) {
    return (x_min >= 0 && x_max < BOARD_SIZE && y_min >= 0 && y_max < BOARD_SIZE);
}

int check_line(const int board[][BOARD_SIZE], int x, int y, int dx, int dy, int currentPlayer) {
    int count = 0, open_ends = 0;
    
    // 一方の端をチェック
    int nx = x - dx, ny = y - dy;
    if (is_valid(nx, ny, nx, ny) && board[ny][nx] == 0) open_ends++;
    
    // 連続する自分のコマをカウント
    for (int i = 0; i < 5; ++i) {
        nx = x + i * dx;
        ny = y + i * dy;
        if (!is_valid(nx, ny, nx, ny)) break;
        if (board[ny][nx] == currentPlayer) ++count;
        else if (board[ny][nx] != 0) return 0; // 相手のコマがある場合はスコア0
    }
    
    // もう一方の端をチェック
    nx = x + 5 * dx;
    ny = y + 5 * dy;
    if (is_valid(nx, ny, nx, ny) && board[ny][nx] == 0) open_ends++;
    
    // 評価値を計算（開放端数を考慮）
    if (count == 5) return 100000;
    else if (count == 4) return open_ends == 2 ? 10000 : 8000; // 両端空きの4連は特に高評価
    else if (count == 3) return open_ends == 2 ? 5000 : 2000; // 両端空きの3連も高評価
    else if (count == 2) return open_ends == 2 ? 300 : 100;
    else if (count == 1) return open_ends == 2 ? 10 : 5;
    return 0;
}

// 評価関数
int evaluate(const int board[][BOARD_SIZE], int currentPlayer, int com) {
    int score = 0;
    int player = (currentPlayer == com) ? 1 : -1;

    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            if (board[i][j] == currentPlayer || board[i][j] == -currentPlayer) {
                int piece = board[i][j];
                int piece_score = 0;

                // 各方向のスコアを計算
                if (is_valid(j - 4, i, j, i)) piece_score += check_line(board, j - 4, i, 1, 0, piece); // 横
                if (is_valid(j, i - 4, j, i)) piece_score += check_line(board, j, i - 4, 0, 1, piece); // 縦
                if (is_valid(j - 4, i - 4, j, i)) piece_score += check_line(board, j - 4, i - 4, 1, 1, piece); // 左斜
                if (is_valid(j - 4, i + 4, j, i)) piece_score += check_line(board, j - 4, i + 4, 1, -1, piece); // 右斜

                // 自分のコマなら加算、相手のコマなら減算（相手の場合はスコア大幅減で防御強化）
                if (piece == currentPlayer) score += piece_score;
                else score -= piece_score * 2; // 相手のスコアは2倍して防御重視
            }
        }
    }
    return score * player;
}

int alpha_beta(int board[][BOARD_SIZE], int dipth, int alpha, int beta, int currentPlayer) {
    const static int com = currentPlayer;
    if (dipth == max_dipth) {
        return evaluate(board, currentPlayer, com);
    }
    // 探索初期位置
    int ai_y = BOARD_SIZE / 2;
    int ai_x = BOARD_SIZE / 2;

    int board_max = BOARD_SIZE * BOARD_SIZE;
    int direct_index = 0;
    int move_length = 1;

    int count = 0;

    if (currentPlayer == com) {
        int max_eval = -INF;
        // らせん型に探索
        while (true) {
            
            for (int i = 0; i < 2; ++i) {
                int dy = directions[direct_index].first;
                int dx = directions[direct_index].second;

                for (int j = 0; j < move_length; ++j) {
                    if (board[ai_y][ai_x] == STONE_SPACE) {
                        board[ai_y][ai_x] = currentPlayer;

                        int eval = alpha_beta(board, dipth + 1, alpha, beta, -currentPlayer);

                        board[ai_y][ai_x] = STONE_SPACE;

                        max_eval = std::max(max_eval, eval);
                        alpha = std::max(alpha, eval);

                        if (beta <= alpha) return max_eval;
                    }
                    // 次の手に備えた処理
                    ++count;
                    if (count >= board_max) goto MAX_END_LOOP;

                    ai_y += dx;
                    ai_x += dy;
                }

                // 方向転換
                direct_index = (direct_index + 1) % 4;
            }
            ++move_length;
        }

        MAX_END_LOOP:

        return max_eval;

    } else {
        int min_eval = INF;
        while (true) {

            for (int i = 0; i < 2; ++i) {
                int dy = directions[direct_index].first;
                int dx = directions[direct_index].second;

                for (int j = 0; j < move_length; ++j) {
                    if (board[ai_y][ai_x] == STONE_SPACE) {
                        board[ai_y][ai_x] = currentPlayer;

                        int eval = alpha_beta(board, dipth + 1, alpha, beta, -currentPlayer);

                        board[ai_y][ai_x] = STONE_SPACE;

                        min_eval = std::min(min_eval, eval);
                        beta = std::min(beta, eval);

                        if (beta <= alpha) return min_eval;
                    }
                    // 次の手に備えた処理
                    ++count;
                    if (count >= board_max) goto MIN_END_LOOP;

                    ai_y += dx;
                    ai_x += dy;
                }

                // 方向転換
                direct_index = (direct_index + 1) % 4;
            }
            ++move_length;
        }
        MIN_END_LOOP:

        return min_eval;
    }
}

std::pair<int, int> find_best_move(int board[][BOARD_SIZE], int com) {
    int best_score = -INF;
    std::pair<int, int> best_move = {-1, -1};
    std::mutex mtx;  // スコア更新用のミューテックス

    int ai_x = BOARD_SIZE / 2;
    int ai_y = BOARD_SIZE / 2;

    int board_max = BOARD_SIZE * BOARD_SIZE;
    int direct_index = 0;
    int move_length = 1;

    int count = 0;
    std::atomic<int> future_count{0};  // futureの実行回数カウンタ

    std::vector<std::future<void>> futures;  // 非同期タスクの管理
    const int max_threads = 12;

    // らせん型に探索
    while (true) {
        for (int i = 0; i < 2; ++i) {
            int dx = directions[direct_index].first;
            int dy = directions[direct_index].second;

            for (int j = 0; j < move_length; ++j) {
                if (board[ai_x][ai_y] == STONE_SPACE) {
                    // 非同期タスクの作成
                    futures.push_back(std::async(std::launch::async, [&, ai_x, ai_y] {
                        int temp_board[BOARD_SIZE][BOARD_SIZE];
                        std::copy(&board[0][0], &board[0][0] + BOARD_SIZE * BOARD_SIZE, &temp_board[0][0]);

                        temp_board[ai_x][ai_y] = com;
                        int eval_score = alpha_beta(temp_board, 0, -INF, INF, com);

                        // スコアの更新
                        {
                            std::lock_guard<std::mutex> lock(mtx);
                            if (eval_score > best_score) {
                                best_score = eval_score;
                                best_move = {ai_x, ai_y};
                            }
                        }

                        // futureの実行回数をインクリメント
                        int current_count = ++future_count;
                        
                        // 各future完了時にコメントを出力
                        std::cout << "Future " << current_count << "が完了しました。位置: (" << ai_x << ", " << ai_y 
                                  << "), スコア: " << eval_score << std::endl;
                    }));

                    // スレッド数の制限
                    if (futures.size() >= max_threads) {
                        futures.front().get();  // 最も古いタスクの終了を待つ
                        futures.erase(futures.begin());
                    }
                }

                // 次の手に備えた処理
                ++count;
                if (count >= board_max) goto END_LOOP;

                ai_x += dx;
                ai_y += dy;
            }

            // 方向転換
            direct_index = (direct_index + 1) % 4;
        }
        ++move_length;
    }

    END_LOOP:

    // 残りのタスクが完了するのを待機
    for (auto &f : futures) {
        f.get();
    }

    // 最終的なfutureの実行回数を出力
    std::cout << "全探索が終了しました。最終的な実行されたfutureの回数: " << future_count.load() << std::endl;

    return best_move;
}

// コマを置く処理
int calcPutPos(int board[][BOARD_SIZE], int com, int *pos_x, int *pos_y) {
    static bool start_flag = true;

    // コンピュータが初手の場合の処理
    // 先手は黒
    if (start_flag && com == STONE_BLACK) {
        *pos_x = BOARD_SIZE / 2;
        *pos_y = BOARD_SIZE / 2;
        start_flag = false;
        return 0;
    }

    std::pair<int, int> best_move = find_best_move(board, com);

    *pos_y = best_move.first;
    *pos_x = best_move.second;

    return 0;
}