function (glms_deps TARGET_NAME)
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


find_library(GLFW_LIB glfw)


if (NOT GLFW_LIB)
    if (NOT TARGET glfw)
	FetchContent_Declare(
	glfw
	GIT_REPOSITORY https://github.com/glfw/glfw
	GIT_TAG "3.3.8"
	)
	FetchContent_MakeAvailable(glfw)
    endif()
endif()

set(GLMS_DEPS m gimg_static memo_static arena_static hashy_static text_static mif_static vec3_static spath_static cglm GL glfw GLEW dl)

target_link_libraries(${TARGET_NAME} ${GLMS_DEPS})
endfunction()
