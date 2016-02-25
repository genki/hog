project(hog)
cmake_minimum_required(VERSION 3.0)
add_executable(hog src/main.c)
target_link_libraries(hog groonga)
