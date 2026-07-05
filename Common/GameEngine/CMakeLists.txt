# Target setup
add_library(c_gameengine STATIC)
  
target_sources(c_gameengine
    PRIVATE
    Include/Common/ArchiveFile.h
    Include/Common/ArchiveFileSystem.h
    Include/Common/AsciiString.h
    Include/Common/CopyProtection.h
    Include/Common/CRC.h
    Include/Common/CriticalSection.h
    Include/Common/Debug.h
    Include/Common/Dict.h
    Include/Common/Directory.h
    Include/Common/Encrypt.h
    Include/Common/Errors.h
    Include/Common/FileSystem.h
    Include/Common/GameCommon.h
    Include/Common/GameMemory.h
    Include/Common/GameType.h
    Include/Common/INIException.h
    Include/Common/Language.h
    Include/Common/List.h
    Include/Common/LocalFile.h
    Include/Common/LocalFileSystem.h
    Include/Common/NameKeyGenerator.h
    Include/Common/Overridable.h
    Include/Common/Override.h
    Include/Common/PartitionSolver.h
    Include/Common/QuotedPrintable.h
    Include/Common/RAMFile.h
    Include/Common/Registry.h
    Include/Common/StackDump.h
    Include/Common/STLTypedefs.h
    Include/Common/StreamingArchiveFile.h
    Include/Common/SubsystemInterface.h
    Include/Common/UnicodeString.h
    Include/GameClient/Color.h
    Include/GameClient/Line2D.h
    Include/GameClient/Statistics.h
    Include/GameClient/VideoPlayer.h
    Include/GameClient/Water.h
    Include/GameLogic/FPUControl.h
    Include/GameNetwork/GameSpy/ThreadUtils.h
    Source/Common/INI/INI.cpp
    Source/Common/System/ArchiveFile.cpp
    Source/Common/System/ArchiveFileSystem.cpp
    Source/Common/System/AsciiString.cpp
    Source/Common/System/CopyProtection.cpp
    Source/Common/System/CriticalSection.cpp
    Source/Common/System/Debug.cpp
    Source/Common/System/Directory.cpp
    Source/Common/System/Encrypt.cpp
    Source/Common/System/File.cpp
    Source/Common/System/FileSystem.cpp
    Source/Common/System/GameMemory.cpp
    Source/Common/System/GameType.cpp
    Source/Common/System/List.cpp
    Source/Common/System/LocalFile.cpp
    Source/Common/System/LocalFileSystem.cpp
    Source/Common/System/MemoryInit.cpp
    Source/Common/System/QuotedPrintable.cpp
    Source/Common/System/RAMFile.cpp
    Source/Common/System/Registry.cpp
    Source/Common/System/StackDump.cpp
    Source/Common/System/StreamingArchiveFile.cpp
    Source/Common/System/SubsystemInterface.cpp
    Source/Common/System/UnicodeString.cpp
    Source/Common/CRC.cpp
    Source/Common/Dict.cpp
    Source/Common/Language.cpp
    Source/Common/NameKeyGenerator.cpp
    Source/Common/PartitionSolver.cpp
    Source/GameClient/Color.cpp
    Source/GameClient/Line2D.cpp
    Source/GameClient/Statistics.cpp
    Source/GameClient/VideoPlayer.cpp
    Source/GameClient/Water.cpp
    Source/GameLogic/System/FPUControl.cpp
    Source/GameNetwork/GameSpy/Thread/ThreadUtils.cpp
)

set_target_properties(c_gameengine PROPERTIES OUTPUT_NAME "GameEngine"
                                              POSITION_INDEPENDENT_CODE TRUE)

target_link_libraries(c_gameengine PUBLIC c_wwvegas c_library_include)

target_include_directories(
  c_gameengine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
