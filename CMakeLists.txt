cmake_minimum_required(VERSION 3.20)

project(glms)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB sourcefiles ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

add_executable(glms_e ${sourcefiles})
list(FILTER sourcefiles EXCLUDE REGEX "main.c")
add_library(glms SHARED ${sourcefiles})
add_library(glms_static STATIC ${sourcefiles})

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


include(FetchContent)

FetchContent_Declare(
  memo_static
  URL /home/ianertson/workspace/memo
)
FetchContent_MakeAvailable(memo_static)

FetchContent_Declare(
  arena_static
  URL /home/ianertson/workspace/arena
)
FetchContent_MakeAvailable(arena_static)

FetchContent_Declare(
  text_static
  URL /home/ianertson/workspace/text
)
FetchContent_MakeAvailable(text_static)

FetchContent_Declare(
  hashy_static
  URL /home/ianertson/workspace/hashy
)
FetchContent_MakeAvailable(hashy_static)


FetchContent_Declare(
  mif_static
  URL /home/ianertson/workspace/mif
)
FetchContent_MakeAvailable(mif_static)

FetchContent_Declare(
  vec3_static
  URL /home/ianertson/workspace/vec3
)
FetchContent_MakeAvailable(vec3_static)

set(GLMS_DEPS m memo_static arena_static hashy_static text_static mif_static vec3_static)

target_link_libraries(glms ${GLMS_DEPS})
target_link_libraries(glms_e ${GLMS_DEPS})
target_link_libraries(glms_static ${GLMS_DEPS})


add_subdirectory(test)
