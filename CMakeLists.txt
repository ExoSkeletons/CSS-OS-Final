cmake_minimum_required(VERSION 3.30.5)
project(Final)

set(CMAKE_CXX_STANDARD 20)

add_executable(main main.cpp
        PthTools.hpp
        fd_polling.hpp
        fd_polling.cpp
        pthread_patterns.cpp
        pthread_patterns.hpp
)

add_executable(server server.cpp
        fd_polling.cpp
        fd_polling.hpp
        pthread_patterns.cpp
        pthread_patterns.hpp
)
add_executable(client client.cpp
        fd_polling.cpp
        fd_polling.hpp
        pthread_patterns.cpp
        pthread_patterns.hpp
)
