cmake_minimum_required(VERSION 2.8.3)

find_package(Despot CONFIG REQUIRED)

add_executable("navigation"
  src/navigation.cpp
  src/main.cpp
)

target_link_libraries("navigation"
  despot
)
