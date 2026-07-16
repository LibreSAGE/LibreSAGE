# Qt6 port of the Zero Hour WorldBuilder.  The MFC sources in src/ & include/
# are ported to Qt6 in place; files are added here as they are ported.

# GuiPrivate was introduced starting with 6.10
set(QT_NO_PRIVATE_MODULE_WARNING ON)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui OPTIONAL_COMPONENTS GuiPrivate)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

qt_add_executable(
  z_worldbuilder
  include/addplayerdialog.h
  include/BrushTool.h
  include/CUndoable.h
  include/DrawObject.h
  include/FeatherOptions.h
  include/FeatherTool.h
  include/HandScrollTool.h
  include/MainFrm.h
  include/MapSettings.h
  include/MoundOptions.h
  include/MoundTool.h
  include/ObjectOptions.h
  include/ObjectPreview.h
  include/ObjectTool.h
  include/PointerTool.h
  include/PolygonTool.h
  include/RoadTool.h
  include/SplashScreen.h
  include/Tool.h
  include/WBHeightMap.h
  include/WHeightMapEdit.h
  include/WorldBuilder.h
  include/WorldBuilderDoc.h
  include/brushoptions.h
  include/mapobjectprops.h
  include/wbview.h
  include/wbview3d.h
  src/addplayerdialog.cpp
  src/BrushTool.cpp
  src/CUndoable.cpp
  src/DrawObject.cpp
  src/FeatherOptions.cpp
  src/FeatherTool.cpp
  src/HandScrollTool.cpp
  src/MainFrm.cpp
  src/MapSettings.cpp
  src/MoundOptions.cpp
  src/MoundTool.cpp
  src/ObjectOptions.cpp
  src/ObjectPreview.cpp
  src/ObjectTool.cpp
  src/PointerTool.cpp
  src/PolygonTool.cpp
  src/RoadTool.cpp
  src/SplashScreen.cpp
  src/Tool.cpp
  src/WBHeightMap.cpp
  src/WHeightMapEdit.cpp
  src/WorldBuilder.cpp
  src/WorldBuilderDoc.cpp
  src/brushoptions.cpp
  src/mapobjectprops.cpp
  src/placeholderview.cpp
  src/wbview.cpp
  src/wbview3d.cpp)

# The splash artwork (IDB_WORLDBUILDERSPLASH in the original .rc).
qt_add_resources(z_worldbuilder "splash" PREFIX "/" BASE res FILES
                 res/WorldBuilder.bmp)

# Tool cursors from the original resources.
qt_add_resources(z_worldbuilder "cursors" PREFIX "/cursors" BASE res FILES
                 res/cross.cur
                 res/IDC_HAND_SCROLL.cur
                 res/IDC_MOVE_POINTER.cur
                 res/IDC_POINTER.cur
                 res/IDC_ROTATE.cur
                 res/IDC_ROAD.cur)

set_target_properties(z_worldbuilder PROPERTIES OUTPUT_NAME "WorldBuilder")

target_include_directories(z_worldbuilder
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(z_worldbuilder PRIVATE Qt6::Core Qt6::Widgets Qt6::Gui
                                             z_gameengine z_gameenginedevice c_wwqt6)
if(TARGET Qt6::GuiPrivate)
  target_link_libraries(z_worldbuilder PRIVATE Qt6::GuiPrivate)
endif()
