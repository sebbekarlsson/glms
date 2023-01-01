cmake_minimum_required(VERSION 3.20)

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


include(FetchContent)

FetchContent_Declare(
  memo_static
#  URL /home/ianertson/workspace/memo
  GIT_REPOSITORY https://github.com/sebbekarlsson/memo.git 
)
FetchContent_MakeAvailable(memo_static)

FetchContent_Declare(
  arena_static
# URL /home/ianertson/workspace/arena
  GIT_REPOSITORY https://github.com/sebbekarlsson/arena.git 
)
FetchContent_MakeAvailable(arena_static)

FetchContent_Declare(
  text_static
#  URL /home/ianertson/workspace/text
  GIT_REPOSITORY https://github.com/sebbekarlsson/text.git 
)
FetchContent_MakeAvailable(text_static)

FetchContent_Declare(
  hashy_static
#  URL /home/ianertson/workspace/hashy
  GIT_REPOSITORY https://github.com/sebbekarlsson/hashy.git 
)
FetchContent_MakeAvailable(hashy_static)


FetchContent_Declare(
  mif_static
#  URL /home/ianertson/workspace/mif
  GIT_REPOSITORY https://github.com/sebbekarlsson/mif.git 
)
FetchContent_MakeAvailable(mif_static)

FetchContent_Declare(
  vec3_static
#  URL /home/ianertson/workspace/vec3
  GIT_REPOSITORY https://github.com/sebbekarlsson/vec3.git 
)
FetchContent_MakeAvailable(vec3_static)

FetchContent_Declare(
  spath_static
#  URL /home/ianertson/workspace/spath
  GIT_REPOSITORY https://github.com/sebbekarlsson/spath.git 
)
FetchContent_MakeAvailable(spath_static)
target_compile_options(spath_static PUBLIC -fPIC)

FetchContent_Declare(
  gimg_static
#  URL /home/ianertson/workspace/gimg
  GIT_REPOSITORY https://github.com/sebbekarlsson/gimg.git 
)
FetchContent_MakeAvailable(gimg_static)
target_compile_options(gimg_static PUBLIC -fPIC)


if (NOT TARGET cglm)
FetchContent_Declare(
  cglm
  GIT_REPOSITORY https://github.com/recp/cglm.git
  GIT_TAG "v0.8.4"
  )
FetchContent_MakeAvailable(cglm)
endif()

set(GLMS_DEPS m gimg_static memo_static arena_static hashy_static text_static mif_static vec3_static spath_static cglm dl)

target_link_libraries(glms ${GLMS_DEPS})
target_link_libraries(glms_e ${GLMS_DEPS})
target_link_libraries(glms_static ${GLMS_DEPS})


add_subdirectory(test)
