cmake_minimum_required(VERSION 3.23)
project(dungeon-crawler CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# raylib via FetchContent so cloners need no system install
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
FetchContent_Declare(
  raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG 6.0
  GIT_SHALLOW TRUE
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.12.0
  GIT_SHALLOW TRUE
)
set(JSON_BuildTests OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(nlohmann_json)

add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE
  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall;-Wextra;-Wpedantic;-Wshadow;-Werror>
  $<$<CXX_COMPILER_ID:MSVC>:/W4;/permissive-;/WX>
)

# Build the core library before the targets that link it.
add_subdirectory(src/core)

enable_testing()

add_subdirectory(tests)

add_executable(dungeon-crawler
  src/main.cpp
)
# raylib: main.cpp includes <raylib.h> directly. nlohmann_json is unused by main today
# (kept from the original wiring; drop it if it's still unused when you next touch this).
target_link_libraries(dungeon-crawler PRIVATE game_core raylib nlohmann_json::nlohmann_json project_warnings)

# Hide the console window on Windows Release builds; keep it in Debug so
# stdout/stderr stay visible while iterating.
if(WIN32)
  set_target_properties(dungeon-crawler PROPERTIES
    WIN32_EXECUTABLE $<CONFIG:Release>
  )
  # /SUBSYSTEM:WINDOWS defaults to expecting WinMain; keep the standard
  # int main() entry point by forcing the CRT's main-style startup.
  target_link_options(dungeon-crawler PRIVATE
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/ENTRY:mainCRTStartup>
  )
endif()
