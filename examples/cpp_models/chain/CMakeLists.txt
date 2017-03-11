cmake_minimum_required(VERSION 2.8.3)

add_executable("${PROJECT_NAME}_chain"
  src/chain.cpp
  src/main.cpp
)
target_link_libraries("${PROJECT_NAME}_chain"
  "${PROJECT_NAME}"
)
install(TARGETS "${PROJECT_NAME}_chain"
  RUNTIME DESTINATION "${BINARY_INSTALL_PATH}"
)