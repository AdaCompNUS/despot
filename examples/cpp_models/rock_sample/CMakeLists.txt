cmake_minimum_required(VERSION 2.8.3)

find_package(Despot CONFIG REQUIRED)

include_directories(
  src
)

add_executable("rock_sample"
  src/base/base_rock_sample.cpp
  src/rock_sample/rock_sample.cpp
  src/rock_sample/main.cpp
)
target_link_libraries("rock_sample"
  despot
)


add_executable("fvrs"
  src/base/base_rock_sample.cpp
  src/fvrs/fvrs.cpp
  src/fvrs/main.cpp
)

target_link_libraries("fvrs"
  despot
)