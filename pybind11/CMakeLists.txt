cmake_minimum_required(VERSION 3.16)
project(point_litgen_example LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# Fetch pybind11
FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG        v2.11.0
)
FetchContent_MakeAvailable(pybind11)

# Create a pybind11 module from the generated C++ file
pybind11_add_module(pose_module
    ${CMAKE_SOURCE_DIR}/pose_module.cpp
)

# This helps Pybind11 produce the correct symbol name for Python
set_target_properties(pose_module PROPERTIES
    PREFIX ""
    SUFFIX ".so"
    OUTPUT_NAME "pose_module"
)

add_custom_target(copy_shared_lib
    ALL
    DEPENDS pose_module
    COMMENT "Copying '.so' file to the output directory"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_BINARY_DIR}/pose_module.so" "${CMAKE_SOURCE_DIR}/pose_module.so"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    VERBATIM
)

###############################################################
# POST BUILD
# Create a `.pyi` stub with `stubgen`
# Requires `mypy` to be installed
###############################################################
# check if `stubgen` is available
find_program(STUBGEN_EXECUTABLE NAMES stubgen)
if(NOT STUBGEN_EXECUTABLE)
        message(FATAL_ERROR "stubgen not found. Please run 'pip install mypy' to generate .pyi stub files.")
endif()

add_custom_target(gen_stub
    ALL
    DEPENDS pose_module
    COMMAND ${STUBGEN_EXECUTABLE} --include-docstrings -o . -m pose_module
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_BINARY_DIR}/pose_module.pyi" "${CMAKE_SOURCE_DIR}/pose_module.pyi"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    VERBATIM
)
