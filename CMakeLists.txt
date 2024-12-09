cmake_minimum_required(VERSION 3.14)

project(Gomoku_ai)

message("project name is automaically set as: ${PROJECT_NAME}")

set(PROJECT_VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SRC
  src/gomoku_ai.cpp
  src/alpha_beta.cpp
  src/zobrist_hash.cpp
  src/evaluate.cpp
  src/bit_board.cpp
  src/gomoku.cpp
  src/.libgomoku.cpp
  src/prohibited.cpp
  src/operater.cpp
)

set(HEADERS
  include/gomoku.hpp
  include/gomoku_ai.hpp
  include/evaluate.hpp
  include/bit_board.hpp
  include/alpha_beta.hpp
  include/common.hpp
  include/prohibited.hpp
  include/operater.hpp
)

add_executable(gomoku ${SRC} ${HEADERS})
target_include_directories(gomoku PUBLIC include)

add_definitions(-DDATA_PATH="${CMAKE_SOURCE_DIR}/data/")

target_compile_options(gomoku PRIVATE -Wall -Wextra -g3 -O3)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=core-avx2")

# インクルードディレクトリを指定
target_include_directories(gomoku PRIVATE ${Curses_INCLUDE_DIRS} include)