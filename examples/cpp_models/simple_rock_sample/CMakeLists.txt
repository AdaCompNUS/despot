cmake_minimum_required(VERSION 2.8.3)

find_package(Despot CONFIG REQUIRED)

add_executable("simple_rock_sample"
  src/simple_rock_sample.cpp
  src/main.cpp
)

target_link_libraries("simple_rock_sample"
  despot
)
