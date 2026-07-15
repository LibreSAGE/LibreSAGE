/*
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

// Tool.h
// Tool classes for worldbuilder.
// Author: John Ahlquist, April 2001

#pragma once

#ifndef TOOL_H
#define TOOL_H

#include <QCursor>
#include <QPoint>

#include "Lib/BaseType.h"
#include "Common/STLTypedefs.h"

enum TTrackingMode {
	TRACK_NONE,
	TRACK_L,
	TRACK_M,
	TRACK_R
};

// The tool ids used to be the MFC toolbar button resource ids; now they are
// just unique identifiers connecting tools to their palette actions.
enum ToolID {
	ID_NO_TOOL = 0,
	ID_BRUSH_TOOL,
	ID_TILE_TOOL,
	ID_BIG_TILE_TOOL,
	ID_FEATHER_TOOL,
	ID_AUTO_EDGE_OUT_TOOL,
	ID_FLOOD_FILL_TOOL,
	ID_MOUND_TOOL,
	ID_DIG_TOOL,
	ID_EYEDROPPER_TOOL,
	ID_OBJECT_TOOL,
	ID_POINTER_TOOL,
	ID_BLEND_EDGE_TOOL,
	ID_GROVE_TOOL,
	ID_MESH_MOLD_TOOL,
	ID_ROAD_TOOL,
	ID_HAND_SCROLL_TOOL,
	ID_WAYPOINT_TOOL,
	ID_POLYGON_TOOL,
	ID_BUILD_LIST_TOOL,
	ID_FENCE_TOOL,
	ID_WATER_TOOL,
	ID_RAMP_TOOL,
	ID_SCORCH_TOOL,
	ID_BORDER_TOOL,
	ID_RULER_TOOL
};

#include <vector>
typedef std::vector<QPoint> VecHeightMapIndexes;

#define MAGIC_GROUND_Z (0)

//#define IS_MAGIC_GROUND(z) ((z)==MAGIC_GROUND_Z)

// for backwards compatibility with existing maps:
#define IS_MAGIC_GROUND(z) ((z)==0)

class CWorldBuilderDoc;
class CWorldBuilderView;
class WorldHeightMapEdit;
class WbView;

/*************************************************************************
**                             Tool
***************************************************************************/
class Tool
{
protected:
	Int	m_toolID;  //< Tool id (see ToolID).
	QCursor m_cursor;  //< The tool's mouse cursor.

	Int m_prevXIndex;
	Int m_prevYIndex;
public:
	Tool(Int toolID, const char *cursorResource);
	virtual ~Tool(void);

public:
	Int getToolID(void) {return m_toolID;}
	virtual QCursor getCursor(void) {return m_cursor;}

	virtual void activate(); ///< Become the current tool.
	virtual void deactivate(){}; ///< Become not the current tool.

	virtual Bool followsTerrain(void) {return true;};	 ///< True if the tool tracks the terrain, generally false if it modifies the terrain heights.

	virtual void mouseMoved(TTrackingMode m, QPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc) {}
	virtual void mouseDown(TTrackingMode m, QPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc) {}
	virtual void mouseUp(TTrackingMode m, QPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc) {}
	virtual WorldHeightMapEdit *getHeightMap(void) {return NULL;}

	static Real calcRoundBlendFactor(QPoint center, Int x, Int y, Int brushWidth, Int featherWidth);
	static Real calcSquareBlendFactor(QPoint center, Int x, Int y, Int brushWidth, Int featherWidth);
	static void getCenterIndex(Coord3D *docLocP, Int brushWidth, QPoint *center, CWorldBuilderDoc *pDoc);
	static void getAllIndexesIn(const Coord3D *bl, const Coord3D *br,
															const Coord3D *tl, const Coord3D *tr,
															Int widthOutside, CWorldBuilderDoc *pDoc,
															VecHeightMapIndexes* allIndices);
};


#endif //TOOL_H
