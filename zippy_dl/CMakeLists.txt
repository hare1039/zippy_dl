
project(zippy_dl)
FIND_PACKAGE(CURL REQUIRED)
if(CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIRS})
    include_directories(${CMAKE_SOURCE_DIR}/mujs)
    set(CMAKE_CXX_FLAGS "-O2 -std=c++14")
    add_executable(zippy_dl main.cpp mujs_bridge.cpp)

    link_directories(${CMAKE_BINARY_DIR}/mujs)
    target_link_libraries(zippy_dl mujs)
    target_link_libraries(zippy_dl ${CURL_LIBRARIES})
endif()
