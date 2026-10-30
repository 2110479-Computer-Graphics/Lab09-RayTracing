cmake_minimum_required(VERSION 3.16)
project(lab09 LANGUAGES CXX)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
  message(FATAL_ERROR
    "in-source builds overwrite this project's Makefile. Use: cmake -S . -B build")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

find_package(Threads REQUIRED)

set(STB_COMMIT f0569113c93ad095470c54bf34a17b36646bbbb5)
set(STB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/stb)
if(NOT EXISTS ${STB_DIR}/stb_image_write.h)
  message(STATUS "fetching stb_image_write.h")
  file(DOWNLOAD
    https://raw.githubusercontent.com/nothings/stb/${STB_COMMIT}/stb_image_write.h
    ${STB_DIR}/stb_image_write.h
    STATUS STB_STATUS
    TLS_VERIFY ON
  )
  list(GET STB_STATUS 0 STB_CODE)
  if(NOT STB_CODE EQUAL 0)
    file(REMOVE ${STB_DIR}/stb_image_write.h)
    message(FATAL_ERROR "could not download stb_image_write.h. Run external/fetch.sh "
                        "(external\\fetch.bat on Windows), or copy it into external/stb by hand")
  endif()
endif()
file(SHA256 ${STB_DIR}/stb_image_write.h STB_WRITE_HASH)
if(NOT STB_WRITE_HASH STREQUAL "cbd5f0ad7a9cf4468affb36354a1d2338034f2c12473cf1a8e32053cb6914a05")
  message(FATAL_ERROR "external/stb/stb_image_write.h does not match the pinned commit")
endif()
add_library(stb INTERFACE)
target_include_directories(stb SYSTEM INTERFACE ${STB_DIR})

set(STUDENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/student" CACHE PATH "folder with raytracer.cpp, mesh.cpp, pathtracer.cpp")
get_filename_component(STUDENT_PATH "${STUDENT_DIR}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(STUDENT_SOURCES ${STUDENT_PATH}/raytracer.cpp ${STUDENT_PATH}/mesh.cpp ${STUDENT_PATH}/pathtracer.cpp)
foreach(FILE ${STUDENT_SOURCES})
  if(NOT EXISTS ${FILE})
    message(FATAL_ERROR "missing ${FILE}")
  endif()
endforeach()
if(NOT STUDENT_PATH STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}/student")
  message(STATUS "student files: ${STUDENT_PATH}")
endif()

file(GLOB CORE_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cpp)
add_library(lab09core STATIC ${CORE_SOURCES} ${STUDENT_SOURCES})
target_include_directories(lab09core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/student)
target_compile_definitions(lab09core PUBLIC LAB09_ROOT="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(lab09core PUBLIC Threads::Threads PRIVATE stb)
target_compile_features(lab09core PUBLIC cxx_std_17)

if(MSVC)
  target_compile_options(lab09core PUBLIC /W4 /permissive-)
  target_compile_definitions(lab09core PUBLIC _CRT_SECURE_NO_WARNINGS)
  set_source_files_properties(${STUDENT_SOURCES} PROPERTIES COMPILE_OPTIONS "/wd4100")
else()
  target_compile_options(lab09core PUBLIC -Wall -Wextra)
  set_source_files_properties(${STUDENT_SOURCES} PROPERTIES COMPILE_OPTIONS "-Wno-unused-parameter")
endif()

foreach(PART raytrace mesh pathtrace)
  add_executable(${PART} src/${PART}_main.cpp)
  target_link_libraries(${PART} PRIVATE lab09core)
endforeach()
