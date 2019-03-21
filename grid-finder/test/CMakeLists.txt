# Add an executable with tests, and specify the source files to compile
add_executable(grid_finder_test 
    test-GridFinder.cpp
    test-Bresenham.cpp
    test-Angle.cpp
    test-Line.cpp
)
# Link the test executable with the Google Test main entry point and the library
# under test
target_link_libraries(grid_finder_test gtest_main grid_finder)

# Add the tests to Google Test
include(GoogleTest)
gtest_discover_tests(grid_finder_test)