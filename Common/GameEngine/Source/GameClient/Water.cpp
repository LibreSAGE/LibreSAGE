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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: Water.cpp ////////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, December 2001
// Desc:   Map water settings
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/STLTypedefs.h"

#include "GameClient/Water.h"
#include "Common/INI.h"

// GLOBALS ////////////////////////////////////////////////////////////////////////////////////////
WaterSetting WaterSettings[ TIME_OF_DAY_COUNT ];
OVERRIDE<WaterTransparencySetting> TheWaterTransparency = NULL;

// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
const FieldParse WaterSetting::m_waterSettingFieldParseTable[] = 
{

	{ "SkyTexture",									INI::parseAsciiString,			NULL, offsetof( WaterSetting, m_skyTextureFile ) },
	{ "WaterTexture",								INI::parseAsciiString,			NULL, offsetof( WaterSetting, m_waterTextureFile ) },
  { "Vertex00Color",							INI::parseRGBAColorInt,			NULL, offsetof( WaterSetting, m_vertex00Diffuse ) },
	{ "Vertex10Color",							INI::parseRGBAColorInt,			NULL, offsetof( WaterSetting, m_vertex10Diffuse ) },
	{ "Vertex01Color",							INI::parseRGBAColorInt,			NULL, offsetof( WaterSetting, m_vertex01Diffuse ) },
  { "Vertex11Color",							INI::parseRGBAColorInt,			NULL, offsetof( WaterSetting, m_vertex11Diffuse ) },
	{ "DiffuseColor",								INI::parseRGBAColorInt,			NULL, offsetof( WaterSetting, m_waterDiffuseColor ) },
  { "TransparentDiffuseColor",		INI::parseRGBAColorInt,			NULL, offsetof( WaterSetting, m_transparentWaterDiffuse ) },
  { "UScrollPerMS",								INI::parseReal,							NULL, offsetof( WaterSetting, m_uScrollPerMs ) },
  { "VScrollPerMS",								INI::parseReal,							NULL, offsetof( WaterSetting, m_vScrollPerMs ) },
	{ "SkyTexelsPerUnit",						INI::parseReal,							NULL, offsetof( WaterSetting, m_skyTexelsPerUnit ) },
  { "WaterRepeatCount",						INI::parseInt,							NULL, offsetof( WaterSetting, m_waterRepeatCount ) },

	{ NULL,													NULL,												NULL, 0 },

};

// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
const FieldParse WaterTransparencySetting::m_waterTransparencySettingFieldParseTable[] = 
{

	{ "TransparentWaterDepth",			INI::parseReal,				NULL,			offsetof( WaterTransparencySetting, m_transparentWaterDepth ) },
	{ "TransparentWaterMinOpacity",	INI::parseReal,				NULL,			offsetof( WaterTransparencySetting, m_minWaterOpacity ) },
	{ "StandingWaterColor",	INI::parseRGBColor,			NULL,			offsetof( WaterTransparencySetting, m_standingWaterColor ) },
	{ "StandingWaterTexture",INI::parseAsciiString,		NULL,			offsetof( WaterTransparencySetting, m_standingWaterTexture ) },
	{ "AdditiveBlending", INI::parseBool,				NULL,			offsetof( WaterTransparencySetting, m_additiveBlend) },
	{ "RadarWaterColor", INI::parseRGBColor,			NULL,			offsetof( WaterTransparencySetting, m_radarColor) },
	{ "SkyboxTextureN",							INI::parseAsciiString,NULL,			offsetof( WaterTransparencySetting, m_skyboxTextureN ) },
	{ "SkyboxTextureE",							INI::parseAsciiString,NULL,			offsetof( WaterTransparencySetting, m_skyboxTextureE ) },
	{ "SkyboxTextureS",							INI::parseAsciiString,NULL,			offsetof( WaterTransparencySetting, m_skyboxTextureS ) },
	{ "SkyboxTextureW",							INI::parseAsciiString,NULL,			offsetof( WaterTransparencySetting, m_skyboxTextureW ) },
	{ "SkyboxTextureT",							INI::parseAsciiString,NULL,			offsetof( WaterTransparencySetting, m_skyboxTextureT ) },


	{ 0, 0, 0, 0 },
};


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
WaterSetting::WaterSetting( void )
{

	m_skyTextureFile.clear();
	m_waterTextureFile.clear();
	m_waterRepeatCount = 0;
	m_skyTexelsPerUnit = 0.0f;

	m_vertex00Diffuse.red = 0;
	m_vertex00Diffuse.green = 0; 
	m_vertex00Diffuse.blue = 0; 
	m_vertex00Diffuse.alpha = 0;

	m_vertex01Diffuse.red = 0; 
	m_vertex01Diffuse.green = 0; 
	m_vertex01Diffuse.blue = 0; 
	m_vertex01Diffuse.alpha = 0;

	m_vertex10Diffuse.red = 0; 
	m_vertex10Diffuse.green = 0; 
	m_vertex10Diffuse.blue = 0; 
	m_vertex10Diffuse.alpha = 0;

	m_vertex11Diffuse.red = 0; 
	m_vertex11Diffuse.green = 0; 
	m_vertex11Diffuse.blue = 0; 
	m_vertex11Diffuse.alpha = 0;

	m_waterDiffuseColor.red = 0;
	m_waterDiffuseColor.green = 0;
	m_waterDiffuseColor.blue = 0;
	m_waterDiffuseColor.alpha = 0;

	m_transparentWaterDiffuse.red = 0;
	m_transparentWaterDiffuse.green = 0;
	m_transparentWaterDiffuse.blue = 0;
	m_transparentWaterDiffuse.alpha = 0;

	m_uScrollPerMs = 0.0f;
	m_vScrollPerMs = 0.0f;

}  // end WaterSetting

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
WaterSetting::~WaterSetting( void )
{

}  // end WaterSetting


