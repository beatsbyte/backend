# Locate mpg123 library and headers
find_path(MPG123_INCLUDE_DIR mpg123.h)
find_library(MPG123_LIBRARY mpg123)

# Debugging output
message(STATUS "MPG123_INCLUDE_DIR: ${MPG123_INCLUDE_DIR}")
message(STATUS "MPG123_LIBRARY: ${MPG123_LIBRARY}")

# Check if both the header and library are found
if (MPG123_INCLUDE_DIR AND MPG123_LIBRARY)
    set(MPG123_FOUND TRUE)
else()
    set(MPG123_FOUND FALSE)
endif()

# Set variables for use
if (MPG123_FOUND)
    set(MPG123_INCLUDE_DIRS ${MPG123_INCLUDE_DIR})
    set(MPG123_LIBRARIES ${MPG123_LIBRARY})
else()
    message(FATAL_ERROR "Could not find mpg123! Ensure it is installed")
endif()
