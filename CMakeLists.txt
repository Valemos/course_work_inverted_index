cmake_minimum_required(VERSION 3.9.0)
project(inverted_index VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include_directories(${PROJECT_SOURCE_DIR}/src)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_definitions(-DBOOST_ERROR_CODE_HEADER_ONLY -DCMAKE_BUILD_TYPE=Debug)
add_definitions(${Boost_LIB_DIAGNOSTIC_DEFINITIONS})

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.77 COMPONENTS thread serialization REQUIRED)
message("Boost ${Boost_VERSION} found with components ${Boost_LIBRARIES}")
set(BOOST_FULL_PATH_LIBRARIES /usr/lib/libboost_log-mt.a /usr/lib/libboost_log_setup-mt.a /usr/lib/libboost_thread-mt.a)

find_package(OpenSSL)
if (OPENSSL_FOUND)
    message("OpenSSL " ${OPENSSL_VERSION} " found")
endif()


set(PROJECT_BOOST_LIBRARIES Threads::Threads ${BOOST_FULL_PATH_LIBRARIES} ${Boost_LIBRARIES})

add_library(index_lib src/index/Index.cpp src/index/IndexBuilder.cpp)
target_link_libraries(index_lib ${PROJECT_BOOST_LIBRARIES})

add_library(encrypted_session
        src/session/DHKeyExchange.cpp
        src/session/DHKeyExchange.h
        src/session/AESEncryption.cpp
        src/session/AESEncryption.h
        src/session/EncryptedSession.h
        src/session/EncryptedSession.cpp
        src/session/Errors.cpp
        src/session/SHA256Algorithm.cpp
        src/session/SHA256Algorithm.h
        src/session/RSAKeyPair.cpp
        src/session/RSAKeyPair.h)
target_link_libraries(encrypted_session ~/openssl/libcrypto.so.3 ${PROJECT_BOOST_LIBRARIES})


add_executable(client src/index_client/main.cpp src/index_client/IndexSearchClient.cpp)
target_include_directories(client PRIVATE src/index_client)
target_link_libraries(client index_lib encrypted_session ${PROJECT_BOOST_LIBRARIES})


add_executable(server src/index_server/main.cpp src/index_server/SocketListener.cpp src/index_server/IndexSearchServer.cpp)
target_include_directories(server PRIVATE src/index_server)
target_link_libraries(server index_lib encrypted_session ${PROJECT_BOOST_LIBRARIES})

add_executable(create_index src/index/create_index.cpp)
target_link_libraries(create_index index_lib ${PROJECT_BOOST_LIBRARIES})

add_custom_target(client_server)
add_dependencies(client_server client server)


# testing
enable_testing()
find_package(GTest REQUIRED)
add_executable(tests test/test_main.cpp test/test_index.cpp test/performance.cpp test/test_session.cpp src/index_server/SocketListener.cpp)
target_link_libraries(tests GTest::gtest index_lib encrypted_session)
gtest_discover_tests(tests)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
