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

// PointerTool.h
// Select and move tool for worldbuilder.
// Author: John Ahlquist, April 2001
//
// Qt6 port note: the polygon-trigger editing branch (PolygonTool base) is not
// ported yet; selection, drag-select, move and rotate work.  @todo

#pragma once

#ifndef POINTER_TOOL_H
#define POINTER_TOOL_H

#include "Tool.h"
#include "Common/MapObject.h"

class ModifyObjectUndoable;
class WorldHeightMapEdit;

/*************************************************************************/
/**                             PointerTool
	 Does the select/move tool operation.
***************************************************************************/
class PointerTool : public Tool
{
protected:
	enum {HYSTERESIS = 3};
	QPoint m_downPt2d;
	Coord3D m_downPt3d;
	MapObject *m_curObject;

	Bool m_moving; ///< True if we are drag moving an object.
	Bool m_rotating; ///< True if we are rotating an object.
	Bool m_dragSelect; ///< True if we are drag selecting.

	ModifyObjectUndoable *m_modifyUndoable;	 ///< The modify undoable that is in progress while we track the mouse.

	Bool m_mouseUpRotate;///< True if we are over the "rotate" hotspot.
	QCursor m_rotateCursor;
	Bool m_mouseUpMove;///< True if we are over the "move" hotspot.
	QCursor m_moveCursor;

protected:
	void checkForPropertiesPanel(void);

public:
	PointerTool(void);
	~PointerTool(void);

public:
	/// Clear the selection on activate or deactivate.
	virtual void activate();
	virtual void deactivate();

	virtual QCursor getCursor(void);
	virtual void mouseDown(TTrackingMode m, QPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void mouseMoved(TTrackingMode m, QPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void mouseUp(TTrackingMode m, QPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);

public:
	static void clearSelection(void); ///< Clears the selected objects selected flags.
	static Bool allowPick(MapObject* pMapObj, WbView* pView);
	static Real calcAngle(Coord3D downPt, Coord3D curPt); ///< From ObjectTool; angle of curPt around downPt.
};


#endif //POINTER_TOOL_H
