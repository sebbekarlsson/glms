cmake_minimum_required(VERSION 3.20)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

include(glms_deps)

project(glms)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(
    GLOB
    sourcefiles
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/*.c
)

#file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(glms_e ${sourcefiles})
list(FILTER sourcefiles EXCLUDE REGEX "main.c")
add_library(glms SHARED ${sourcefiles})
add_library(glms_static STATIC ${sourcefiles})


# set (GLMS_DEBUG_FLAGS -g -Wall)
set (GLMS_DEBUG_FLAGS )

target_compile_options(glms_e PRIVATE ${GLMS_DEBUG_FLAGS})
target_compile_options(glms PRIVATE ${GLMS_DEBUG_FLAGS})
target_compile_options(glms_static PRIVATE ${GLMS_DEBUG_FLAGS})


target_include_directories(
  glms
    PUBLIC
      $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      ${CMAKE_CURRENT_BINARY_DIR}/include
      ${CMAKE_CURRENT_SOURCE_DIR}/include)



target_include_directories(
  glms_static
    PUBLIC
      $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      ${CMAKE_CURRENT_BINARY_DIR}/include
      ${CMAKE_CURRENT_SOURCE_DIR}/include)


glms_deps(glms)
glms_deps(glms_e)
glms_deps(glms_static)


add_subdirectory(test)
