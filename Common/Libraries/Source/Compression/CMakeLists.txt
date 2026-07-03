# Target setup
add_library(c_compression)

target_sources(
  c_compression
  PRIVATE Compression.h
          CompressionManager.cpp
          EAC/btreeabout.cpp
          EAC/btreecodex.h
          EAC/btreedecode.cpp
          EAC/btreeencode.cpp
          EAC/codex.h
          EAC/gimex.h
          EAC/huffabout.cpp
          EAC/huffcodex.h
          EAC/huffdecode.cpp
          EAC/huffencode.cpp
          EAC/refabout.cpp
          EAC/refcodex.h
          EAC/refdecode.cpp
          EAC/refencode.cpp
          LZHCompress/NoxCompress.cpp
          LZHCompress/NoxCompress.h)

set_target_properties(c_compression PROPERTIES OUTPUT_NAME "Compression")

target_link_libraries(c_compression PUBLIC c_library_include)
target_include_directories(c_compression PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# ZLIB
find_package(ZLIB REQUIRED)
target_link_libraries(c_compression PRIVATE ZLIB::ZLIB)

# LZHL
include(FetchContent)
FetchContent_Declare(
  lzhl
  GIT_REPOSITORY https://github.com/TheAssemblyArmada/liblzhl.git
)

FetchContent_MakeAvailable(lzhl)
target_link_libraries(c_compression PRIVATE LZHL::LZHL)