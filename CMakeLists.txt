cmake_minimum_required(VERSION 3.23)
project(dungeon-crawler CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# raylib via FetchContent so cloners need no system install
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
FetchContent_Declare(
  raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG 5.5
  GIT_SHALLOW TRUE
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE
  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall;-Wextra;-Wpedantic;-Wshadow;-Werror>
  $<$<CXX_COMPILER_ID:MSVC>:/W4;/permissive-;/WX>
)

add_executable(dungeon-crawler src/main.cpp)
target_link_libraries(dungeon-crawler PRIVATE raylib project_warnings)

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
