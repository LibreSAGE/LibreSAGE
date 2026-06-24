# Target setup
add_library(c_gameengine STATIC)
  
target_sources(c_gameengine
    PRIVATE
    Include/Common/AsciiString.h
    Include/Common/CriticalSection.h
    Include/Common/Debug.h
    Include/Common/Errors.h
    Include/Common/GameCommon.h
    Include/Common/GameMemory.h
    Include/Common/NameKeyGenerator.h
    Include/Common/STLTypedefs.h
    Include/Common/SubsystemInterface.h
    Include/Common/UnicodeString.h
    Source/Common/NameKeyGenerator.cpp
    Source/Common/System/AsciiString.cpp
    Source/Common/System/UnicodeString.cpp
)

set_target_properties(c_gameengine PROPERTIES OUTPUT_NAME "GameEngine"
                                              POSITION_INDEPENDENT_CODE TRUE)

target_link_libraries(c_gameengine PUBLIC c_wwvegas c_library_include)

target_include_directories(
  c_gameengine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
