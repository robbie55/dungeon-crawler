add_library(dungeon_core STATIC
  grid/tile_coord.cpp
)

target_include_directories(dungeon_core PUBLIC ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(dungeon_core PUBLIC raylib)
target_link_libraries(dungeon_core PRIVATE project_warnings)

target_compile_features(dungeon_core PUBLIC cxx_std_17)
