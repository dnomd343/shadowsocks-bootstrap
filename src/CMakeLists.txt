cmake_minimum_required(VERSION 2.8.12)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/utils)

add_subdirectory(utils)

add_executable(ss-bootstrap-local local.c common.c load.c sip003.c)
target_link_libraries(ss-bootstrap-local utils pthread)

add_executable(ss-bootstrap-server server.c common.c load.c sip003.c)
target_link_libraries(ss-bootstrap-server utils pthread)
