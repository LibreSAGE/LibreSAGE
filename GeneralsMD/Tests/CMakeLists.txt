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
# Zero Hour also needs the base Generals assets via -bigdir.
if(DEFINED ENV{SAGE_RETAIL_DATA_DIR_GENERALSMD} AND NOT "$ENV{SAGE_RETAIL_DATA_DIR_GENERALSMD}" STREQUAL ""
   AND DEFINED ENV{SAGE_RETAIL_DATA_DIR_GENERALS} AND NOT "$ENV{SAGE_RETAIL_DATA_DIR_GENERALS}" STREQUAL "")
  # -autoexit gives a real process exit (ctest PASS) instead of relying on
  # ctest's own TIMEOUT to kill a still-running-and-healthy process (which
  # ctest can only ever report as a failure). TIMEOUT here is just a safety
  # net in case shutdown itself hangs.
  sage_add_smoke_test(
    NAME_PREFIX GeneralsMD_RTS_SmokeTest
    TARGET z_rts_sdl
    TIMEOUT 30
    BASE_ARGS -dir "$ENV{SAGE_RETAIL_DATA_DIR_GENERALSMD}"
              -bigdir "$ENV{SAGE_RETAIL_DATA_DIR_GENERALS}"
              -win -ignoreAsserts -autoexit 15
    SCENARIOS
      "quickstart:-quickstart"
      "novideo:-novideo"
      "noaudio:-noaudio"
      "nomusic:-nomusic"
      "multi:-multi"
      "full:-multi -quickstart -noaudio -novideo -nomusic"
  )
endif()

# Smoke test: launch the WorldBuilder editor against the retail data and have
# it load a real map from cnc_mod_support, then quit automatically. Mirrors
# the RTS smoke tests above via the same sage_add_smoke_test helper;
# WorldBuilder has no -autoexit/-quickstart flags of its own, so the
# equivalent behavior is driven via the WB_TEST_OPEN / WB_TEST_AUTOEXIT env
# var hooks in WorldBuilder.cpp's main(). Guarded on SAGE_BUILD_TOOLS too,
# since the z_worldbuilder target it references only exists when Tools is
# added as a subdirectory (independent of SAGE_BUILD_TESTS).
if(SAGE_BUILD_TOOLS
   AND DEFINED ENV{SAGE_RETAIL_DATA_DIR_GENERALSMD} AND NOT "$ENV{SAGE_RETAIL_DATA_DIR_GENERALSMD}" STREQUAL ""
   AND DEFINED ENV{SAGE_RETAIL_DATA_DIR_GENERALS} AND NOT "$ENV{SAGE_RETAIL_DATA_DIR_GENERALS}" STREQUAL "")
  set(SAGE_WORLDBUILDER_SMOKETEST_MAP
      "${SAGE_MOD_SUPPORT_DIR}/Zero Hour/Maps/official/Tournament Plains/Tournament Plains.map")
  sage_add_smoke_test(
    NAME_PREFIX GeneralsMD_WorldBuilder_SmokeTest
    TARGET z_worldbuilder
    TIMEOUT 30
    BASE_ARGS -dir "$ENV{SAGE_RETAIL_DATA_DIR_GENERALSMD}"
              -bigdir "$ENV{SAGE_RETAIL_DATA_DIR_GENERALS}"
              -win
    ENVIRONMENT "WB_TEST_OPEN=${SAGE_WORLDBUILDER_SMOKETEST_MAP};WB_TEST_AUTOEXIT=15;QT_QPA_PLATFORM=xcb;SDL_VIDEODRIVER=x11"
    SCENARIOS
      "default:-ignoreAsserts"
  )
endif()