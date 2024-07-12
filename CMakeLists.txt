cmake_minimum_required(VERSION 3.21)

project(
  hdtSMP-force-fields
  VERSION 0.9.1
  LANGUAGES CXX)

# ##############################################################################
# # Source build guard
# ##############################################################################

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

# ##############################################################################
# # Add CommonLibNG plugin
# ##############################################################################

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(CLIBNGPlugin)
add_simple_commonlibsse_ng_plugin(AUTHOR "jgernandt" LICENSE "MIT License"
                                  SRC_FOLDER "hdtSMP-force-fields")

# ##############################################################################
# # Mod specific
# ##############################################################################

find_package(Bullet CONFIG REQUIRED)
find_path(DETOURS_INCLUDE_DIRS "detours/detours.h")
find_library(DETOURS_LIBRARY detours REQUIRED)
find_path(CLIB_UTIL_INCLUDE_DIRS "ClibUtil/utils.hpp")

target_include_directories(${PROJECT_NAME} PRIVATE ${DETOURS_INCLUDE_DIRS}
                                                   ${CLIB_UTIL_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PRIVATE ${BULLET_LIBRARIES}
                                              ${DETOURS_LIBRARY})

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "HDT-SMP Force Fields")