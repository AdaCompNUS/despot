cmake_minimum_required(VERSION 2.8.3)

include_directories(
  src
)

add_executable("${PROJECT_NAME}_rock_sample"
  src/base/base_rock_sample.cpp
  src/rock_sample/rock_sample.cpp
  src/rock_sample/main.cpp
)
target_link_libraries("${PROJECT_NAME}_rock_sample"
  "${PROJECT_NAME}"
)

add_executable("${PROJECT_NAME}_fvrs"
  src/base/base_rock_sample.cpp
  src/fvrs/fvrs.cpp
  src/fvrs/main.cpp
)
target_link_libraries("${PROJECT_NAME}_fvrs"
  "${PROJECT_NAME}"
)

install(TARGETS "${PROJECT_NAME}_rock_sample" "${PROJECT_NAME}_fvrs"
  RUNTIME DESTINATION "${BINARY_INSTALL_PATH}"
)