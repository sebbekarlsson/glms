cmake_minimum_required(VERSION 3.20)

project(glms_test)


file(GLOB PUBLIC_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file(GLOB glms_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/samples DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/.)


add_executable(glms_test ${glms_sources})

target_link_libraries(glms_test PUBLIC glms_static)
