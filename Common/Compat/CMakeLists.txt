add_library(c_compat STATIC d3d8math.cpp)
target_include_directories(c_compat PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(UNIX)
  target_compile_definitions(c_compat PUBLIC -D_UNIX)
  target_link_libraries(c_compat PUBLIC DXVK::D3D8)
endif()

if(SAGE_USE_GLM)
  find_package(glm CONFIG REQUIRED)
  target_link_libraries(c_compat PRIVATE glm::glm)
  target_compile_definitions(c_compat PRIVATE SAGE_USE_GLM)
endif()