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

# sage_add_smoke_test: registers one ctest test per startup scenario, all
# launching the same TARGET with the same BASE_ARGS, differing only in the
# extra flags each scenario appends. Splitting one "launch with everything"
# smoke test into named per-flag scenarios means a failure points at which
# specific startup flag broke, instead of one opaque pass/fail for a whole
# flag combination -- and it's shared here so both the RTS and WorldBuilder
# smoke tests (and any future launch-based smoke test) can reuse it.
#
# Usage:
#   sage_add_smoke_test(
#     NAME_PREFIX GeneralsMD_RTS_SmokeTest
#     TARGET z_rts_sdl
#     TIMEOUT 30
#     BASE_ARGS -dir "${SOME_DIR}" -ignoreAsserts -autoexit 15
#     ENVIRONMENT "FOO=bar;BAZ=qux"
#     SCENARIOS
#       "quickstart:-quickstart"
#       "novideo:-novideo"
#   )
# Each entry in SCENARIOS is "name:extra flags", where "extra flags" is a
# plain space-separated command-line string (parsed with separate_arguments,
# so ordinary shell-style quoting works) appended after BASE_ARGS. Registers
# one test per scenario named "${NAME_PREFIX}_${name}".
function(sage_add_smoke_test)
  set(oneValueArgs NAME_PREFIX TARGET TIMEOUT)
  set(multiValueArgs BASE_ARGS SCENARIOS ENVIRONMENT)
  cmake_parse_arguments(SAT "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(NOT SAT_NAME_PREFIX OR NOT SAT_TARGET OR NOT SAT_SCENARIOS)
    message(FATAL_ERROR "sage_add_smoke_test: NAME_PREFIX, TARGET and SCENARIOS are required")
  endif()

  foreach(scenario IN LISTS SAT_SCENARIOS)
    string(REPLACE ":" ";" scenario_split "${scenario}")
    list(GET scenario_split 0 scenario_name)
    list(REMOVE_AT scenario_split 0)
    # Rejoin (in case the flags themselves contained a ':') into one string,
    # then parse that into real, separate command-line arguments.
    list(JOIN scenario_split ":" scenario_flags_str)
    separate_arguments(scenario_flags UNIX_COMMAND "${scenario_flags_str}")

    set(test_name "${SAT_NAME_PREFIX}_${scenario_name}")
    add_test(
      NAME ${test_name}
      COMMAND $<TARGET_FILE:${SAT_TARGET}> ${SAT_BASE_ARGS} ${scenario_flags}
    )

    # Set each property with its own call rather than building one combined
    # PROPERTIES list: ENVIRONMENT's value is itself a ';'-joined string, and
    # list(APPEND ...) can't hold a literal ';' inside one element -- it
    # would just splice in as extra (unpaired) list items.
    if(DEFINED SAT_TIMEOUT AND NOT "${SAT_TIMEOUT}" STREQUAL "")
      set_tests_properties(${test_name} PROPERTIES TIMEOUT ${SAT_TIMEOUT})
    endif()
    if(SAT_ENVIRONMENT)
      set_tests_properties(${test_name} PROPERTIES ENVIRONMENT "${SAT_ENVIRONMENT}")
    endif()
  endforeach()
endfunction()

add_subdirectory(Stub)
add_subdirectory(GameEngineDevice)
add_subdirectory(Compression)
add_subdirectory(WWVegas)