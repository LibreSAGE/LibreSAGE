#include <SDL3/SDL.h>

const char *gAppPrefix = ""; /// So WB can have a different debug log file name.
SDL_Window *ApplicationWindow = NULL;

#include "Common/SubsystemInterface.h"
// Subsystems
// ----------------------------------------------------------------------------
SubsystemInterfaceList* TheSubsystemList = NULL;
