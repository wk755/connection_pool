cmake_minimum_required(VERSION 3.28)
project(connection_pool)

set(CMAKE_CXX_STANDARD 17)
include_directories("C:/Program Files/MySQL/MySQL Server 5.7/include")
link_directories("C:/Program Files/MySQL/MySQL Server 5.7/lib")
add_executable(connection_pool main.cpp
        connect_pool.cpp
        connect_pool.h
        connection.cpp
        connection.h
        public.h
)
target_link_libraries(connection_pool mysql)