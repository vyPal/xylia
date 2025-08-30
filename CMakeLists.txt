cmake_minimum_required(VERSION 3.20)

project(
  xylia
  LANGUAGES C
  VERSION 0.0.1
)

add_compile_options(
  # -DDECOMPILE
  # -DDEBUG_STRESS_GC
  # -g
  # -rdynamic
  # -no-pie
  -O3
)

file(GLOB_RECURSE SOURCES "src/*.c")

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(${PROJECT_NAME} PRIVATE m)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
