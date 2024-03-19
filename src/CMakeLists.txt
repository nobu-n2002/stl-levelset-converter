# cmake_minimum_required(VERSION 3.12)
# project(STLToPorosity
#     VERSION 1.0
#     DESCRIPTION "Converts STL files to porosity data"
#     LANGUAGES CXX)

# Find required packages
find_package(VTK REQUIRED COMPONENTS)

# Check if VTK was found
if (NOT VTK_FOUND)
  message(FATAL_ERROR "VTK not found. Please set VTK_DIR to the VTK build directory.")
endif()

# For OpenMP
find_package(OpenMP REQUIRED)
if(OpenMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# Create executable
add_executable(STLToPorosity stl2poro.cpp)

# Include directories
target_include_directories(STLToPorosity PRIVATE ${VTK_INCLUDE_DIRS})

# Link libraries
target_link_libraries(STLToPorosity PRIVATE ${VTK_LIBRARIES})

# Specify C++ standard
set_target_properties(STLToPorosity PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# Install target
install(TARGETS STLToPorosity
    RUNTIME DESTINATION bin
)

# MIT License
file(READ "stl2poro.cpp" SOURCE)
string(REGEX MATCH "(\\/\\*([^*]|(\\*+[^*/]))*\\*+\\/)" SOURCE_LICENSE "${SOURCE}")
string(APPEND LICENSE "${SOURCE_LICENSE}")

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/LICENSE.txt "${LICENSE}")
