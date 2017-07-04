cmake_minimum_required(VERSION 2.8.12)
project(hog)
set(PROJECT_VERSION "0.5.3")
include(FindPkgConfig)
pkg_check_modules(GROONGA REQUIRED "groonga")
include_directories(${GROONGA_INCLUDE_DIRS})
link_directories(${GROONGA_LIBRARY_DIRS})
file(GLOB SRC "src/*")
add_executable(hog ${SRC})
target_link_libraries(hog ${GROONGA_LIBRARIES})
set(CMAKE_C_FLAGS "--std=gnu99 ${CMAKE_C_FLAGS}")
find_package(Threads)
target_link_libraries(hog ${CMAKE_THREAD_LIBS_INIT})
install(TARGETS hog DESTINATION ./bin)

add_custom_target(run ./hog -V tmp/db/test DEPENDS hog)

configure_file(${CMAKE_SOURCE_DIR}/cmake/config.h.cmake
  ${CMAKE_SOURCE_DIR}/src/config.h)
