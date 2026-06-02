add_library(game_core STATIC
  logger.cpp
  states.cpp
  state_fsm.cpp
  fsm_helpers.cpp
)

# PUBLIC so consumers (the game executable, the tests) can #include "core/..."
target_include_directories(game_core
  PUBLIC ${CMAKE_SOURCE_DIR}/src
)

# project_warnings is PRIVATE: the -Werror bar applies while compiling core's own
# translation units; consumers carry their own copy.
target_link_libraries(game_core PRIVATE project_warnings)

# PUBLIC so anything linking game_core inherits C++20 (this is what lets the tests
# target drop its hand-set standard).
target_compile_features(game_core PUBLIC cxx_std_20)

# NOTE: no raylib / nlohmann_json here on purpose — nothing compiled into game_core
# includes them yet. raylib becomes a real dependency once State::Render() bodies call
# raylib draws (PUBLIC if raylib types reach a core header, PRIVATE if only in a .cpp);
# nlohmann_json arrives with save/load (DUN-30). Add them then, not before.
