PROGRAM = gomoku

CC = g++
CFLAGS = -Wall -O3 -lncursesw -I$(INCLUDE_DIR)

SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin

# ソースファイルからオブジェクトファイルのリストを生成
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

TARGET = $(BIN_DIR)/$(PROGRAM)
# すべてのビルドターゲット
all: $(TARGET)

# 実行ファイルの生成
$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

# オブジェクトファイルの生成ルール (buildフォルダに出力)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# binディレクトリが存在しない場合は作成
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# buildディレクトリが存在しない場合は作成
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# 実行ターゲット
.PHONY: run
run: $(BIN_DIR)/$(PROGRAM)
	$(BIN_DIR)/$(PROGRAM)

# クリーンアップ
.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJS)