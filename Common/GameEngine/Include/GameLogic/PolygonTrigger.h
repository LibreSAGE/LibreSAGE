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


// PolygonTrigger.h
// Class to encapsulate polygon triggers for maps.
// Note - Polygons are used for two reasons - one is area triggers for 
// scripts, so units can be tested for entering or exiting areas, and 
// second to specify areas that are filled with water in the map.
// See the m_isWaterArea to differentiate.
// Author: John Ahlquist, November 2001

#pragma once

#ifndef PolygonTrigger_H
#define PolygonTrigger_H

#include "Common/GameMemory.h"
#include "Common/Snapshot.h"
#include "Common/STLTypedefs.h"

class DataChunkInput;
class DataChunkOutput;
struct DataChunkInfo;
class PolygonTrigger;
class Xfer;

// ------------------------------------------------------------------------------------------------
/** Water handles are used to represent instances of areas of water, no matter which type
	* of implementation the water is (grid, trigger area, etc) */
// ------------------------------------------------------------------------------------------------
class WaterHandle
{

public:

	WaterHandle( void ) { m_polygon = NULL; }

	///@todo we need to formalize the water systems 
	PolygonTrigger *m_polygon;	///< valid when water is a polygon area, NULL if water is a grid

};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class PolygonTrigger : public MemoryPoolObject,
											 public Snapshot
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(PolygonTrigger, "PolygonTrigger")		

protected:
	PolygonTrigger*		m_nextPolygonTrigger;		///< linked list.
	AsciiString				m_triggerName;		///< The name of this polygon area.
	Int								m_triggerID; ///< Unique int id for the trigger.
	WaterHandle				m_waterHandle;		///< handle to use this polygon as a water table
	ICoord3D*					m_points;	///< Points that are the polygon.
	Int								m_numPoints; ///< Num points in m_points.
	Int								m_sizePoints; ///< Space allocated for m_points.
	mutable IRegion2D	m_bounds;  ///< 2D bounding box for quick checks.	
	mutable Real			m_radius;
	Int								m_riverStart;	///< Identifies the start point of the river.
	mutable Bool			m_boundsNeedsUpdate;
	Bool							m_exportWithScripts;
	Bool							m_isWaterArea; ///< Used to specify water areas in the map.
	Bool							m_isRiver;		///< Used to specify that a water area is a river.
	AsciiString			m_layerName;  ///< Used to specify the layer in the World Builder.
	Bool				m_shouldRender;
	Bool				m_selected;

	// BFME+ river rendering properties (see K_TRIGGERS_VERSION_5).
	AsciiString			m_riverTexture;
	AsciiString			m_noiseTexture;
	AsciiString			m_alphaEdgeTexture;
	AsciiString			m_sparkleTexture;
	AsciiString			m_bumpMapTexture;
	AsciiString			m_skyTexture;
	Bool				m_useAdditiveBlending;
	RGBColor			m_riverColor;
	Coord2D				m_uvScrollSpeed;
	Real				m_riverAlpha;

	static PolygonTrigger* ThePolygonTriggerListPtr;
	static Int s_currentID; ///< Current id for new triggers.

protected:
	void reallocate(void);
	void updateBounds(void) const;

	// snapshot methods
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

public:
	PolygonTrigger(Int initialAllocation);
	//~PolygonTrigger(void);		///< Note that deleting the head of a list deletes all linked objects in the list.

public:
	static PolygonTrigger *getFirstPolygonTrigger(void) {return ThePolygonTriggerListPtr;}
	static PolygonTrigger *getPolygonTriggerByID(Int triggerID);
	static Bool ParsePolygonTriggersDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData);
	/// Writes Triggers Info
	static void WritePolygonTriggersDataChunk(DataChunkOutput &chunkWriter);
	static void deleteTriggers(void);

