cmake_minimum_required(VERSION 3.18)
project(cpp_web_playground)
set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
find_package(OpenSSL REQUIRED)
find_package(Boost CONFIG REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)
find_package(PostgreSQL REQUIRED)
find_library(PQXX_LIB pqxx REQUIRED)

FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG        1.14.2
)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(cpr json googletest)

add_definitions(-DCROW_USE_BOOST)
add_definitions(-DASIO_STANDALONE)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS 
    "src/*.cpp" 
    "apps/autodesk_viewer/src/*.cpp"
)
list(REMOVE_ITEM SOURCES "src/main.cpp")

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS 
    "tests/*.cpp" 
    "apps/autodesk_viewer/tests/*.cpp"
)

add_library(app_core STATIC ${SOURCES})
target_link_libraries(app_core
    PUBLIC 
    Boost::system
    Threads::Threads
    cpr::cpr
    nlohmann_json::nlohmann_json
    ${PostgreSQL_LIBRARIES}
    pqxx
    OpenSSL::SSL
    OpenSSL::Crypto
)

add_executable(cpp_web_playground src/main.cpp)
target_link_libraries(cpp_web_playground PRIVATE app_core)

enable_testing()

add_executable(unit_tests ${TEST_SOURCES})
target_link_libraries(unit_tests PRIVATE GTest::gtest_main app_core)

add_test(NAME AllTests COMMAND unit_tests)

target_include_directories(app_core
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OPENSSL_INCLUDE_DIR}
    include
    include/external
    apps/autodesk_viewer/include
)