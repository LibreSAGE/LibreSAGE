find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

qt_add_library(
  c_debugwindow SHARED
  DebugWindowExport.h
  DebugWindow.cpp
  DebugWindowDialog.cpp
  DebugWindowDialog.h
  DebugWindowDialog.ui)

set_target_properties(c_debugwindow PROPERTIES OUTPUT_NAME "DebugWindow")
target_include_directories(c_debugwindow PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(c_debugwindow PRIVATE Qt6::Widgets)
