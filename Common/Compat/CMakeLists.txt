add_library(c_compat INTERFACE)
target_include_directories(c_compat INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(UNIX)
  target_compile_definitions(c_compat INTERFACE -D_UNIX)
  target_link_libraries(c_compat INTERFACE DXVK::D3D8)
endif()
