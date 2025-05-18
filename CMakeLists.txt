# Project/
# ├── src/                # Source code files
# │   ├── main            # Main entry point
# │   ├── module1         # Module 1 logic
# │   ├── module2         # Module 2 logic
# │   ├── utilities       # Utility functions 
# │   └── assets          # Asset handling
# ├── include/            # Header files
# │   ├── module1.h
# │   ├── module2.h
# │   ├── utilities.h
# │   ├── assets.h
# │   └── config.h        # Configuration constants
# ├── tests/              # Test files
# │   ├── test_assets     # Test cases for the main program
# │   ├── test_utils      # Test cases for module 1
# │   └── test_some       # Test cases for module 2
# ├── data/               # Data files
# │   ├── logs            # Log files
# │   └── assets/         # Additional assets
# ├── bin/                # Compiled binaries
# │   └── executable      # Final executable
# ├── lib/               # External libraries
# ├── build/              # Build artifacts
# │   ├── Debug/          # Debug build files // object files go here
# │   └── Release/        # Release build files
# ├── .config/            # IDE or tool configuration files
# │   ├── tasks.json      # Build tasks
# │   ├── launch.json     # Debug configuration
# │   ├── settings.json   # Project-specific settings
# │   └── properties.json # Tool-specific configuration
# ├── Makefile            # build script  
# └── README.md           # Project documentation

cmake_minimum_required(VERSION 3.10)

project(C-ChessLib VERSION 1.0 LANGUAGES C)
add_subdirectory(lib)


file(GLOB_RECURSE SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.c"
)

add_library(C-ChessLib STATIC ${SOURCES})

add_executable(chess "${PROJECT_SOURCE_DIR}/src/main.c")

target_link_libraries(chess PRIVATE C-ChessLib)
target_include_directories(chess PRIVATE "${CMAKE_SOURCE_DIR}/include")


target_link_libraries(C-ChessLib PUBLIC zlib)

target_include_directories(C-ChessLib PUBLIC
                    "${PROJECT_BINARY_DIR}"
                    "${PROJECT_SOURCE_DIR}/include"
)

add_subdirectory(tests)

enable_testing()

