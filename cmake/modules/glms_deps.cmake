macro (glms_get_dep DEP_NAME DEP_URL)
  set (extra_args ${ARGN})
  list(LENGTH extra_args extra_count)
  if (NOT TARGET ${DEP_NAME})
    FetchContent_Declare(
      ${DEP_NAME}	
      GIT_REPOSITORY ${DEP_URL} 
    )
    FetchContent_MakeAvailable(${DEP_NAME})

    if (${extra_count} GREATER 0)
      list(GET extra_args 0 optional_arg)
      target_compile_options(${DEP_NAME} PUBLIC ${optional_arg})
    endif ()
  endif()
endmacro()

function (glms_deps TARGET_NAME)
  include(FetchContent)


  glms_get_dep(memo_static https://github.com/sebbekarlsson/memo.git)
  glms_get_dep(arena_static https://github.com/sebbekarlsson/arena.git)
  glms_get_dep(text_static https://github.com/sebbekarlsson/text.git)
  glms_get_dep(hashy_static https://github.com/sebbekarlsson/hashy.git)
  glms_get_dep(mif_static https://github.com/sebbekarlsson/mif.git)
  glms_get_dep(vec3_static https://github.com/sebbekarlsson/vec3.git)
  glms_get_dep(fjson_static https://github.com/sebbekarlsson/fastjson.git)
  glms_get_dep(spath_static https://github.com/sebbekarlsson/spath.git -fPIC)
  glms_get_dep(gimg_static https://github.com/sebbekarlsson/gimg.git -fPIC)



  if (NOT TARGET cglm)
    FetchContent_Declare(
      cglm
      GIT_REPOSITORY https://github.com/recp/cglm.git
      GIT_TAG "v0.8.4"
    )
    FetchContent_MakeAvailable(cglm)
  endif()


  if (NOT TARGET curl)
    
   find_library(CURL_LIB curl)

   if (NOT CURL_LIB)
     FetchContent_Declare(
       curl
       GIT_REPOSITORY https://github.com/curl/curl.git
       GIT_TAG "curl-7_87_0"
     )
    FetchContent_MakeAvailable(curl)
   endif()
  endif()



  set(GLMS_DEPS m gimg_static memo_static arena_static hashy_static text_static mif_static vec3_static spath_static fjson_static cglm curl dl)

  target_link_libraries(${TARGET_NAME} ${GLMS_DEPS})
endfunction()
