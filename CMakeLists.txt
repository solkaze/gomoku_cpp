cmake_minimum_required(VERSION 3.14)

project(Gomoku_ai)

message("project name is automaically set as: ${PROJECT_NAME}")

set(PROJECT_VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Curses REQUIRED)

if (Curses_FOUND)
    message(STATUS "Curses found")
else(Curses_FOUND)
    message(WARNING "Curses not found")
endif()

set(SRC
  src/gomoku_ai.cpp
  src/gomoku.cpp
)

set(HEADERS
  include/gomoku.hpp
)

add_executable(gomoku ${SRC} ${HEADERS})
target_include_directories(gomoku PUBLIC include)

# ncursesライブラリをリンク
target_link_libraries(gomoku PRIVATE ${Curses_LIBRARIES})

target_compile_options(gomoku PRIVATE -Wall -Wextra -g3 -O2)

# インクルードディレクトリを指定
target_include_directories(gomoku PRIVATE ${Curses_INCLUDE_DIRS} include)