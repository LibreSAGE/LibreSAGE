# CNC Modding Support published by EA
include(FetchContent)
FetchContent_Declare(
  cnc_mod_support
  GIT_REPOSITORY https://github.com/electronicarts/CnC_Modding_Support.git
  GIT_TAG main
)
FetchContent_MakeAvailable(cnc_mod_support)
set(SAGE_MOD_SUPPORT_DIR "${cnc_mod_support_SOURCE_DIR}")
set(SAGE_TEST_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Data")

# Header-only helpers shared by the Common, Generals and GeneralsMD test trees.
# Common/Tests is added before either game, so this target is already defined
# by the time they link against it.
add_library(sage_test_support INTERFACE)
target_include_directories(sage_test_support INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/Include")

add_subdirectory(Stub)
add_subdirectory(GameEngineDevice)
add_subdirectory(Compression)
add_subdirectory(WWVegas)