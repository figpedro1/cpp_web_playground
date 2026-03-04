cmake_minimum_required(VERSION 3.10)
project(cpp_web_playground)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)

include_directories(include)
include_directories(include/external)
include_directories(${Boost_INCLUDE_DIRS})

add_definitions(-DCROW_USE_BOOST)
add_definitions(-DASIO_STANDALONE)

add_executable(cpp_web_playground
    src/main.cpp
)

target_link_libraries(cpp_web_playground Boost::system Threads::Threads)