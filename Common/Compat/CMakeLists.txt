add_library(c_compat INTERFACE)
target_include_directories(c_compat INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(UNIX)
  target_compile_definitions(c_compat INTERFACE -D_UNIX)
endif()
