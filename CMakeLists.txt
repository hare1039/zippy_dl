cmake_minimum_required(VERSION 2.6)

add_subdirectory(mujs)
add_subdirectory(zippy_dl)
add_dependencies(zippy_dl mujs)

