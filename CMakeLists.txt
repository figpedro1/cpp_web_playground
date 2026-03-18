cmake_minimum_required(VERSION 3.18)
project(cpp_web_playground)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost CONFIG REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)

include_directories(include)
include_directories(include/external)
include_directories(${Boost_INCLUDE_DIRS})

add_definitions(-DCROW_USE_BOOST)
add_definitions(-DASIO_STANDALONE)

include(FetchContent)

FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG 1.14.2
)
FetchContent_MakeAvailable(cpr)

FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(json)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp" "apps/autodesk_viewer/src/*.cpp")

add_executable(cpp_web_playground
    ${SOURCES}
)

target_link_libraries(cpp_web_playground 
    PRIVATE
    Boost::system
    Threads::Threads
    cpr::cpr
    nlohmann_json::nlohmann_json
)