cmake_minimum_required(VERSION 3.20)

project(jscript)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB sourcefiles ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

add_executable(jscript_e ${sourcefiles})
list(FILTER sourcefiles EXCLUDE REGEX "main.c")
add_library(jscript SHARED ${sourcefiles})
add_library(jscript_static STATIC ${sourcefiles})

target_include_directories(
  jscript
    PUBLIC
      $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      ${CMAKE_CURRENT_BINARY_DIR}/include
      ${CMAKE_CURRENT_SOURCE_DIR}/include)



target_include_directories(
  jscript_static
    PUBLIC
      $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      ${CMAKE_CURRENT_BINARY_DIR}/include
      ${CMAKE_CURRENT_SOURCE_DIR}/include)


include(FetchContent)

FetchContent_Declare(
  memo_static
  URL /home/ianertson/workspace/memo
)
FetchContent_MakeAvailable(memo_static)

FetchContent_Declare(
  hashy_static
  URL /home/ianertson/workspace/hashy
)
FetchContent_MakeAvailable(hashy_static)

set(JSCRIPT_DEPS m memo_static hashy_static)

target_link_libraries(jscript ${JSCRIPT_DEPS})
target_link_libraries(jscript_e ${JSCRIPT_DEPS})
target_link_libraries(jscript_static ${JSCRIPT_DEPS})
