# Simulator tool: loads a replay file given on the command line and runs the
# game simulation on it, reusing the existing game engine and device layer.

# Generate the version headers the same way the main game target does.
add_custom_command(OUTPUT buildVersion.h COMMAND c_buildversionupdate ARGS
                                                 buildVersion.h)
add_custom_command(OUTPUT generatedVersion.h COMMAND c_versionupdate ARGS
                                                     generatedVersion.h)

add_executable(g_simulator)
target_sources(
  g_simulator
  PRIVATE SimulatorMain.cpp ${CMAKE_CURRENT_BINARY_DIR}/buildVersion.h
          ${CMAKE_CURRENT_BINARY_DIR}/generatedVersion.h)

set_target_properties(g_simulator PROPERTIES OUTPUT_NAME "simulator")

# Reuse the existing engine + device layer (no stub device layer).
target_link_libraries(g_simulator PRIVATE g_gameengine g_gameenginedevice)
target_include_directories(g_simulator PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
