cmake_minimum_required(VERSION 3.23)
project(RTS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(SAGE_USE_GLM "Use GLM for math operations" ON)
option(SAGE_USE_MILES "Use Miles Sound System for audio" OFF)
option(SAGE_USE_FFMPEG "Use FFmpeg for video playback" OFF)
option(SAGE_USE_BINK "Use Bink for video playback" OFF)
option(SAGE_USE_SDL3 "Use SDL3 for input handling" ON)
option(SAGE_USE_STDFS "Use std::filesystem for file handling" ON)

include(FeatureSummary)
add_feature_info(GLM SAGE_USE_GLM "Use GLM for math operations")
add_feature_info(Miles SAGE_USE_MILES "Use Miles Sound System for audio")
add_feature_info(FFmpeg SAGE_USE_FFMPEG "Use FFmpeg for video playback")
add_feature_info(Bink SAGE_USE_BINK "Use Bink for video playback")
add_feature_info(SDL3 SAGE_USE_SDL3 "Use SDL3 for input handling")
add_feature_info(stdfs SAGE_USE_STDFS "Use std::filesystem for file handling")
feature_summary(WHAT ALL)

add_subdirectory(Common)
add_subdirectory(Generals/Code)