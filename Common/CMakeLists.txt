# Set binary & library output directories to Common
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/Common)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/Common)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/Common)

add_subdirectory(Compat)
add_subdirectory(Libraries)
add_subdirectory(Tools)
if (SAGE_BUILD_TESTS)
  add_subdirectory(Tests)
endif()
