cmake_minimum_required (VERSION 3.5.1)
project (locality)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

find_package(Threads REQUIRED)
find_package(Hwloc REQUIRED)

if (NOT Hwloc_FOUND) 
  message(FATAL "Cannot find HW LOC library")
endif()

find_program(
  CLANG_TIDY_EXE
  NAMES "clang-tidy"
  DOC "Path to the clang-tidy executable"
  )

if(NOT CLANG_TIDY_EXE)
  message(STATUS "clang-tidy not found")
else()
  message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
  set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}" "-checks=*,-clang-analyzer-alpha.*,-google-runtime-int")
endif()


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

include_directories(include)
include_directories(${Hwloc_INCLUDE_DIRS})

add_executable (example example.c)
target_link_libraries(example ${Hwloc_LIBRARIES})
add_test(example example)

add_executable (locality locality.cpp)
target_link_libraries(locality ${Hwloc_LIBRARIES} ${THREADS})
set_property(TARGET locality PROPERTY CXX_STANDARD 14)
set_property(TARGET locality PROPERTY CXX_STANDARD_REQUIRED ON)

add_executable (executor_context executor_context.cpp)
target_link_libraries(executor_context ${Hwloc_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
set_property(TARGET executor_context PROPERTY CXX_STANDARD 17)
set_property(TARGET executor_context PROPERTY CXX_STANDARD_REQUIRED ON)
add_test(executor_context executor_context)


if (CLANG_TIDY_EXE)
  set_property(TARGET locality PROPERTY CXX_CLANG_TIDY "${DO_CLANG_TIDY}")
endif()

add_test(locality locality)



