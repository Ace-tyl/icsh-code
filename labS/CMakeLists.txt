cmake_minimum_required(VERSION 3.5)

project(lc3simulator)

set (CMAKE_CXX_STANDARD 14)

set(BOOST_ROOT C:/w64devkit/boost_1_84_0)
set(BOOST_INCLUDEDIR ${BOOST_ROOT})
set(BOOST_LIBRARYDIR ${BOOST_ROOT}/stage/lib)
set(Boost_DEBUG ON)
set(Boost_LIB_PREFIX lib)
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.84.0 COMPONENTS program_options REQUIRED)

set(SOURCES
    src/main.cpp
    src/memory.cpp
    src/register.cpp
    src/simulator.cpp)

add_executable(lc3simulator ${SOURCES})

target_include_directories(lc3simulator
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(lc3simulator ${Boost_LIBRARIES})
endif()
