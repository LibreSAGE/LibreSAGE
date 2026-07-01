/*
**	Command & Conquer Generals(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// FILE: SimulatorMain.cpp ////////////////////////////////////////////////////
//
// Entry point for the "simulator" tool: loads a replay file given on the command
// line and runs the game simulation on it, then exits when the replay finishes.
//
// It reuses the existing game engine (g_gameengine) and device layer
// (g_gameenginedevice) - it constructs the real SDL3GameEngine, just like the
// game does, and feeds the replay in through the standard "-file" mechanism.
// The only behavioural difference vs. the game is that it auto-quits once
// playback completes, so it can be driven from scripts.
//
// Usage:  simulator <replay.rep> [extra engine args...]
///////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#endif
// Resolves to the real Windows header on Win32, or the Common/Compat shim on
// other platforms - either way it provides HWND / HINSTANCE / DWORD.
#include <windows.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#include <SDL3/SDL.h>
#include <SDL3_net/SDL_net.h>

#include "Lib/BaseType.h"
#include "Common/CriticalSection.h"
#include "Common/GlobalData.h"
#include "Common/GameEngine.h"
#include "Common/Debug.h"
#include "Common/GameMemory.h"
#include "Common/MessageStream.h"
#include "Common/Recorder.h"
#include "Common/Version.h"
#include "GameClient/Mouse.h"
#include "GameLogic/GameLogic.h"
#include "SDL3Device/Common/SDL3GameEngine.h"
#include "SDL3Device/GameClient/SDL3Mouse.h"
#include "buildVersion.h"
#include "generatedVersion.h"

// GLOBALS ////////////////////////////////////////////////////////////////////
// These globals are normally provided by the game's main translation unit
// (SDL3Main.cpp). Since the simulator has its own entry point and does not link
// that file, it must provide them itself.
HINSTANCE ApplicationHInstance = NULL;
HWND ApplicationHWnd = NULL;
Bool ApplicationIsWindowed = true;                ///< run windowed; never grab fullscreen
Bool CheckForMultipleInstances = false;           ///< allow running alongside the game
SDL3Mouse *TheWin32Mouse = NULL;
DWORD TheMessageTime = 0;
SDL_Window *ApplicationWindow = NULL;

const Char *g_strFile = "data\\Generals.str";
const Char *g_csfFile = "data\\%s\\Generals.csf";
const char *gAppPrefix = "sim_"; /// distinct debug-log prefix so we don't clobber the game's

#define DEFAULT_XRESOLUTION 800
#define DEFAULT_YRESOLUTION 600

// Necessary to allow memory managers and such to have useful critical sections
static CriticalSection critSec1, critSec2, critSec3, critSec4, critSec5;

//-----------------------------------------------------------------------------
// SimulationGameEngine
//
// The real SDL3GameEngine with one addition: it watches the replay and quits
// the engine loop once playback has finished, so the tool terminates instead of
// dropping back to the shell. Everything else (logic, client, device layer) is
// exactly what the game uses - no stub device layer.
//-----------------------------------------------------------------------------
class SimulationGameEngine : public SDL3GameEngine
{
public:
	SimulationGameEngine() : m_checkedStart(false), m_sawReplay(false), m_lastReported(0) {}

	virtual void update(void) override
	{
		SDL3GameEngine::update();

		// One-shot check on the first update: if playback never engaged (e.g. the
		// replay failed to load), there is nothing to simulate - bail out.
		if (!m_checkedStart)
		{
			m_checkedStart = true;
			if (TheRecorder == NULL || TheRecorder->getMode() != RECORDERMODETYPE_PLAYBACK)
			{
				fprintf(stderr, "simulator: no replay playback is active - quitting.\n");
				setQuitting(TRUE);
				return;
			}
		}

		if (TheGameLogic != NULL && TheGameLogic->isInReplayGame())
		{
			m_sawReplay = true;
			UnsignedInt frame = TheGameLogic->getFrame();
			if (frame >= m_lastReported + REPORT_INTERVAL)
			{
				m_lastReported = frame;
				fprintf(stdout, "simulator: simulating frame %u\n", frame);
				fflush(stdout);
			}
		}
		else if (m_sawReplay)
		{
			// We were in the replay game and now we are not - playback finished.
			fprintf(stdout, "simulator: replay finished at frame %u\n",
				TheGameLogic ? TheGameLogic->getFrame() : 0);
			fflush(stdout);
			setQuitting(TRUE);
		}
	}

private:
	static const UnsignedInt REPORT_INTERVAL = 100;
	Bool m_checkedStart;
	Bool m_sawReplay;
	UnsignedInt m_lastReported;
};

//-----------------------------------------------------------------------------
// The engine factory called by GameMain(). Returns our simulation engine, which
// is just the device-layer SDL3GameEngine plus replay-end detection.
//-----------------------------------------------------------------------------
GameEngine *CreateGameEngine(void)
{
	SimulationGameEngine *engine = NEW SimulationGameEngine;
	engine->setIsActive(true);
	return engine;
}

//-----------------------------------------------------------------------------
static void setWorkingDirectoryToExecutable(void)
{
	const char *basePath = SDL_GetBasePath();
	if (basePath == NULL)
		return;
#ifdef _WIN32
	SetCurrentDirectoryA(basePath);
#else
	chdir(basePath);
#endif
}

//-----------------------------------------------------------------------------
static Bool createApplicationWindow(void)
{
	// W3DDisplay (the reused device-layer display) needs a window/surface, so we
	// create one even though we are only interested in the simulation.
	SDL_WindowFlags flags = SDL_WINDOW_VULKAN | SDL_WINDOW_HIDDEN;
	ApplicationWindow = SDL_CreateWindow("Generals Simulator",
		DEFAULT_XRESOLUTION, DEFAULT_YRESOLUTION, flags);
	if (ApplicationWindow == NULL)
	{
		fprintf(stderr, "simulator: failed to create SDL window: %s\n", SDL_GetError());
		return false;
	}
	ApplicationHWnd = reinterpret_cast<HWND>(ApplicationWindow);
	return true;
}

//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
	// Find the replay file argument (first non-flag argument, or one ending .rep).
	const char *replayPath = NULL;
	for (int i = 1; i < argc; ++i)
	{
		if (argv[i][0] == '-')
			continue;
		replayPath = argv[i];
		break;
	}

	if (replayPath == NULL)
	{
		fprintf(stderr, "usage: %s <replay.rep> [engine args...]\n", argv[0]);
		return 1;
	}

#ifdef _WIN32
	ApplicationHInstance = GetModuleHandleA(NULL);
#endif

	setenv("DXVK_WSI_DRIVER", "SDL3", 1);

	TheAsciiStringCriticalSection = &critSec1;
	TheUnicodeStringCriticalSection = &critSec2;
	TheDmaCriticalSection = &critSec3;
	TheMemoryPoolCriticalSection = &critSec4;
	TheDebugLogCriticalSection = &critSec5;

	setWorkingDirectoryToExecutable();

	// Honor -dir <path> so data can be located, mirroring the game.
	for (int i = 1; i < argc; ++i)
	{
		if (SDL_strcasecmp(argv[i], "-dir") == 0 && i + 1 < argc)
		{
#ifdef _WIN32
			SetCurrentDirectoryA(argv[i + 1]);
#else
			chdir(argv[i + 1]);
#endif
			++i;
		}
	}

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == 0)
	{
		fprintf(stderr, "simulator: SDL failed to initialize: %s\n", SDL_GetError());
		return 1;
	}
	if (NET_Init() == false)
	{
		fprintf(stderr, "simulator: failed to initialize SDL_net: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}
	if (createApplicationWindow() == false)
	{
		SDL_Quit();
		return 1;
	}

	DEBUG_INIT(DEBUG_FLAGS_DEFAULT);
	initMemoryManager();

	TheVersion = NEW Version;
	TheVersion->setVersion(VERSION_MAJOR, VERSION_MINOR, VERSION_BUILDNUM, VERSION_LOCALBUILDNUM,
		AsciiString(VERSION_BUILDUSER), AsciiString(VERSION_BUILDLOC),
		AsciiString(__TIME__), AsciiString(__DATE__));

	// Build the argument list handed to the engine: force the replay in via the
	// standard "-file" path (the same path the game uses for command-line replays),
	// and forward any extra args the caller supplied.
	std::vector<char *> engineArgs;
	engineArgs.push_back(argv[0]);
	engineArgs.push_back(const_cast<char *>("-file"));
	engineArgs.push_back(const_cast<char *>(replayPath));
	for (int i = 1; i < argc; ++i)
	{
		if (argv[i] == replayPath)
			continue; // already passed via -file
		engineArgs.push_back(argv[i]);
	}

	// Run the engine. GameMain() creates the engine via CreateGameEngine() (our
	// SimulationGameEngine), inits it with our args, and runs until it quits -
	// which our engine does as soon as the replay finishes.
	GameMain(static_cast<int>(engineArgs.size()), engineArgs.data());

	delete TheVersion;
	TheVersion = NULL;

	shutdownMemoryManager();
	DEBUG_SHUTDOWN();

	if (ApplicationWindow != NULL)
	{
		SDL_DestroyWindow(ApplicationWindow);
		ApplicationWindow = NULL;
	}
	NET_Quit();
	SDL_Quit();

	TheAsciiStringCriticalSection = NULL;
	TheUnicodeStringCriticalSection = NULL;
	TheDmaCriticalSection = NULL;
	TheMemoryPoolCriticalSection = NULL;
	TheDebugLogCriticalSection = NULL;

	return 0;
}
