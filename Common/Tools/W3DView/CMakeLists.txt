# GuiPrivate was introduced starting with 6.10
set(QT_NO_PRIVATE_MODULE_WARNING ON)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui OPTIONAL_COMPONENTS GuiPrivate)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

qt_add_executable(
  c_w3dview
  animationtoolbar.cpp
  animationtoolbar.h
  assetinfo.cpp
  assetinfo.h
  assettypes.h
  datatreemodel.cpp
  datatreemodel.h
  globals.cpp
  globals.h
  graphicview.cpp
  graphicview.h
  loddefs.h
  utils.cpp
  utils.h
  viewerassetmgr.cpp
  viewerassetmgr.h
  viewerscene.cpp
  viewerscene.h
  w3dview.cpp
  w3dview.h
  w3dviewdoc.cpp
  w3dviewdoc.h
  w3dviewwin.cpp
  w3dviewwin.h
  w3dviewwin.ui)

qt_add_resources(c_w3dview "w3d" PREFIX "/w3d" BASE Res FILES Res/Light.w3d)
qt_add_resources(c_w3dview "icons" PREFIX "/icons" BASE Res FILES 
  Res/aggregate.ico
  Res/anim.ico
  Res/anim_adaptive.ico
  Res/anim_timecoded.ico
  Res/emitter.ico
  Res/frame.ico
  Res/hierarchy.ico
  Res/icon1.ico
  Res/line.ico
  Res/lineup.ico
  Res/lod.ico
  Res/material.ico
  Res/mesh.ico
  Res/particle.ico
  Res/primitives.ico
  Res/rotation.ico
  Res/sound.ico
  Res/W3DView.ico)

set_target_properties(c_w3dview PROPERTIES OUTPUT_NAME "W3DView")
target_include_directories(c_w3dview PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(c_w3dview PRIVATE Qt6::Core Qt6::Widgets Qt6::Gui c_wwlib
                                        c_ww3d2 c_wwstub c_wwqt6)
if(TARGET Qt6::GuiPrivate)
  target_link_libraries(c_w3dview PRIVATE Qt6::GuiPrivate)
endif()