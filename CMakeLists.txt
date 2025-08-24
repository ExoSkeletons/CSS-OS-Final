cmake_minimum_required(VERSION 3.30.5)
project(Final)

set(CMAKE_CXX_STANDARD 20)


add_library(fd_polling SHARED fd_polling.cpp)
add_library(pthread_patterns SHARED pthread_patterns.cpp)

add_subdirectory(graph)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE graph pthread_patterns fd_polling)

add_executable(server server.cpp)
target_link_libraries(server PRIVATE graph pthread_patterns fd_polling)

add_executable(client client.cpp)
target_link_libraries(client PRIVATE pthread_patterns fd_polling)
