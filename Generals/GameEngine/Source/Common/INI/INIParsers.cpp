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

// FILE: INIParsers.cpp /////////////////////////////////////////////////////////////////////////////
// Desc:   Freestanding INI field parsers moved out of the commonized INI reader; see INIParsers.h.
///////////////////////////////////////////////////////////////////////////////////////////////////

#define DEFINE_DEATH_NAMES		// for TheDeathNames[], used by parseDeathTypeFlags

#include "Common/INIParsers.h"
#include "Common/INI.h"
#include "Common/AudioEventRTS.h"
#include "Common/GameAudio.h"
#include "GameClient/ClientRandomValue.h"
#include "GameLogic/Damage.h"
#include "GameClient/Water.h"

//-------------------------------------------------------------------------------------------------
/** Parse a dynamic audio event and assign to the 'DynamicAudioEventRTS*' at store */
//-------------------------------------------------------------------------------------------------
/*static*/ void INIParsers::parseDynamicAudioEventRTS( INI *ini, void * /*instance*/, void *store, const void* userData )
{
	const char *token = ini->getNextToken();
	DynamicAudioEventRTS** theSound = (DynamicAudioEventRTS**)store;

	// translate the string into a sound
	if (stricmp(token, "NoSound") == 0)
	{
		if (*theSound)
		{
			(*theSound)->deleteInstance();
			*theSound = NULL;
		}
	}
	else
	{
		if (*theSound == NULL)
			*theSound = newInstance(DynamicAudioEventRTS);
		(*theSound)->m_event.setEventName(AsciiString(token));
	}

	if (*theSound)
		TheAudio->getInfoForAudioEvent(&(*theSound)->m_event);
}

//-------------------------------------------------------------------------------------------------
/** Parse an audio event and assign to the 'AudioEventRTS*' at store */
//-------------------------------------------------------------------------------------------------
/*static*/ void INIParsers::parseAudioEventRTS( INI *ini, void * /*instance*/, void *store, const void* userData )
{
	const char *token = ini->getNextToken();

	AudioEventRTS *theSound = (AudioEventRTS*)store;

	// translate the string into a sound
	if (stricmp(token, "NoSound") != 0) {
		theSound->setEventName(AsciiString(token));
	}

	TheAudio->getInfoForAudioEvent(theSound);
}

//-------------------------------------------------------------------------------------------------
/**
 * Parse a "random variable".
 * The format is "FIELD = low high [distribution]".
 */
