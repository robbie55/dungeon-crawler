include(FetchContent)

FetchContent_Declare(
  doctest
  GIT_REPOSITORY https://github.com/doctest/doctest.git
  GIT_TAG 6804767ee637789db8a5cb281381cae98dc36906 # v2.5.2
)
FetchContent_MakeAvailable(doctest)

list(APPEND CMAKE_MODULE_PATH ${doctest_SOURCE_DIR}/scripts/cmake)
include(doctest)

add_executable(tests
  test_smoke.cpp
  dummy/tile_coord.cpp
  dummy/test_tile_coord.cpp
)

target_link_libraries(tests PRIVATE doctest::doctest project_warnings)

target_compile_features(tests PRIVATE cxx_std_17)

doctest_discover_tests(tests)
