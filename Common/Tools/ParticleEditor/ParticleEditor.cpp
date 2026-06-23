/*
**	Command & Conquer Generals(tm)
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

// FILE: ParticleEditor.cpp /////////////////////////////////////////////////////
//
// Desc:       The C export boundary of the particle editor shared library.  The
//             game (ScriptEngine) loads this library and resolves these symbols
//             by name, driving the editor one call per frame.  This is the Qt6
//             replacement for the original MFC DLL entry points.
//
///////////////////////////////////////////////////////////////////////////////

#include "ParticleEditorDialog.h"
#include "ParticleEditorExport.h"

#include <QApplication>

// The one and only editor window.  Created on demand by the host.
static DebugWindowDialog *theDialog = NULL;

// The editor lives inside a host process that is not necessarily a Qt
// application, so make sure a QApplication exists before we create any widgets.
static void ensureApplication( void )
{
	if (QApplication::instance() == NULL) {
		static int s_argc = 1;
		static char s_arg0[] = "ParticleEditor";
		static char *s_argv[] = { s_arg0, NULL };
		// Intentionally leaked: it must outlive every widget for the whole
		// lifetime of the host process.
		new QApplication( s_argc, s_argv );
	}
}

extern "C" {

void CreateParticleSystemDialog( void )
{
	try {
		ensureApplication();
		if (!theDialog) {
			theDialog = new DebugWindowDialog;
			theDialog->InitPanel();
			theDialog->show();
		}
	} catch (...) { }
}

void DestroyParticleSystemDialog( void )
{
	try {
		delete theDialog;
		theDialog = NULL;
	} catch (...) { }
}

void RemoveAllParticleSystems( void )
{
	try {
		if (theDialog) {
			theDialog->clearAllParticleSystems();
		}
	} catch (...) { }
}

void AppendParticleSystem( const char* particleSystemName )
{
	try {
		if (theDialog) {
			theDialog->addParticleSystem( particleSystemName );
		}
	} catch (...) { }
}

void RemoveAllThingTemplates( void )
{
	try {
		if (theDialog) {
			theDialog->clearAllThingTemplates();
		}
	} catch (...) { }
}

void AppendThingTemplate( const char* thingTemplateName )
{
	try {
		if (theDialog) {
			theDialog->addThingTemplate( thingTemplateName );
		}
	} catch (...) { }
}

Bool HasUpdatedSelectedParticleSystem( void )
{
	try {
		if (theDialog) {
			return theDialog->hasSelectionChanged();
		}
	} catch (...) { }

	return false;
}

void GetSelectedParticleSystemName( char *bufferToCopyInto )
{
	try {
		if (theDialog) {
			theDialog->getSelectedSystemName( bufferToCopyInto );
		}
	} catch (...) { }
}

void UpdateCurrentParticleCap( int currentParticleCap )
{
	try {
		if (theDialog) {
			theDialog->updateCurrentParticleCap( currentParticleCap );
		}
	} catch (...) { }
}

void UpdateCurrentNumParticles( int currentParticleCount )
{
	try {
		if (theDialog) {
			theDialog->updateCurrentNumParticles( currentParticleCount );
		}
	} catch (...) { }
}

int GetNewParticleCap( void )
{
	try {
		if (theDialog) {
			return theDialog->getNewParticleCap();
		}
	} catch (...) { }

	return -1;
}

void GetSelectedParticleAsciiStringParm( int parmNum, char *bufferToCopyInto, ParticleSystemTemplate **whichTemplate )
{
	try {
		if (theDialog) {
			theDialog->getSelectedParticleAsciiStringParm( parmNum, bufferToCopyInto );
			if (whichTemplate) {
				(*whichTemplate) = theDialog->getCurrentParticleSystem();
			}
		}
	} catch (...) { }
}

void UpdateParticleAsciiStringParm( int parmNum, const char *bufferToCopyFrom, ParticleSystemTemplate **whichTemplate )
{
	try {
		if (theDialog) {
			theDialog->updateParticleAsciiStringParm( parmNum, bufferToCopyFrom );
			if (whichTemplate) {
				(*whichTemplate) = theDialog->getCurrentParticleSystem();
			}
		}
	} catch (...) { }
}

void UpdateCurrentParticleSystem( ParticleSystemTemplate *particleTemplate )
{
	try {
		if (theDialog) {
			theDialog->updateCurrentParticleSystem( particleTemplate );
		}
	} catch (...) { }
}

void UpdateSystemUseParameters( ParticleSystemTemplate *particleTemplate )
{
	try {
		if (theDialog) {
			theDialog->updateSystemUseParameters( particleTemplate );
		}
	} catch (...) { }
}

Bool ShouldWriteINI( void )
{
	try {
		if (theDialog) {
			return theDialog->shouldWriteINI();
		}
	} catch (...) { }

	return false;
}

// Not exported in the header, used only by NextParticleEditorBehavior below.
static Bool HasRequestedReload( void )
{
	try {
		if (theDialog) {
			return theDialog->hasRequestedReload();
		}
	} catch (...) { }

	return false;
}

Bool ShouldBusyWait( void )
{
	try {
		if (theDialog) {
			return theDialog->shouldBusyWait();
		}
	} catch (...) { }

	return false;
}

Bool ShouldUpdateParticleCap( void )
{
	try {
		if (theDialog) {
			return theDialog->shouldUpdateParticleCap();
		}
	} catch (...) { }

	return false;
}

Bool ShouldReloadTextures( void )
{
	try {
		if (theDialog) {
			return theDialog->shouldReloadTextures();
		}
	} catch (...) { }

	return false;
}

// Not exported in the header, used only by NextParticleEditorBehavior below.
static Bool HasRequestedKillAllSystems( void )
{
	try {
		if (theDialog) {
			return theDialog->shouldKillAllParticleSystems();
		}
	} catch (...) { }

	return false;
}

int NextParticleEditorBehavior( void )
{
	try {
		// Pump the Qt event loop so the editor stays responsive while the host
		// drives us one call per frame.
		if (QApplication::instance()) {
			QApplication::processEvents();
		}

		if (HasUpdatedSelectedParticleSystem()) {
			return PEB_UpdateCurrentSystem;
		}

		if (ShouldWriteINI()) {
			return PEB_SaveAllSystems;
		}

		if (HasRequestedReload()) {
			return PEB_ReloadCurrentSystem;
		}

		if (ShouldBusyWait()) {
			return PEB_BusyWait;
		}

		if (ShouldUpdateParticleCap()) {
			return PEB_SetParticleCap;
		}

		if (ShouldReloadTextures()) {
			return PEB_ReloadTextures;
		}

		if (HasRequestedKillAllSystems()) {
			return PEB_KillAllSystems;
		}

		return PEB_Continue;
	} catch (...) { }

	return PEB_Error;
}

} // extern "C"
