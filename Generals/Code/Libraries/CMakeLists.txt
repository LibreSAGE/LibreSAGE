add_library(g_library_include INTERFACE)
target_include_directories(g_library_include
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_link_libraries(g_library_include INTERFACE c_compat)

add_subdirectory(Source/Compression)
add_subdirectory(Source/WWVegas)
