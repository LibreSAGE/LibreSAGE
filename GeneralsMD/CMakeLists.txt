# Set binary & library output directories to GeneralsMD
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/GeneralsMD)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/GeneralsMD)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/GeneralsMD)

add_subdirectory(GameEngine)
add_subdirectory(GameEngineDevice)
add_subdirectory(Main)
if (SAGE_BUILD_TESTS)
  add_subdirectory(Tests)
endif()
