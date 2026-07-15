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

// RoadTool.h
// Road drawing tool for worldbuilder.
// Author: John Ahlquist, April 2001
//
// Qt6 port note: only the snap helper is ported so far; the interactive road
// drawing comes back with the object tools.  @todo

#pragma once

#ifndef ROADTOOL_H
#define ROADTOOL_H

#include "Tool.h"

#define ROAD_SNAP_DISTANCE (1.0f)

/*************************************************************************/
/**                             RoadTool
	 Draws roads.
***************************************************************************/
class RoadTool : public Tool
{
public:
	RoadTool(void);
	~RoadTool(void);

public:
	/// Snap a location to nearby road segment endpoints.
	static Bool snap(Coord3D *pLoc, Bool skipFirst);
};

#endif //ROADTOOL_H
