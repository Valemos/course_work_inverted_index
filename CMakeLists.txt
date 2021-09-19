cmake_minimum_required(VERSION 3.9.0)
project(inverted_index VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include_directories(${PROJECT_SOURCE_DIR})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_definitions(-DBOOST_ERROR_CODE_HEADER_ONLY)
add_definitions(${Boost_LIB_DIAGNOSTIC_DEFINITIONS})

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.77 COMPONENTS thread serialization REQUIRED)
message("Boost ${Boost_VERSION} found with components ${Boost_LIBRARIES}")
set(BOOST_FULL_PATH_LIBRARIES /usr/lib/libboost_log-mt.a /usr/lib/libboost_log_setup-mt.a /usr/lib/libboost_thread-mt.a)

find_package(OpenSSL)
if (OPENSSL_FOUND)
    message("OpenSSL " ${OPENSSL_VERSION} " found")
endif()

add_library(index_lib index/Index.cpp index/IndexBuilder.cpp)
target_link_libraries(index_lib ${BOOST_LOG_LIBRARIES})

add_library(encrypted_session session/ClientSession.cpp session/ClientSession.h session/ServerSession.cpp session/ServerSession.h session/AESEncryption.cpp session/AESEncryption.h)
target_link_libraries(encrypted_session OpenSSL::Crypto)

set(PROJECT_LIBRARIES index_lib Threads::Threads ${BOOST_LOG_LIBRARIES} ${Boost_LIBRARIES})

add_executable(client index_client/main.cpp index_client/IndexSearchClient.cpp)
target_include_directories(client PRIVATE index_client)
target_link_libraries(client ${PROJECT_LIBRARIES})


add_executable(server index_server/main.cpp index_server/SocketListener.cpp index_server/IndexSearchServer.cpp)
target_include_directories(server PRIVATE index_server)
target_link_libraries(server ${PROJECT_LIBRARIES})

add_executable(create_index index/create_index.cpp)
target_link_libraries(create_index ${PROJECT_LIBRARIES})

add_custom_target(client_server)
add_dependencies(client_server client server)


# testing
include(GoogleTest)
add_executable(tests test/test_index.cpp test/performance.cpp)
target_link_libraries(tests gtest_main index_lib)
gtest_discover_tests(tests)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
