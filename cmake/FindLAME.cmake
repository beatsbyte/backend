# Locate LAME library and headers
find_path(LAME_INCLUDE_DIR lame/lame.h)
find_library(LAME_LIBRARY mp3lame)

# Debugging output
message(STATUS "LAME_INCLUDE_DIR: ${LAME_INCLUDE_DIR}")
message(STATUS "LAME_LIBRARY: ${LAME_LIBRARY}")

if (LAME_INCLUDE_DIR AND LAME_LIBRARY)
    set(LAME_FOUND TRUE)
else()
    set(LAME_FOUND FALSE)
endif()

# Set variables for use
if (LAME_FOUND)
    set(LAME_INCLUDE_DIRS ${LAME_INCLUDE_DIR})
    set(LAME_LIBRARIES ${LAME_LIBRARY})
else()
    message(FATAL_ERROR "Could not find LAME! Ensure it is installed!")
endif()
