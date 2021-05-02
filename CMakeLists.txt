cmake_minimum_required(VERSION 3.0.0)
project(inverted_index VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)
enable_testing()


find_package(Boost 1.73.0 REQUIRED COMPONENTS system)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
endif()

include_directories(${PROJECT_SOURCE_DIR})

add_library(index_lib index/Index.cpp)

add_executable(client index_client/client.cpp)
add_executable(server index_server/server.cpp)
target_link_libraries(server index_lib)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
