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

// FILE: ParticleEditorExport.h /////////////////////////////////////////////////
//
// Desc:       C export boundary for the particle editor shared library.  The game
//             (ScriptEngine) loads this library at runtime and resolves these
//             symbols by name, so the signatures must remain stable.  The Qt6 port
//             keeps the exact same interface as the original MFC DLL.
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Lib/BaseType.h"

// Cross platform symbol visibility.  On Windows the symbols are exported from the
// DLL, on every other platform we make them publicly visible in the shared object.
#if defined(_WIN32)
#	define PARTED_EXPORT __declspec(dllexport)
#else
#	define PARTED_EXPORT __attribute__((visibility("default")))
#endif

class ParticleSystemTemplate;

// Declared extern C to prevent name mangling, which makes life very unhappy
extern "C" {
	// Called to create the dialog
	void PARTED_EXPORT CreateParticleSystemDialog( void );

	// Called to (not surprisingly) destroy the dialog (and free the resources)
	void PARTED_EXPORT DestroyParticleSystemDialog( void );

	void PARTED_EXPORT RemoveAllParticleSystems( void );
	void PARTED_EXPORT AppendParticleSystem( const char* particleSystemName );
	void PARTED_EXPORT RemoveAllThingTemplates( void );
	void PARTED_EXPORT AppendThingTemplate( const char* thingTemplateName );

	Bool PARTED_EXPORT HasUpdatedSelectedParticleSystem( void );

	void PARTED_EXPORT GetSelectedParticleSystemName( char *bufferToCopyInto );

	void PARTED_EXPORT UpdateCurrentParticleCap( int currentParticleCap );
	void PARTED_EXPORT UpdateCurrentNumParticles( int currentParticleCount );
	int PARTED_EXPORT GetNewParticleCap( void );


#	define PARM_ParticleTypeName		0x00
# define PARM_SlaveSystemName			0x01
#	define PARM_AttachedSystemName	0x02

// Keep this one last
# define PARM_NumParms	0x03
	// parmNum can be exactly one of the above defines (PARM_*)
	void PARTED_EXPORT GetSelectedParticleAsciiStringParm( int parmNum, char *bufferToCopyInto, ParticleSystemTemplate **whichTemplate );
	void PARTED_EXPORT UpdateParticleAsciiStringParm( int parmNum, const char *bufferToCopyFrom, ParticleSystemTemplate **whichTemplate  );


	void PARTED_EXPORT UpdateCurrentParticleSystem( ParticleSystemTemplate *particleTemplate );
	void PARTED_EXPORT UpdateSystemUseParameters( ParticleSystemTemplate *particleTemplate );

	Bool PARTED_EXPORT ShouldWriteINI( void );
	Bool PARTED_EXPORT ShouldBusyWait( void );
	Bool PARTED_EXPORT ShouldUpdateParticleCap( void );
	Bool PARTED_EXPORT ShouldReloadTextures( void );


#	define PEB_Continue								0x00
#	define PEB_UpdateCurrentSystem		0x01
#	define PEB_ChangeToAnotherSystem	0x02
#	define PEB_SaveCurrentSystem			0x03
#	define PEB_SaveAllSystems					0x03
#	define PEB_ReloadCurrentSystem		0x04
#	define PEB_SetParticleCap					0x05
# define PEB_ReloadTextures					0x06
# define PEB_KillAllSystems					0x07
#	define PEB_BusyWait								0xFE
#	define PEB_Error									0xFF


	int PARTED_EXPORT NextParticleEditorBehavior( void );


}
