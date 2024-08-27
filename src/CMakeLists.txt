cmake_minimum_required(VERSION 3.11)

#cmake will warn about CMP0167 not being set in cmake >= 3.30.
#Setting this as new keeps the cmake up to the latest policy standard.
if(CMAKE_VERSION GREATER_EQUAL 3.30)
    cmake_policy(SET CMP0167 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

project(restgresql LANGUAGES CXX C)

include(FetchContent GNUInstallDirs)
include(CMakePrintHelpers)

find_package(Threads)
find_package(PkgConfig)
find_package(Boost 1.83 REQUIRED COMPONENTS filesystem iostreams program_options)
find_package(OpenSSL)

include_directories(${Boost_INCLUDE_DIRS})

FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_Declare(
    mongoose_git
    GIT_REPOSITORY https://github.com/cesanta/mongoose.git
    GIT_TAG 97296934ab594241ba2489f018a21779882a7936 #7.15
)

FetchContent_MakeAvailable(json mongoose_git)

pkg_check_modules(PQXX REQUIRED libpqxx)

file(COPY ${mongoose_git_SOURCE_DIR}/mongoose.h DESTINATION ${CMAKE_SOURCE_DIR}/mongoose)
file(COPY ${mongoose_git_SOURCE_DIR}/mongoose.c DESTINATION ${CMAKE_SOURCE_DIR}/mongoose)
add_library(mongoose STATIC ${CMAKE_SOURCE_DIR}/mongoose/mongoose.h ${CMAKE_SOURCE_DIR}/mongoose/mongoose.c)
set_target_properties(mongoose PROPERTIES COMPILE_FLAGS "-DMG_TLS=MG_TLS_OPENSSL")
target_link_libraries(mongoose OpenSSL::SSL OpenSSL::Crypto)

add_executable(
    restgresql
    main.cpp
    postgresql.h
    postgresql.cpp
    restapi.h
    restapi.cpp
)

target_link_libraries(restgresql PRIVATE nlohmann_json::nlohmann_json mongoose ${PQXX_LINK_LIBRARIES} ${Boost_LIBRARIES})

install(
    TARGETS restgresql
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
