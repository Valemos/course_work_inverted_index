cmake_minimum_required(VERSION 3.0.0)
project(inverted_index VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)
enable_testing()


find_package(Boost 1.73.0 REQUIRED COMPONENTS system thread)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
endif()

add_compile_definitions(_WIN32_WINNT=0x0601)

add_library(index_lib index/Index.cpp)
include_directories(${PROJECT_SOURCE_DIR})

add_executable(client index_client/client.cpp)

add_executable(server index_server/server.cpp index_server/SocketListener.cpp)
target_include_directories(server PRIVATE index_server)
target_link_libraries(server index_lib)

add_executable(create_index index/create_index.cpp)
target_link_libraries(create_index index_lib)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
