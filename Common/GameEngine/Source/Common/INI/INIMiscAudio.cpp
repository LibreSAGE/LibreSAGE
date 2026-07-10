/*
**	Command & Conquer Generals(tm)
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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

#include "Common/MiscAudio.h"
#include "Common/INI.h"
#include "Common/INIParsers.h"

const FieldParse MiscAudio::m_fieldParseTable[] = 
{ 
	{ "RadarNotifyUnitUnderAttackSound",			INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_radarUnitUnderAttackSound ) },
	{ "RadarNotifyHarvesterUnderAttackSound",	INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_radarHarvesterUnderAttackSound ) },
	{ "RadarNotifyStructureUnderAttackSound", INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_radarStructureUnderAttackSound ) },
	{ "RadarNotifyUnderAttackSound",					INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_radarUnderAttackSound ) },
	{ "RadarNotifyInfiltrationSound",					INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_radarInfiltrationSound ) },
	{ "RadarNotifyOnlineSound",								INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_radarOnlineSound ) },
	{ "RadarNotifyOfflineSound",							INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_radarOfflineSound ) },
	{ "DefectorTimerTickSound",			  				INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_defectorTimerTickSound ) },
	{ "DefectorTimerDingSound",			  				INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_defectorTimerDingSound ) },
	{ "LockonTickSound",			  							INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_lockonTickSound ) },
	{ "AllCheerSound",												INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_allCheerSound )	},
	{ "BattleCrySound",												INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_battleCrySound )	},
	{ "GUIClickSound",												INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_guiClickSound )	},
	{ "NoCanDoSound",													INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_noCanDoSound )	},
	{ "StealthDiscoveredSound",								INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_stealthDiscoveredSound ) },
	{ "StealthNeutralizedSound",							INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_stealthNeutralizedSound ) },
	{ "MoneyDepositSound",										INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_moneyDepositSound ) },
	{ "MoneyWithdrawSound",										INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_moneyWithdrawSound ) },
	{ "BuildingDisabled",											INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_buildingDisabled ) },
	{ "BuildingReenabled",										INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_buildingReenabled ) },
	{ "VehicleDisabled",											INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_vehicleDisabled ) },
	{ "VehicleReenabled",											INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_vehicleReenabled ) },
	{ "SplatterVehiclePilotsBrain",						INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_splatterVehiclePilotsBrain ) },
	{ "TerroristInCarMoveVoice",							INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_terroristInCarMoveVoice ) },
	{ "TerroristInCarAttackVoice",						INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_terroristInCarAttackVoice ) },
	{ "TerroristInCarSelectVoice",						INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_terroristInCarSelectVoice ) },
	{ "CrateHeal",														INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_crateHeal ) },
	{ "CrateShroud",													INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_crateShroud ) },
	{ "CrateSalvage",													INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_crateSalvage ) },
	{ "CrateFreeUnit",												INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_crateFreeUnit ) },
	{ "CrateMoney",														INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_crateMoney ) },
	{ "UnitPromoted",													INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_unitPromoted ) },
	{ "RepairSparks",													INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_repairSparks ) },
	{ "SabotageShutDownBuilding",							INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_sabotageShutDownBuilding ) },
	{ "SabotageResetTimeBuilding",						INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_sabotageResetTimerBuilding ) },
  { "AircraftWheelScreech",									INIParsers::parseAudioEventRTS, NULL, offsetof( MiscAudio, m_aircraftWheelScreech ) },
	
	{ 0, 0, 0, 0 }
};


//-------------------------------------------------------------------------------------------------
void INI::parseMiscAudio( INI *ini )
{
	ini->initFromINI(TheAudio->friend_getMiscAudio(), MiscAudio::m_fieldParseTable);
}