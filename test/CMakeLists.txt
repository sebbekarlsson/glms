cmake_minimum_required(VERSION 3.20)

project(jscript_test)


file(GLOB PUBLIC_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file(GLOB jscript_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/samples DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/.)


add_executable(jscript_test ${jscript_sources})

target_link_libraries(jscript_test PUBLIC jscript_static)
