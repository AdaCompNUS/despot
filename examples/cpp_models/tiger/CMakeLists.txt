cmake_minimum_required(VERSION 2.8.3)

add_executable("${PROJECT_NAME}_tiger"
  src/tiger.cpp
  src/main.cpp
)
target_link_libraries("${PROJECT_NAME}_tiger"
  "${PROJECT_NAME}"
)
install(TARGETS "${PROJECT_NAME}_tiger"
  RUNTIME DESTINATION "${BINARY_INSTALL_PATH}"
)