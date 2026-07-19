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

add_subdirectory(GameEngine)
add_subdirectory(GameEngineDevice)

# Smoke test: launch the actual RTS binary against the retail data the CI
# workflow downloads from S3. Only registered when that data is present (env
# var set at configure time), matching how the gtest retail-data tests skip
# themselves when it's absent (see RetailData.h).
if(DEFINED ENV{SAGE_RETAIL_DATA_DIR_GENERALS} AND NOT "$ENV{SAGE_RETAIL_DATA_DIR_GENERALS}" STREQUAL "")
  add_test(
    NAME Generals_RTS_SmokeTest
    COMMAND $<TARGET_FILE:g_rts_sdl>
            -dir "$ENV{SAGE_RETAIL_DATA_DIR_GENERALS}"
            -win -multi -quickstart -noaudio -novideo -nomusic -ignoreAsserts -autoexit 15
  )
  # -autoexit gives a real process exit (ctest PASS) instead of relying on
  # ctest's own TIMEOUT to kill a still-running-and-healthy process (which
  # ctest can only ever report as a failure). TIMEOUT here is just a safety
  # net in case shutdown itself hangs.
  set_tests_properties(Generals_RTS_SmokeTest PROPERTIES TIMEOUT 30)
endif()