/*static*/ void INIParsers::parseGameClientRandomVariable( INI* ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	GameClientRandomVariable *var = static_cast<GameClientRandomVariable *>(store);

	const char* token;

	token = ini->getNextToken();
	Real low = INI::scanReal(token);

	token = ini->getNextToken();
	Real high = INI::scanReal(token);

	// if omitted, assume uniform
	GameClientRandomVariable::DistributionType type = GameClientRandomVariable::UNIFORM;
	token = ini->getNextTokenOrNull();
	if (token)
		type = (GameClientRandomVariable::DistributionType)INI::scanIndexList(token, GameClientRandomVariable::DistributionTypeNames);

	// set the range of the random variable
	var->setRange( low, high, type );
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/*static*/ void INIParsers::parseVeterancyLevelFlags(INI* ini, void* /*instance*/, void* store, const void* /*userData*/)
{
	VeterancyLevelFlags flags = VETERANCY_LEVEL_FLAGS_ALL;
	for (const char* token = ini->getNextToken(); token; token = ini->getNextTokenOrNull())
	{
		if (stricmp(token, "ALL") == 0)
		{
			flags = VETERANCY_LEVEL_FLAGS_ALL;
			continue;
		}
		else if (stricmp(token, "NONE") == 0)
		{
			flags = VETERANCY_LEVEL_FLAGS_NONE;
			continue;
		}
		else if (token[0] == '+')
		{
			VeterancyLevel dt = (VeterancyLevel)INI::scanIndexList(token+1, TheVeterancyNames);
			flags = setVeterancyLevelFlag(flags, dt);
			continue;
		}
		else if (token[0] == '-')
		{
			VeterancyLevel dt = (VeterancyLevel)INI::scanIndexList(token+1, TheVeterancyNames);
			flags = clearVeterancyLevelFlag(flags, dt);
			continue;
		}
		else
		{
			throw INI_UNKNOWN_TOKEN;
		}
	}
	*(VeterancyLevelFlags*)store = flags;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/*static*/ void INIParsers::parseDamageTypeFlags(INI* ini, void* /*instance*/, void* store, const void* /*userData*/)
{
	DamageTypeFlags flags = DAMAGE_TYPE_FLAGS_NONE;
	flags.flip();

	for (const char* token = ini->getNextToken(); token; token = ini->getNextTokenOrNull())
	{
		if (stricmp(token, "ALL") == 0)
		{
			flags = DAMAGE_TYPE_FLAGS_NONE;
			flags.flip();
			continue;
		}
		if (stricmp(token, "NONE") == 0)
		{
			flags = DAMAGE_TYPE_FLAGS_NONE;
			continue;
		}
		if (token[0] == '+')
		{
			DamageType dt = (DamageType)DamageTypeFlags::getSingleBitFromName(token+1);
			flags = setDamageTypeFlag(flags, dt);
			continue;
		}
		if (token[0] == '-')
		{
			DamageType dt = (DamageType)DamageTypeFlags::getSingleBitFromName(token+1);
			flags = clearDamageTypeFlag(flags, dt);
			continue;
		}
		throw INI_UNKNOWN_TOKEN;
	}
	*(DamageTypeFlags*)store = flags;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/*static*/ void INIParsers::parseDeathTypeFlags(INI* ini, void* /*instance*/, void* store, const void* /*userData*/)
{
	DeathTypeFlags flags = DEATH_TYPE_FLAGS_ALL;
	for (const char* token = ini->getNextToken(); token; token = ini->getNextTokenOrNull())
	{
		if (stricmp(token, "ALL") == 0)
		{
			flags = DEATH_TYPE_FLAGS_ALL;
			continue;
		}
		if (stricmp(token, "NONE") == 0)
		{
			flags = DEATH_TYPE_FLAGS_NONE;
			continue;
		}
		if (token[0] == '+')
		{
			DeathType dt = (DeathType)INI::scanIndexList(token+1, TheDeathNames);
			flags = setDeathTypeFlag(flags, dt);
			continue;
		}
		if (token[0] == '-')
		{
			DeathType dt = (DeathType)INI::scanIndexList(token+1, TheDeathNames);
			flags = clearDeathTypeFlag(flags, dt);
			continue;
		}
		throw INI_UNKNOWN_TOKEN;
	}
	*(DeathTypeFlags*)store = flags;
}

//-------------------------------------------------------------------------------------------------
void registerINIBlockParsers( void )
{
	INI::registerBlockParse( "Credits",							INI::parseCredits );
	INI::registerBlockParse( "WindowTransition",				INI::parseWindowTransitions );
	INI::registerBlockParse( "DrawGroupInfo",					INI::parseDrawGroupNumberDefinition );
	INI::registerBlockParse( "EvaEvent",						INI::parseEvaEvent );
	INI::registerBlockParse( "InGameUI",						INI::parseInGameUIDefinition );
	INI::registerBlockParse( "MapCache",						INI::parseMapCacheDefinition );
	INI::registerBlockParse( "MapData",							INI::parseMapDataDefinition );
	INI::registerBlockParse( "MappedImage",						INI::parseMappedImageDefinition );
	INI::registerBlockParse( "Mouse",							INI::parseMouseDefinition );
	INI::registerBlockParse( "MouseCursor",						INI::parseMouseCursorDefinition );
	INI::registerBlockParse( "ShellMenuScheme",					INI::parseShellMenuSchemeDefinition );
	INI::registerBlockParse( "Video",							INI::parseVideoDefinition );
	INI::registerBlockParse( "WaterSet",						WaterSetting::parse );
	INI::registerBlockParse( "WaterTransparency",				WaterTransparencySetting::parse );
	INI::registerBlockParse( "WebpageURL",						INI::parseWebpageURLDefinition );
	INI::registerBlockParse( "HeaderTemplate",					INI::parseHeaderTemplateDefinition );
}
