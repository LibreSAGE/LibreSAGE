/*
**	Command & Conquer Generals Zero Hour(tm)
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

// FILE: INIParsers.h ///////////////////////////////////////////////////////////////////////////////
// Desc:   Freestanding INI field parsers that have no natural owning subsystem class.
//         These live in the per-tree GameEngine (not the commonized INI reader) because
//         they reference per-tree types (audio events, damage/death/veterancy flags,
//         client random variable), keeping Common/INI.cpp free of those headers.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __INIPARSERS_H_
#define __INIPARSERS_H_

class INI;

//-------------------------------------------------------------------------------------------------
class INIParsers
{
public:
	static void parseAudioEventRTS( INI *ini, void *instance, void *store, const void *userData );
	static void parseDynamicAudioEventRTS( INI *ini, void *instance, void *store, const void *userData );
	static void parseGameClientRandomVariable( INI *ini, void *instance, void *store, const void *userData );
	static void parseVeterancyLevelFlags( INI *ini, void *instance, void *store, const void *userData );
	static void parseDamageTypeFlags( INI *ini, void *instance, void *store, const void *userData );
	static void parseDeathTypeFlags( INI *ini, void *instance, void *store, const void *userData );
};

//-------------------------------------------------------------------------------------------------
void registerINIBlockParsers( void );

#endif // __INIPARSERS_H_
