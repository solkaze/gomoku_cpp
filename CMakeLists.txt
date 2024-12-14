cmake_minimum_required(VERSION 3.14)

project(Gomoku_ai)

message("project name is automaically set as: ${PROJECT_NAME}")

set(PROJECT_VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SRC
  src/.libgomoku.cpp
  src/alpha_beta.cpp
  src/bit_board.cpp
  src/csv_data.cpp
  src/evaluate.cpp
  src/gomoku_ai.cpp
  src/gomoku.cpp
  src/operater.cpp
  src/prohibit.cpp
  src/zobrist_hash.cpp
)

set(HEADERS
  include/alpha_beta.hpp
  include/bit_board.hpp
  include/common.hpp
  include/csv_data.hpp
  include/evaluate.hpp
  include/gomoku_ai.hpp
  include/gomoku.hpp
  include/operater.hpp
  include/prohibit.hpp
  include/zobrist_hash.hpp
)

add_executable(gomoku ${SRC} ${HEADERS})
target_include_directories(gomoku PUBLIC include)

target_compile_options(gomoku PRIVATE -Wall -Wextra -g3 -O3)

# インクルードディレクトリを指定
target_include_directories(gomoku PRIVATE ${Curses_INCLUDE_DIRS} include)

# データフォルダのパス
set(DATA_DIR ${CMAKE_SOURCE_DIR}/data)

add_custom_command(
    TARGET gomoku POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:gomoku>/data
)
