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

// RoadTool.cpp
// Road drawing tool for worldbuilder.
// Author: John Ahlquist, April 2001
//
// Qt6 port note: only the snap helper is ported so far; the interactive road
// drawing comes back with the object tools.  @todo

#include "RoadTool.h"

#include "Common/MapObject.h"

#include "vector2.h"

/// Constructor
RoadTool::RoadTool(void) :
	Tool(ID_ROAD_TOOL, ":/cursors/IDC_ROAD.cur")
{
}

/// Destructor
RoadTool::~RoadTool(void)
{
}

//=============================================================================
/** Snaps to nearby road segment end points. */
//=============================================================================
Bool RoadTool::snap(Coord3D *pLoc, Bool skipFirst)
{
	MapObject *pMapObj;
	MapObject *pMapObj2;
	Real snapDist = ROAD_SNAP_DISTANCE*MAP_XY_FACTOR;
	Coord3D newLoc = *pLoc;
	Bool snapped = false;

	for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {
		if (skipFirst) {
			skipFirst = false;
			continue;
		}
		if (pMapObj->getFlag(FLAG_ROAD_POINT1)) {
			pMapObj2 = pMapObj->getNext();
			if (pMapObj2==NULL) break;
			if (!pMapObj2->getFlag(FLAG_ROAD_POINT2)) continue;
			Vector2 dist;
			if (!pMapObj->isSelected()) {
				dist.Set(pMapObj->getLocation()->x - pLoc->x, pMapObj->getLocation()->y - pLoc->y);
				if (dist.Length() < snapDist) {
					newLoc = *pMapObj->getLocation();
					snapDist = dist.Length();
					snapped = true;
				}
			}
			if (!pMapObj2->isSelected()) {
				dist.Set(pMapObj2->getLocation()->x - pLoc->x, pMapObj2->getLocation()->y - pLoc->y);
				if (dist.Length() < snapDist) {
					newLoc = *pMapObj2->getLocation();
					snapDist = dist.Length();
					snapped = true;
				}
			}
		}
	}
	newLoc.z = MAGIC_GROUND_Z; // roads always snap to terrain.
	if (snapped) {
		*pLoc = newLoc;
	}
	return snapped;
}
