cmake_minimum_required(VERSION 3.15)
project(SceneDirectorAI VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OBS Studio
if(NOT DEFINED LIBOBS_INCLUDE_DIR)
    set(LIBOBS_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../../obs-studio/libobs" CACHE PATH "libobs include directory")
endif()

if(NOT DEFINED LIBOBS_LIB)
    set(LIBOBS_LIB "${CMAKE_SOURCE_DIR}/../../obs-studio/build/libobs/libobs.so" CACHE PATH "libobs library file")
endif()

# Include directories
include_directories(
    ${LIBOBS_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

# Source files
set(SOURCES
    src/scene-director-ai.cpp
    src/window-detector.cpp
    src/audio-monitor.cpp
    src/websocket-server.cpp
    src/scene-predictor.cpp
)

set(HEADERS
    src/scene-director-ai.hpp
    src/window-detector.hpp
    src/audio-monitor.hpp
    src/websocket-server.hpp
    src/scene-predictor.hpp
)

# Create shared library
add_library(${PROJECT_NAME} SHARED
    ${SOURCES}
    ${HEADERS}
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${LIBOBS_LIB}
)

# Platform-specific settings
if(WIN32)
    target_link_libraries(${PROJECT_NAME}
        Ws2_32
        Winmm
        User32
        Shell32
    )
    set_target_properties(${PROJECT_NAME} PROPERTIES
        PREFIX ""
        SUFFIX ".dll"
    )
elseif(APPLE)
    find_library(CORE_FOUNDATION_FRAMEWORK CoreFoundation)
    find_library(CORESERVICES_FRAMEWORK CoreServices)
    target_link_libraries(${PROJECT_NAME}
        ${CORE_FOUNDATION_FRAMEWORK}
        ${CORESERVICES_FRAMEWORK}
    )
    set_target_properties(${PROJECT_NAME} PROPERTIES
        PREFIX ""
        SUFFIX ".so"
    )
endif()

# Installation
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION obs-plugins/64bit
    RUNTIME DESTINATION obs-plugins/64bit
)

# Copy locale files
install(DIRECTORY locale/
    DESTINATION data/obs-plugins/SceneDirectorAI
    USE_SOURCE_PERMISSIONS)
