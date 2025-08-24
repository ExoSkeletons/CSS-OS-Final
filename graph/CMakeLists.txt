project(Final)

file(GLOB GRAPH_SOURCES *.cpp)

add_library(graph SHARED ${GRAPH_SOURCES})

target_include_directories(graph PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
