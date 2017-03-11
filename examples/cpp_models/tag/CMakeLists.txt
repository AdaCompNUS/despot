
cmake_minimum_required(VERSION 2.8.3)

include_directories(
  src
)

add_executable("${PROJECT_NAME}_laser_tag"
  src/base/base_tag.cpp
  src/laser_tag/laser_tag.cpp
  src/laser_tag/main.cpp
)
target_link_libraries("${PROJECT_NAME}_laser_tag"
  "${PROJECT_NAME}"
)

add_executable("${PROJECT_NAME}_noisy_laser_tag"
  src/base/base_tag.cpp
  src/noisy_laser_tag/noisy_laser_tag.cpp
  src/noisy_laser_tag/main.cpp
)
target_link_libraries("${PROJECT_NAME}_noisy_laser_tag"
  "${PROJECT_NAME}"
)

add_executable("${PROJECT_NAME}_tag"
  src/base/base_tag.cpp
  src/tag/tag.cpp
  src/tag/main.cpp
)
target_link_libraries("${PROJECT_NAME}_tag"
  "${PROJECT_NAME}"
)

install(TARGETS
    "${PROJECT_NAME}_laser_tag"
    "${PROJECT_NAME}_noisy_laser_tag"
    "${PROJECT_NAME}_tag"
  RUNTIME DESTINATION "${BINARY_INSTALL_PATH}"
)