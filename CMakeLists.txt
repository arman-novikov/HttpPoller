cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME health_poller)
project(${PROJECT_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Boost COMPONENTS program_options REQUIRED)
find_package(Threads REQUIRED)
include_directories( ${Boost_INCLUDE_DIR} )

add_library(${PROJECT_NAME}-lib
    requestcontext.cpp
    requestcontext.hpp
    session.cpp
    session.hpp
    httppoller.cpp
    httppoller.hpp
)
target_link_libraries(${PROJECT_NAME}-lib
    ${Boost_LIBRARIES}
    Threads::Threads
)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME}
    ${PROJECT_NAME}-lib
)
