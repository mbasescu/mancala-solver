cmake_minimum_required(VERSION 3.0)

project(MancalaSolver VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SOURCES
    src/game_mechanics.cpp
)

add_executable(mancala-solver main.cpp ${SOURCES})
target_include_directories(mancala-solver PRIVATE ${PROJECT_SOURCE_DIR}/include/mancala-solver)