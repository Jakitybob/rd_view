cmake_minimum_required(VERSION 4.2)
project(rd_view)

set(CMAKE_CXX_STANDARD 20)

# Find the X11 package
find_package(X11 REQUIRED)

# Add all header files from the include directory
include_directories(${PROJECT_SOURCE_DIR}/include)

# Add our static library
add_library(cs631 STATIC IMPORTED)
set_target_properties(cs631 PROPERTIES
    IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/libcs631.a
)

# Add X11 include directories
if (X11_FOUND)
    include_directories(${X11_INCLUDE_DIR})
endif()

# Create the executable
add_executable(${PROJECT_NAME}
        src/rd_direct.cpp
        src/pnm_display.cpp
        include/pnm_display.h
        include/rd_direct.h
        include/rd_enginebase.h
        include/rd_error.h
        include/rd_display.h
        include/global_variables.h
)

# Link the library to the executable
target_link_libraries(${PROJECT_NAME} cs631 ${X11_LIBRARIES})