public:
	static void addPolygonTrigger(PolygonTrigger *pTrigger);
	static void removePolygonTrigger(PolygonTrigger *pTrigger);
	void setNextPoly(PolygonTrigger *nextPoly) {m_nextPolygonTrigger = nextPoly;} ///< Link the next map object.
	void addPoint(const ICoord3D &point);
	void setPoint(const ICoord3D &point, Int ndx);
	void insertPoint(const ICoord3D &point, Int ndx);
	void deletePoint(Int ndx);
	void setTriggerName(AsciiString name) {m_triggerName = name;};

	void setLayerName(AsciiString name) {m_layerName = name;};
	AsciiString getLayerName(void)  const {return m_layerName;} 

	void setShouldRender(Bool toggle) {m_shouldRender = toggle;}
	Bool getShouldRender() {return m_shouldRender;}

	void setSelected(Bool toggle) {m_selected = toggle;}
	Bool getSelected() {return m_selected;}

	void getCenterPoint(Coord3D* pOutCoord) const;
	Real getRadius(void) const;

public:
	const ICoord3D *getPoint(Int ndx) const {if (ndx<0) ndx=0; if (ndx>=m_numPoints) ndx=m_numPoints-1; return m_points+ndx;} ///< Get a point.
	Int getNumPoints(void) const {return m_numPoints;}
	Int getID(void) const {return m_triggerID;}
	PolygonTrigger *getNext(void) {return m_nextPolygonTrigger;}
	const PolygonTrigger *getNext(void) const {return m_nextPolygonTrigger;}
	AsciiString getTriggerName(void)  const {return m_triggerName;} ///< Gets the trigger name.
	Bool pointInTrigger(ICoord3D &point) const;
	Bool doExportWithScripts(void) const {return m_exportWithScripts;} 
	void setDoExportWithScripts(Bool val) {m_exportWithScripts = val;} 
	Bool isWaterArea(void) const {return m_isWaterArea;} 
	void setWaterArea(Bool val) {m_isWaterArea = val;} 
	Bool isRiver(void) const {return m_isRiver;} 
	void setRiver(Bool val) {m_isRiver = val;} 
	Int getRiverStart(void) const {return m_riverStart;}
	void setRiverStart(Int val) {m_riverStart = val;}

	AsciiString getRiverTexture(void) const {return m_riverTexture;}
	void setRiverTexture(AsciiString val) {m_riverTexture = val;}
	AsciiString getNoiseTexture(void) const {return m_noiseTexture;}
	void setNoiseTexture(AsciiString val) {m_noiseTexture = val;}
	AsciiString getAlphaEdgeTexture(void) const {return m_alphaEdgeTexture;}
	void setAlphaEdgeTexture(AsciiString val) {m_alphaEdgeTexture = val;}
	AsciiString getSparkleTexture(void) const {return m_sparkleTexture;}
	void setSparkleTexture(AsciiString val) {m_sparkleTexture = val;}
	AsciiString getBumpMapTexture(void) const {return m_bumpMapTexture;}
	void setBumpMapTexture(AsciiString val) {m_bumpMapTexture = val;}
	AsciiString getSkyTexture(void) const {return m_skyTexture;}
	void setSkyTexture(AsciiString val) {m_skyTexture = val;}
	Bool getUseAdditiveBlending(void) const {return m_useAdditiveBlending;}
	void setUseAdditiveBlending(Bool val) {m_useAdditiveBlending = val;}
	const RGBColor& getRiverColor(void) const {return m_riverColor;}
	void setRiverColor(const RGBColor &val) {m_riverColor = val;}
	const Coord2D& getUVScrollSpeed(void) const {return m_uvScrollSpeed;}
	void setUVScrollSpeed(const Coord2D &val) {m_uvScrollSpeed = val;}
	Real getRiverAlpha(void) const {return m_riverAlpha;}
	void setRiverAlpha(Real val) {m_riverAlpha = val;}

	const WaterHandle* getWaterHandle(void) const;
	Bool isValid(void) const;
};

#endif
