PROGRAM = gomoku

CC = g++
CFLAGS = -Wall -O3 -lncursesw

BIN_DIR = bin
SRC_DIR = src

OBJS = $(BIN_DIR)/gomoku_ai.o

# すべてのビルドターゲット
all: $(BIN_DIR)/$(PROGRAM)

# 実行ファイルの生成
$(BIN_DIR)/$(PROGRAM): $(OBJS) | $(BIN_DIR)
	$(CC) $(OBJS) $(CFLAGS) -o $@

# オブジェクトファイルの生成ルール
$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# binディレクトリが存在しない場合は作成
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# クリーンアップ
.PHONY: clean
clean:
	$(RM) $(BIN_DIR)/$(PROGRAM) $(OBJS)