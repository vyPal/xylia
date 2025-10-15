cmake_minimum_required(VERSION 3.20)

project(
  xylia
  LANGUAGES C CXX
  VERSION 0.0.1
)

option(DECOMPILE "Enable decompile mode" OFF)
if(DECOMPILE)
  message(STATUS "Decompile mode enabled")
  add_compile_definitions(DECOMPILE)
endif()

add_subdirectory(replxx)
file(GLOB_RECURSE SOURCES "src/*.c")

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(${PROJECT_NAME} PRIVATE m ffi replxx)

target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CONFIG:Debug>:-Wall -Wextra -Wpedantic -g -rdynamic -no-pie -DDEBUG>
  $<$<CONFIG:Release>:-O3>
)

target_link_options(${PROJECT_NAME} PRIVATE
  $<$<CONFIG:Debug>:-no-pie>
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
  XYLIA_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
  XYLIA_VERSION_MINOR=${PROJECT_VERSION_MINOR}
  XYLIA_VERSION_PATCH=${PROJECT_VERSION_PATCH}
  XYLIA_VERSION="${PROJECT_VERSION}"
)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
