project(hog)
cmake_minimum_required(VERSION 3.0)
file(GLOB SRC "src/*")
add_executable(hog ${SRC})
target_link_libraries(hog groonga)

add_custom_target(run ./hog tmp/db/test DEPENDS hog)
