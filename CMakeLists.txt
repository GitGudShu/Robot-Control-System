cmake_minimum_required(VERSION 3.29)
project(robot-control-system C)

set(CMAKE_C_STANDARD 11)

add_executable(coordinator src/coordinator.c)
add_executable(robot_maker src/robot_maker.c)
add_executable(robot_painter src/robot_painter.c)
add_executable(robot_transporter src/robot_transporter.c)
