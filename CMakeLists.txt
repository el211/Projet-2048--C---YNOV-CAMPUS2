cmake_minimum_required(VERSION 3.16)
project(2048_sdl2 C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(SDL2 CONFIG QUIET)

if(NOT SDL2_FOUND)
    include(FetchContent)
    set(SDL_TESTS OFF CACHE BOOL "" FORCE)
    set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
    set(SDL_SHARED ON CACHE BOOL "" FORCE)
    set(SDL_STATIC OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(SDL2
        URL https://github.com/libsdl-org/SDL/releases/download/release-2.30.11/SDL2-2.30.11.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(SDL2)
endif()

add_executable(2048
    main.c
    grid.c
    logic.c
    render.c
    animation.c
)

# SDL2main must be linked before SDL2: it references SDL_* symbols that the
# linker only resolves from libraries listed after it (left-to-right resolution).
if(TARGET SDL2::SDL2main)
    target_link_libraries(2048 PRIVATE SDL2::SDL2main)
endif()
target_link_libraries(2048 PRIVATE SDL2::SDL2)

if(WIN32)
    add_custom_command(TARGET 2048 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL2::SDL2>
            $<TARGET_FILE_DIR:2048>
        VERBATIM
    )
endif()
