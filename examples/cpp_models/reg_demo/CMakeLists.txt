cmake_minimum_required(VERSION 2.8.3)

find_package(Despot CONFIG REQUIRED)

add_executable("reg_demo"
  src/reg_demo.cpp
  src/main.cpp
)

target_link_libraries("reg_demo"
  despot
)