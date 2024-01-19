cmake_minimum_required(VERSION 3.26)
project(ROBOSCUF)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_executable(ROBOSCUF
        constants.h
        enemy.cpp
        enemy.h
        entity.cpp
        entity.h
        environment.cpp
        environment.h
        input.cpp
        input.h
        inventory.cpp
        inventory.h
        item.cpp
        item.h
        level.cpp
        level.h
        logger.cpp
        logger.h
        menu.cpp
        menu.h
        player.cpp
        player.h
        renderer.cpp
        renderer.h
        rng.cpp
        rng.h
        roguelike.cpp
        roguelike.h
        spiralpath.cpp
        spiralpath.h)

target_link_libraries(ROBOSCUF ${CURSES_LIBRARIES})