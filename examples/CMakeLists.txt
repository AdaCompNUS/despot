# Build all examples at once
cmake_minimum_required(VERSION 2.8.3)

add_subdirectory(cpp_models/adventurer)
add_subdirectory(cpp_models/bridge)
add_subdirectory(cpp_models/chain)
add_subdirectory(cpp_models/navigation)
add_subdirectory(cpp_models/pocman)
add_subdirectory(cpp_models/reg_demo)
add_subdirectory(cpp_models/rock_sample)
add_subdirectory(cpp_models/simple_rock_sample)
add_subdirectory(cpp_models/tag)
add_subdirectory(cpp_models/tiger)

add_subdirectory(pomdpx_models)
