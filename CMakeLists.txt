cmake_minimum_required(VERSION 3.25)

project(restgresql LANGUAGES CXX C)

#cmake will warn about CMP0167 not being set in cmake >= 3.30.
#Setting this as new keeps the cmake up to the latest policy standard.
if(CMAKE_VERSION GREATER_EQUAL 3.30)
    cmake_policy(SET CMP0167 NEW)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(BOOST_LIBS Boost::filesystem Boost::iostreams Boost::program_options)

include(FetchContent)
include(GNUInstallDirs)

find_package(Threads)
find_package(PkgConfig)
find_package(OpenSSL)

FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_Declare(
    libpqxx
    URL https://github.com/jtv/libpqxx/archive/refs/tags/8.0.1.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_Declare(
    mongoose_git
    GIT_REPOSITORY https://github.com/cesanta/mongoose.git
    GIT_TAG b1c2ffe1a0aa13e3d94075b1a2c66b8b43ac9116 #7.21
)

FetchContent_Declare(
    Boost
    URL https://github.com/boostorg/boost/releases/download/boost-1.91.0-1/boost-1.91.0-1-cmake.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(boost json libpqxx mongoose_git)

#Copy mongoose into the binary dir so the source tree stays clean.
file(COPY ${mongoose_git_SOURCE_DIR}/mongoose.h DESTINATION ${CMAKE_BINARY_DIR}/mongoose)
file(COPY ${mongoose_git_SOURCE_DIR}/mongoose.c DESTINATION ${CMAKE_BINARY_DIR}/mongoose)
add_library(mongoose STATIC ${CMAKE_BINARY_DIR}/mongoose/mongoose.h ${CMAKE_BINARY_DIR}/mongoose/mongoose.c)
set_target_properties(mongoose PROPERTIES COMPILE_FLAGS "-DMG_TLS=MG_TLS_OPENSSL")
target_include_directories(mongoose PUBLIC ${CMAKE_BINARY_DIR})
target_link_libraries(mongoose OpenSSL::SSL OpenSSL::Crypto)

add_executable(
    restgresql
    src/main.cpp
    src/api/restapi.h
    src/api/restapi.cpp
    src/api/router.h
    src/api/router.cpp
    src/db/postgresql.h
    src/db/postgresql.cpp
    src/utils/utils.h
    src/utils/utils.cpp
)

target_include_directories(restgresql PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(restgresql PRIVATE nlohmann_json::nlohmann_json mongoose pqxx "${BOOST_LIBS}")

enable_testing()

add_executable(
    unit_tests
    tests/unit/unit_tests.cpp
    src/api/router.cpp
    src/utils/utils.cpp
)
target_include_directories(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(unit_tests PRIVATE nlohmann_json::nlohmann_json "${BOOST_LIBS}")
add_test(NAME unit_tests COMMAND unit_tests)

install(
    TARGETS restgresql
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
