cmake_minimum_required(VERSION 3.9.0)
project(inverted_index VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)
enable_testing()

# testing
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)



find_package(Boost REQUIRED COMPONENTS system thread)
if(Boost_FOUND)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
endif()

add_compile_definitions(_WIN32_WINNT=0x0601)

add_library(index_lib index/Index.cpp)
include_directories(${PROJECT_SOURCE_DIR})

add_executable(client index_client/main.cpp index_client/IndexSearchClient.cpp)

add_executable(server index_server/main.cpp index_server/SocketListener.cpp index_server/IndexSearchServer.cpp)
target_include_directories(server PRIVATE index_server)
target_link_libraries(server index_lib)

add_executable(create_index index/create_index.cpp)
target_link_libraries(create_index index_lib)

# testing
include(GoogleTest)
add_executable(tests index/test_index.cpp)
target_link_libraries(tests gtest_main index_lib)
gtest_discover_tests(tests)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
