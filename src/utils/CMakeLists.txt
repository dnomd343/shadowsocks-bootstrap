cmake_minimum_required(VERSION 2.8.12)

aux_source_directory(${PROJECT_SOURCE_DIR}/src/utils utils_src)
add_library(utils ${utils_src})
