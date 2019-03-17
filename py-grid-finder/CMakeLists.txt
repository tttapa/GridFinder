################################################################################
################################ PYTHON MODULE #################################
################################################################################

find_package(pybind11 REQUIRED)

# Create a Python module
pybind11_add_module(py_grid_finder src/Grid-Finder.module.cpp)

# Set target include directories 
#   These are the folders where the compiler will search for included header 
#   files.
#   The include files in the `include` folder are part of the public API of the
#   library, users of the library need to be able to include them in their code. 
target_include_directories(py_grid_finder
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

# Link the library with its dependencies.
#   This manages both the include directories of the dependencies, as well as  
#   the actual linking of the other libraries.
target_link_libraries(py_grid_finder
    PUBLIC 
        pybind11::module
        py-matrix
        grid_finder
)