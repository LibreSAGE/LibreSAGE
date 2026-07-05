# Target setup
add_library(c_gameenginedevice)

target_sources(
  c_gameenginedevice
  PRIVATE Include/W3DDevice/Common/W3DConvert.h
          Include/W3DDevice/GameClient/W3DDynamicLight.h
          Source/W3DDevice/Common/W3DConvert.cpp
          Source/W3DDevice/GameClient/W3DDynamicLight.cpp)

if(SAGE_USE_SDL3)
  target_sources(
    c_gameenginedevice
    PRIVATE Include/SDL3Device/Common/SDL3BIGFile.h
            Include/SDL3Device/Common/SDL3BIGFileSystem.h
            Include/SDL3Device/Common/SDL3LocalFile.h
            Include/SDL3Device/Common/SDL3LocalFileSystem.h
            Source/SDL3Device/Common/SDL3BIGFile.cpp
            Source/SDL3Device/Common/SDL3BIGFileSystem.cpp
            Source/SDL3Device/Common/SDL3LocalFile.cpp
            Source/SDL3Device/Common/SDL3LocalFileSystem.cpp)

  find_package(SDL3 CONFIG REQUIRED)
  find_package(SDL3_image CONFIG REQUIRED)
  target_link_libraries(c_gameenginedevice PUBLIC SDL3::SDL3
                                                  SDL3_image::SDL3_image)
  target_compile_definitions(c_gameenginedevice PUBLIC SAGE_USE_SDL3)
endif()

if(SAGE_USE_FFMPEG)
  target_sources(
    c_gameenginedevice
    PRIVATE Include/VideoDevice/FFmpeg/FFmpegFile.h
            Source/VideoDevice/FFmpeg/FFmpegFile.cpp)

  find_package(FFMPEG REQUIRED)
  target_include_directories(c_gameenginedevice PRIVATE ${FFMPEG_INCLUDE_DIRS})
  target_link_libraries(c_gameenginedevice PRIVATE ${FFMPEG_LIBRARIES})
  target_compile_definitions(c_gameenginedevice PRIVATE SAGE_USE_FFMPEG)
endif()

if(WIN32)
  target_sources(
    c_gameenginedevice
    PRIVATE Include/Win32Device/Common/Win32BIGFile.h
            Include/Win32Device/Common/Win32BIGFileSystem.h
            Include/Win32Device/Common/Win32LocalFile.h
            Include/Win32Device/Common/Win32LocalFileSystem.h
            Source/Win32Device/Common/Win32BIGFile.cpp
            Source/Win32Device/Common/Win32BIGFileSystem.cpp
            Source/Win32Device/Common/Win32LocalFile.cpp
            Source/Win32Device/Common/Win32LocalFileSystem.cpp)
endif()

set_target_properties(c_gameenginedevice PROPERTIES OUTPUT_NAME
                                                    "GameEngineDevice")

target_include_directories(c_gameenginedevice
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)

target_link_libraries(c_gameenginedevice PUBLIC c_gameengine)
