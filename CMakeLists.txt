cmake_minimum_required(VERSION 3.10)

# SET (CMAKE_C_COMPILER "clang")
# SET (CMAKE_CXX_COMPILER "clang++")
SET (CMAKE_C_COMPILER "gcc-8")
SET (CMAKE_CXX_COMPILER "g++-8")
SET (CMAKE_CXX_STANDARD 17)

project(eagle)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

if(NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif()
if(NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif()
if(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

################################################################################
##############################   GOOGLE TEST   #################################
################################################################################

# <copy>
# Download and unpack googletest at configure time
configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/GoogleTest-CMakeLists.txt.in 
               googletest/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_BINARY_DIR}/googletest/googletest-src
                 ${CMAKE_BINARY_DIR}/googletest/googletest-build
                 EXCLUDE_FROM_ALL)
# </copy>

include(GoogleTest)
enable_testing()

################################################################################
#############################   COMPILER OPTIONS   #############################
################################################################################

# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE) # LTO

# Compiler flags for C++
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} \
-march=native \
-O3 -g3 \
-Wall -Wextra -Werror \
-Wsuggest-override \
-Wno-register \
-Wno-missing-braces -Wno-unknown-pragmas \
-fdiagnostics-show-option \
-fextended-identifiers -finput-charset=UTF-8")

# Compiler flags for C
SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} \
-O3 -g3 \
-Wall -Wextra -Werror -pedantic")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

################################################################################
#################################   LIBRARIES   ################################
################################################################################

add_subdirectory("src")

################################################################################
###############################   APPLICATIONS   ###############################
################################################################################

# add_subdirectory("applications")

################################################################################
##############################   CUSTOM TARGETS   ##############################
################################################################################

# Add Doxygen target
add_custom_target(documentation
                  rm -rf html && doxygen
                  WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/doc")

# Builds a Python Wheel package and installs it using Pip
add_custom_target(python
                  Scripts/Install-Python.sh "${PYTHON_EXECUTABLE}"
                  WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}")