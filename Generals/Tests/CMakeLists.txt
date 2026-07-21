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

# Smoke tests: launch the actual RTS binary against the retail data the CI
# workflow downloads from S3, once per startup scenario (see
# Common/Tests/CMakeLists.txt for sage_add_smoke_test) -- isolating each flag
# means a failure points at which specific startup path broke instead of one
# opaque pass/fail for the whole flag set. Only registered when that data is
# present (env var set at configure time), matching how the gtest
# retail-data tests skip themselves when it's absent (see RetailData.h).
if(DEFINED ENV{SAGE_RETAIL_DATA_DIR_GENERALS} AND NOT "$ENV{SAGE_RETAIL_DATA_DIR_GENERALS}" STREQUAL "")
  # -autoexit gives a real process exit (ctest PASS) instead of relying on
  # ctest's own TIMEOUT to kill a still-running-and-healthy process (which
  # ctest can only ever report as a failure). TIMEOUT here is just a safety
  # net in case shutdown itself hangs.
  sage_add_smoke_test(
    NAME_PREFIX Generals_RTS_SmokeTest
    TARGET g_rts_sdl
    TIMEOUT 30
    BASE_ARGS -dir "$ENV{SAGE_RETAIL_DATA_DIR_GENERALS}" -win -ignoreAsserts -autoexit 15
    SCENARIOS
      "quickstart:-quickstart"
      "novideo:-novideo"
      "noaudio:-noaudio"
      "nomusic:-nomusic"
      "multi:-multi"
      "full:-multi -quickstart -noaudio -novideo -nomusic"
  )
endif()