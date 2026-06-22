find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

qt_add_executable(
  c_imagepacker
  ImageDirectory.h
  ImageInfo.cpp
  ImageInfo.h
  TexturePage.cpp
  TexturePage.h
  ImagePacker.cpp
  ImagePacker.h
  ImagePackerWindow.cpp
  ImagePackerWindow.h
  ImagePackerWindow.ui
  PreviewWindow.cpp
  PreviewWindow.h
  Main.cpp)

set_target_properties(c_imagepacker PROPERTIES OUTPUT_NAME "ImagePacker")
target_include_directories(c_imagepacker PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(c_imagepacker PRIVATE Qt6::Widgets c_wwlib c_wwstub)
