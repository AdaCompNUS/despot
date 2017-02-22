cmake_minimum_required(VERSION 2.8.3)

find_package(Despot CONFIG REQUIRED)

include_directories(
  src
)

add_executable("laser_tag"
  src/base/base_tag.cpp
  src/laser_tag/laser_tag.cpp
  src/laser_tag/main.cpp
)

target_link_libraries("laser_tag"
  despot
)


add_executable("noisy_laser_tag"
  src/base/base_tag.cpp
  src/noisy_laser_tag/noisy_laser_tag.cpp
  src/noisy_laser_tag/main.cpp
)

target_link_libraries("noisy_laser_tag"
  despot
)


add_executable("tag"
  src/base/base_tag.cpp
  src/tag/tag.cpp
  src/tag/main.cpp
)
target_link_libraries("tag"
  despot
)
