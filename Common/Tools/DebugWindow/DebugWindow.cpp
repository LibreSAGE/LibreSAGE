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

// DebugWindow.cpp : Defines the initialization routines for the DLL.
//

#include "DebugWindowDialog.h"
#include "DebugWindowExport.h"

#include <QApplication>

// The one and only debug window.  Created on demand by the host.
static DebugWindowDialog *theDialog = NULL;

// The debug window lives inside a host process that is not necessarily a Qt
// application, so make sure a QApplication exists before we create any widgets.
static void ensureApplication( void )
{
	if (QApplication::instance() == NULL) {
		static int s_argc = 1;
		static char s_arg0[] = "DebugWindow";
		static char *s_argv[] = { s_arg0, NULL };
		// Intentionally leaked: it must outlive every widget for the whole
		// lifetime of the host process.
		new QApplication( s_argc, s_argv );
	}
}

extern "C" {

void CreateDebugDialog( void )
{
	try {
		ensureApplication();
		if (!theDialog) {
			theDialog = new DebugWindowDialog;
			theDialog->show();
		}
	} catch (...) { }
}

void DestroyDebugDialog( void )
{
	try {
		delete theDialog;
		theDialog = NULL;
	} catch (...) { }
}

bool CanAppContinue( void )
{
	try {
		// Pump the Qt event loop so the window stays responsive while the host
		// drives us one call per frame.
		if (QApplication::instance()) {
			QApplication::processEvents();
		}

		if (!theDialog) {
			return true;
		}

		return theDialog->CanProceed();
	} catch (...) { }

	return true;
}

void ForceAppContinue( void )
{
	try {
		if (theDialog) {
			theDialog->ForceContinue();
		}
	} catch (...) { }
}

bool RunAppFast( void )
{
	try {
		if (theDialog) {
			return theDialog->RunAppFast();
		}
	} catch (...) { }

	return false;
}

void AppendMessage( const char* messageToPass )
{
	try {
		if (theDialog && messageToPass) {
			theDialog->AppendMessage( messageToPass );
		}
	} catch (...) { }
}

void SetFrameNumber( int frameNumber )
{
	try {
		if (theDialog) {
			theDialog->SetFrameNumber( frameNumber );
		}
	} catch (...) { }
}

void AppendMessageAndPause( const char* messageToPass )
{
	try {
		if (theDialog && messageToPass) {
			theDialog->AppendMessage( messageToPass );
			theDialog->ForcePause();
		}
	} catch (...) { }
}

void AdjustVariable( const char* variable, const char* value )
{
	try {
		if (theDialog && variable && value) {
			theDialog->AdjustVariable( variable, value );
		}
	} catch (...) { }
}

void AdjustVariableAndPause( const char* variable, const char* value )
{
	try {
		if (theDialog && variable && value) {
			theDialog->AdjustVariable( variable, value );
			theDialog->ForcePause();
		}
	} catch (...) { }
}

} // extern "C"
