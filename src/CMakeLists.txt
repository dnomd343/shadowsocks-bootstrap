cmake_minimum_required(VERSION 2.8.12)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(/usr/lib/i386-linux-gnu/glib-2.0/include)
include_directories(/usr/lib/x86_64-linux-gnu/glib-2.0/include)
include_directories(/usr/lib/aarch64-linux-gnu/glib-2.0/include)
include_directories(/usr/lib64/glib-2.0/include)
include_directories(/usr/lib/glib-2.0/include)
include_directories(/usr/include/glib-2.0)

aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC)
list(REMOVE_ITEM SRC ${PROJECT_SOURCE_DIR}/src/local.c)
list(REMOVE_ITEM SRC ${PROJECT_SOURCE_DIR}/src/server.c)

add_executable(ss-bootstrap-local local.c ${SRC})
target_link_libraries(ss-bootstrap-local glib-2.0 pthread)

add_executable(ss-bootstrap-server server.c ${SRC})
target_link_libraries(ss-bootstrap-server glib-2.0 pthread)
