
cmake_minimum_required(VERSION 2.8.3)

add_executable("${PROJECT_NAME}_reg_demo"
  src/reg_demo.cpp
  src/main.cpp
)
target_link_libraries("${PROJECT_NAME}_reg_demo"
  "${PROJECT_NAME}"
)
install(TARGETS "${PROJECT_NAME}_reg_demo"
  RUNTIME DESTINATION "${BINARY_INSTALL_PATH}"
)