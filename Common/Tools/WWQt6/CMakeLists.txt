# GuiPrivate was introduced starting with 6.10
set(QT_NO_PRIVATE_MODULE_WARNING ON)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui OPTIONAL_COMPONENTS GuiPrivate)

qt_add_library(c_wwqt6 STATIC
    qfilewrapper.cpp
    qfilewrapper.h
    qsdlwindow.cpp
    qsdlwindow.h
)

target_include_directories(c_wwqt6 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(c_wwqt6 PUBLIC Qt6::Core Qt6::Widgets Qt6::Gui c_wwlib c_wwstub)
if(TARGET Qt6::GuiPrivate)
  target_link_libraries(c_wwqt6 PRIVATE Qt6::GuiPrivate)
endif()