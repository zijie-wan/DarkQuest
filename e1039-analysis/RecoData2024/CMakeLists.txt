cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(dataEval CXX C)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(CMAKE_MACOSX_RPATH 1)

# ROOT
find_program(ROOTCONF "root-config")
if(ROOTCONF)
  message("-- Detecting ROOT:    found at ${ROOTCONF}")
else()
  message(FATAL_ERROR "-- Detecting ROOT:    not found")
endif()
execute_process(COMMAND root-config --prefix OUTPUT_VARIABLE ROOT_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND root-config --cflags OUTPUT_VARIABLE ROOT_CFLAGS  OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND root-config --libs   OUTPUT_VARIABLE ROOT_LINK    OUTPUT_STRIP_TRAILING_WHITESPACE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ROOT_CFLAGS}")

# ROOT dict generation
# ROOT dict generation (ROOT 6 / rootcling)
add_custom_command(
  OUTPUT  ${PROJECT_SOURCE_DIR}/src/SRawEventDict.cxx
          ${PROJECT_SOURCE_DIR}/src/SRawEventDict_rdict.pcm
  COMMAND rootcling
  ARGS    -f ${PROJECT_SOURCE_DIR}/src/SRawEventDict.cxx
          -I${PROJECT_SOURCE_DIR}/inc
          ${PROJECT_SOURCE_DIR}/inc/SRawEvent.h
          ${PROJECT_SOURCE_DIR}/inc/SRawEventLinkDef.h
  DEPENDS ${PROJECT_SOURCE_DIR}/inc/SRawEvent.h
          ${PROJECT_SOURCE_DIR}/inc/SRawEventLinkDef.h
)

# Install the .pcm file alongside the library (ROOT 6 requires it at runtime)
install(FILES ${PROJECT_SOURCE_DIR}/src/SRawEventDict_rdict.pcm DESTINATION lib)

include_directories(${PROJECT_SOURCE_DIR}/inc)

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cxx ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/inc/*.h   ${PROJECT_SOURCE_DIR}/inc/*.hh)

# Shared library
add_library(dataEval SHARED
  ${sources}
  ${headers}
  ${PROJECT_SOURCE_DIR}/src/SRawEventDict.cxx
)
target_link_libraries(dataEval ${ROOT_LINK})

# Install library and headers
install(TARGETS dataEval DESTINATION lib)
install(FILES   ${headers} DESTINATION include)

# Executables
file(GLOB execs ${PROJECT_SOURCE_DIR}/exe/*.cxx)
foreach(exec ${execs})
  string(REPLACE "${PROJECT_SOURCE_DIR}/exe/" "" exec ${exec})
  string(REPLACE ".cxx" "" exec ${exec})
  message(STATUS "Adding target: ${exec}")
  add_executable(${exec} ${PROJECT_SOURCE_DIR}/exe/${exec}.cxx)
  target_link_libraries(${exec} dataEval ${ROOT_LINK})
  add_dependencies(${exec} dataEval)
  install(TARGETS ${exec} DESTINATION bin)
endforeach()