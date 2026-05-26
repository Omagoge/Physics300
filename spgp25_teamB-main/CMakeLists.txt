cmake_minimum_required(VERSION 3.20)
project(game LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(TOAST_EDITOR ON CACHE BOOL "Build with TOAST editor features" FORCE)


# multithreaded compile
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

# Workaround for sol2 with GCC 15+
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "15.0")
    add_compile_options(-Wno-template-body)
	add_compile_options(-Wno-format-security)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wno-format-security)
    add_compile_options(-w)
endif()

# Add compile definitions based on build type
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(_DEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(GLM_FORCE_INTRINSICS NDEBUG)
endif()

# Prevent Windows.h from defining min/max macros
if(WIN32)
    add_compile_definitions(NOMINMAX)
endif()

# Forward TOAST_EDITOR to the engine BEFORE fetching it so it compiles accordingly
set(TOAST_EDITOR ${TOAST_EDITOR} CACHE BOOL "Build TOAST ENGINE with editor features" FORCE)

# Fetch toast-engine from GitHub
include(FetchContent)
FetchContent_Declare(
        toast-engine
        GIT_REPOSITORY https://github.com/nullptr-studios/toast-engine.git
        GIT_TAG v1/main
)
FetchContent_MakeAvailable(toast-engine)

# Collect source files
file(GLOB_RECURSE EDITOR_SOURCES "editor/*.cpp")
file(GLOB_RECURSE EDITOR_HEADERS "editor/*.hpp")
file(GLOB_RECURSE GAME_SOURCES   "game/*.cpp")
file(GLOB_RECURSE GAME_HEADERS   "game/*.hpp")
file(GLOB_RECURSE SRC_SOURCES    "src/*.cpp")
file(GLOB_RECURSE SRC_HEADERS    "src/*.hpp")

# Editor executable
add_executable(editor 
    ${EDITOR_SOURCES}
    ${EDITOR_HEADERS}
    ${SRC_SOURCES}
    ${SRC_HEADERS}
)

    target_include_directories(editor PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/editor
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_precompile_headers(editor PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h
    )

target_link_libraries(editor PRIVATE toast.engine)
target_compile_definitions(editor PRIVATE TOAST_EDITOR)

    # Prevent spine circular dependency
    if(UNIX AND NOT APPLE)
        target_link_options(editor PRIVATE "-Wl,--undefined=_ZN5spine19getDefaultExtensionEv")
    endif()

# Copy assets after build
add_custom_command(TARGET editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:editor>/assets
    COMMENT "Copying assets to build directory"
)

if (WIN32)
    add_custom_command(TARGET editor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${toast-engine_SOURCE_DIR}/external/fmod/lib/windows/"
        "$<TARGET_FILE_DIR:editor>"
        COMMENT "Copying FMOD libraries to editor output directory"
    )
    
    add_custom_command(TARGET editor POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${toast-engine_SOURCE_DIR}/external/ultralight/win/"
            "$<TARGET_FILE_DIR:editor>"
            COMMENT "Copying Ultralight libraries to editor output directory"
        )
    endif()

# Game executable
add_executable(game 
    ${GAME_SOURCES}
    ${GAME_HEADERS}
    ${SRC_SOURCES}
    ${SRC_HEADERS}
)

    target_include_directories(game PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/game
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_precompile_headers(game PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h
    )

target_link_libraries(game PRIVATE toast.engine)

# Prevent spine circular dependency
if(UNIX AND NOT APPLE)
    target_link_options(game PRIVATE "-Wl,--undefined=_ZN5spine19getDefaultExtensionEv")
endif()