//-------------------------------------------------------------------------------------------------
/** Water setting, note that this does not support override situations.  As the water
	* system becomes more complex we may want to change this */
//-------------------------------------------------------------------------------------------------
void WaterSetting::parse( INI* ini )
{
	AsciiString name;
	WaterSetting *waterSetting = NULL;

	// read the name
	const char* token = ini->getNextToken();

	name.set( token );

	// get the water setting we want to load based on name
	const char **timeOfDayName = TimeOfDayNames;
	Int timeOfDayIndex = 0;  // TIME_OF_DAY_INVALID
	while( timeOfDayName && *timeOfDayName )
	{

		if( stricmp( *timeOfDayName, name.str() ) == 0 )
		{

			waterSetting = &WaterSettings[ timeOfDayIndex ];
			break;

		}  // end if

		// next name
		timeOfDayName++;
		timeOfDayIndex++;

	}  // end while

	// check for no time of day match
	if( waterSetting == NULL )
		throw INI_INVALID_DATA;

	// parse the data
	ini->initFromINI( waterSetting, waterSetting->getFieldParse() );

}  // end parseWaterSetting

//-------------------------------------------------------------------------------------------------
void WaterTransparencySetting::parse( INI *ini )
{
	if (TheWaterTransparency == NULL) {
		TheWaterTransparency = newInstance(WaterTransparencySetting);
	} else if (ini->getLoadType() == INI_LOAD_CREATE_OVERRIDES) {
		WaterTransparencySetting* wt = (WaterTransparencySetting*) (TheWaterTransparency.getNonOverloadedPointer());
		WaterTransparencySetting* wtOverride = newInstance(WaterTransparencySetting);
		*wtOverride = *wt;

		// Mark that it is an override.
		wtOverride->markAsOverride();

		wt->friend_getFinalOverride()->setNextOverride(wtOverride);
	} else {
		throw INI_INVALID_DATA;
	}

	WaterTransparencySetting* waterTrans = (WaterTransparencySetting*) (TheWaterTransparency.getNonOverloadedPointer());
	waterTrans = (WaterTransparencySetting*) (waterTrans->friend_getFinalOverride());
	// parse the data
	ini->initFromINI( waterTrans, TheWaterTransparency->getFieldParse() );
	
	// If we overrode any skybox textures, then call the W3D Water stuff.
	if (ini->getLoadType() == INI_LOAD_CREATE_OVERRIDES) {
		// Check to see if we overrode any skybox textures.
		// If we did, then we need to replace them in the model.
		// Copy/Paste monkeys PLEASE TAKE NOTE. This technique only works for the skybox because we
		// know that there will never be more than one sky box. If you were to use this technique for
		// technicals, for instance, it would make all technicals in the level have the same new
		// texture.

		const WaterTransparencySetting* wtOriginal = TheWaterTransparency.getNonOverloadedPointer();
		OVERRIDE<WaterTransparencySetting> wtOverride = TheWaterTransparency;

		if (wtOriginal == wtOverride) 
			return;

		// We don't support this path yet, throw an assert
		DEBUG_CRASH(("Not supported currently"));

		const AsciiString *oldTextures[5],*newTextures[5];

		//Copy current texture names into arrays
		oldTextures[0]=&wtOriginal->m_skyboxTextureN;
		newTextures[0]=&wtOverride->m_skyboxTextureN;
		oldTextures[1]=&wtOriginal->m_skyboxTextureE;
		newTextures[1]=&wtOverride->m_skyboxTextureE;
		oldTextures[2]=&wtOriginal->m_skyboxTextureS;
		newTextures[2]=&wtOverride->m_skyboxTextureS;
		oldTextures[3]=&wtOriginal->m_skyboxTextureW;
		newTextures[3]=&wtOverride->m_skyboxTextureW;
		oldTextures[4]=&wtOriginal->m_skyboxTextureT;
		newTextures[4]=&wtOverride->m_skyboxTextureT;

		//TheTerrainVisual->replaceSkyboxTextures(oldTextures, newTextures);
	}
}