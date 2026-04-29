# Target setup
add_library(g_wwdebug)

target_sources(
  g_wwdebug
  PRIVATE wwdebug.cpp
          wwdebug.h
          wwhack.h
          wwmemlog.cpp
          wwmemlog.h
          wwprofile.cpp
          wwprofile.h)

set_target_properties(g_wwdebug PROPERTIES OUTPUT_NAME "WWDebug")

target_include_directories(g_wwdebug PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(g_wwdebug
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../WWLib)

target_link_libraries(g_wwdebug PRIVATE g_library_include)
