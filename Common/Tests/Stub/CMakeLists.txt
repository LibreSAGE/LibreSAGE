# A compiled build of the Common engine and device sources, for tests.
#
# c_gameengine and c_gameenginedevice are INTERFACE source lists: they carry
# sources but produce no archive, so each game compiles them into its own
# g_*/z_* libraries under RTS_GAME_GEN / RTS_GAME_ZH. Tests under Common/Tests
# have no game to borrow from, so this target links the interface libraries --
# pulling in their sources, include paths and dependencies -- and compiles them
# once, game-agnostically.
add_library(c_gameenginestub STATIC gamestub.cpp)

# PRIVATE, not PUBLIC: an INTERFACE source list propagates its sources to every
# consumer, so a PUBLIC link would make each test executable recompile the whole
# engine itself. Linking privately keeps the sources here, and the include paths
# are re-exported below so consumers can still find the headers.
target_link_libraries(c_gameenginestub PRIVATE c_gameenginedevice)

target_include_directories(c_gameenginestub PUBLIC
  $<TARGET_PROPERTY:c_gameenginedevice,INTERFACE_INCLUDE_DIRECTORIES>)

# A handful of Common sources still include headers that live only in the game
# trees (Common/Player.h, GameLogic/Weapon.h, GameLogic/Object.h,
# GameLogic/ExperienceTracker.h, Common/MessageStream.h). They are gameplay
# code that no Common test exercises, so drop them from this target rather than
# block the whole library on finishing their commonization. HEADER_FILE_ONLY
# excludes a source from compilation for one target without disturbing the
# shared interface list. Remove entries here as their headers move to Common.
set(SAGE_STUB_UNCOMMONIZED_SOURCES
    Common/GameEngine/Source/Common/RTS/Science.cpp
    Common/GameEngine/Source/GameClient/MessageStream/HotKey.cpp
    Common/GameEngine/Source/GameLogic/Object/Behavior/TechBuildingBehavior.cpp
    Common/GameEngine/Source/GameLogic/Object/Collide/FireWeaponCollide.cpp
    Common/GameEngine/Source/GameLogic/Object/Die/DieModule.cpp
    Common/GameEngine/Source/GameLogic/Object/Upgrade/StatusBitsUpgrade.cpp
    Common/GameEngine/Source/GameLogic/Map/SidesList.cpp
    Common/GameEngine/Source/GameLogic/Map/PolygonTrigger.cpp
    Common/GameEngine/Source/GameLogic/Map/TerrainLogic.cpp
    Common/GameEngine/Source/GameNetwork/NetPacket.cpp
    Common/GameEngine/Source/GameNetwork/NetworkUtil.cpp
    Common/GameEngineDevice/Source/W3DDevice/GameClient/WorldHeightMap.cpp
    Common/GameEngineDevice/Source/W3DDevice/GameClient/HeightMap.cpp
    Common/GameEngineDevice/Source/W3DDevice/GameClient/BaseHeightMap.cpp
    Common/GameEngineDevice/Source/W3DDevice/GameClient/FlatHeightMap.cpp
    Common/GameEngineDevice/Source/W3DDevice/GameClient/TileData.cpp)

list(TRANSFORM SAGE_STUB_UNCOMMONIZED_SOURCES PREPEND "${CMAKE_SOURCE_DIR}/")
set_source_files_properties(
  ${SAGE_STUB_UNCOMMONIZED_SOURCES}
  TARGET_DIRECTORY c_gameenginestub
  PROPERTIES HEADER_FILE_ONLY ON)
