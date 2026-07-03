find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

qt_add_library(
  c_particleeditor SHARED
  ParticleEditorExport.h
  ParticleEditor.cpp
  ParticleEditorDialog.cpp
  ParticleEditorDialog.h
  ParticleEditorDialog.ui
  ISwapablePanel.cpp
  ISwapablePanel.h
  CButtonShowColor.cpp
  CButtonShowColor.h
  CColorAlphaDialog.cpp
  CColorAlphaDialog.h
  CColorAlphaDialog.ui
  CSwitchesDialog.cpp
  CSwitchesDialog.h
  CSwitchesDialog.ui
  MoreParmsDialog.cpp
  MoreParmsDialog.h
  MoreParmsDialog.ui
  EmissionTypePanels.cpp
  EmissionTypePanels.h
  EmissionPanelPoint.ui
  EmissionPanelLine.ui
  EmissionPanelBox.ui
  EmissionPanelSphere.ui
  EmissionPanelCylinder.ui
  VelocityTypePanels.cpp
  VelocityTypePanels.h
  VelocityPanelOrtho.ui
  VelocityPanelSphere.ui
  VelocityPanelHemisphere.ui
  VelocityPanelCylinder.ui
  VelocityPanelOutward.ui
  ParticleTypePanels.cpp
  ParticleTypePanels.h
  ParticlePanelParticle.ui
  ParticlePanelDrawable.ui
  ParticlePanelStreak.ui)

set_target_properties(c_particleeditor PROPERTIES OUTPUT_NAME "ParticleEditor")

# The editor manipulates ParticleSystemTemplate fields directly, so it compiles
# against the game engine headers (GameClient/ParticleSys.h and friends).  It is
# loaded into the game at runtime and resolves the engine symbols from the host,
# so we only need the headers here -- we deliberately do NOT link the whole
# g_gameengine archive (which keeps this debug DLL lean and decoupled).
target_include_directories(
  c_particleeditor
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
          ${PROJECT_SOURCE_DIR}/Common/GameEngine/Include
          ${PROJECT_SOURCE_DIR}/Generals/GameEngine/Include
          ${PROJECT_SOURCE_DIR}/Generals/GameEngine/Include/Precompiled
          ${PROJECT_SOURCE_DIR}/Common/Libraries/Source/WWVegas
          ${PROJECT_SOURCE_DIR}/Common/Libraries/Source/WWVegas/WWMath
          ${PROJECT_SOURCE_DIR}/Common/Libraries/Source/WWVegas/WWLib
          ${PROJECT_SOURCE_DIR}/Common/Libraries/Source/WWVegas/WWSaveLoad)

# c_library_include is an INTERFACE target exposing Common/Libraries/Include
# (BaseType.h) plus the Win32 compatibility shim (Common/Compat) used by the
# engine headers on non-Windows platforms.
target_link_libraries(c_particleeditor PRIVATE Qt6::Widgets c_library_include)
