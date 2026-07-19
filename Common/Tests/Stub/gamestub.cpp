/*
**	Command & Conquer Generals(tm)
**	Command & Conquer Generals Zero Hour(tm)
**
**	Stubs for symbols the Common engine sources reference but do not define.
**
**	In a real build these come from the game's entry point (SDL3Main.cpp) and
**	from the per-game engine library. Common tests exercise only the
**	game-agnostic subsystems -- file access, archives, strings -- so plain
**	definitions are enough to satisfy the linker.
*/

#include <SDL3/SDL.h>

#include "Common/SubsystemInterface.h"

// Normally created by the game's GameEngine.cpp. Tests that need subsystems
// registered can assign to this; the ones that only touch the file systems
// leave it null.
SubsystemInterfaceList *TheSubsystemList = NULL;

/// So WB can have a different debug log file name.
const char *gAppPrefix = "";

SDL_Window *ApplicationWindow = NULL;

// Normally defined in the game's Main entry point; the string tables are
// referenced by GameText.cpp in the engine library.
const char *g_strFile = "data\\Generals.str";
const char *g_csfFile = "data\\%s\\Generals.csf";
