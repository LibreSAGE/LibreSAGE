/*
**	Command & Conquer Generals(tm)
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**  Copyright 2026 Stephan Vedder
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

#pragma once

// Cross platform symbol visibility.  On Windows the symbols are exported from the
// DLL, on every other platform we make them publicly visible in the shared object.
#if defined(_WIN32)
#	define DEBUGWINDOW_EXPORT __declspec(dllexport)
#else
#	define DEBUGWINDOW_EXPORT __attribute__((visibility("default")))
#endif

// Declared extern C to prevent name mangling, which makes life very unhappy
extern "C" {
	// Called to create the dialog
	void DEBUGWINDOW_EXPORT CreateDebugDialog(void);

	// Called to (not surprisingly) destroy the dialog (and free the resources)
	void DEBUGWINDOW_EXPORT DestroyDebugDialog(void);

	// Call this each frame to determine whether to continue or not
	bool DEBUGWINDOW_EXPORT CanAppContinue(void);

	// Call this to force the app to continue. (Unpause if necessary.)
	void DEBUGWINDOW_EXPORT ForceAppContinue(void);

	// Call this to tell the app to run really fast
	bool DEBUGWINDOW_EXPORT RunAppFast(void);

	// Call this to add a message to the script window
	void DEBUGWINDOW_EXPORT AppendMessage(const char* messageToPass);

	// Call this to set the frame number of the app
	void DEBUGWINDOW_EXPORT SetFrameNumber(int frameNumber);

	// Call this to add a message, and simulate pressing Pause immediately after
	void DEBUGWINDOW_EXPORT AppendMessageAndPause(const char* messageToPass);

	// Call this to add or update a variable value
	void DEBUGWINDOW_EXPORT AdjustVariable(const char* variable, const char* value);

	// Call this to add or update a variable value, and simulate pressing Pause immediately after
	void DEBUGWINDOW_EXPORT AdjustVariableAndPause(const char* variable, const char* value);
}
