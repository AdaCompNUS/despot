cmake_minimum_required(VERSION 2.8.3)

find_package(Despot CONFIG REQUIRED)

add_executable("pomdpx"
  src/main.cpp
)

target_link_libraries("pomdpx"
  despot
)
