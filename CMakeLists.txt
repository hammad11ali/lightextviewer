cmake_minimum_required(VERSION 3.20)
project(LightweightEditor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Windows-specific settings
if(WIN32)
    add_definitions(-DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN -DNOMINMAX)
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/window.cpp
    src/renderer.cpp
    src/piece_tree.cpp
    src/search_engine.cpp
    src/undo_manager.cpp
    src/file_handler.cpp
    src/clipboard_handler.cpp
    src/utf_codec.cpp
)

# Header files
set(HEADERS
    include/window.h
    include/renderer.h
    include/piece_tree.h
    include/search_engine.h
    include/undo_manager.h
    include/file_handler.h
    include/clipboard_handler.h
    include/utf_codec.h
    include/types.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link Windows libraries
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        user32
        gdi32
        ole32
        uuid
        dwrite
    )
endif()

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
