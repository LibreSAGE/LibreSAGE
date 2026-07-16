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

// PolygonTool.cpp
// Polygon area trigger tool for worldbuilder (Qt6 port).
// Author: John Ahlquist, Nov. 2001
//
// Qt6 port note: the water-tool filter and the waypoint/polygon options panel
// are not ported yet, so WaterTool::isActive() is treated as false and the
// WaypointOptions updates are dropped.  @todo

#include <QCursor>

#include "PolygonTool.h"
#include "CUndoable.h"
#include "PointerTool.h"
#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"
#include "MainFrm.h"
#include "DrawObject.h"
#include "wbview.h"
#include "GameLogic/PolygonTrigger.h"
#include "W3DDevice/GameClient/HeightMap.h"

//
// PolygonTool class.
//

Bool PolygonTool::m_poly_isActive = false;
Bool PolygonTool::m_poly_isAdding = false;
PolygonTrigger *PolygonTool::m_poly_curSelectedPolygon = NULL;
Int	PolygonTool::m_poly_dragPointNdx = -1;

enum {SNAP_DISTANCE = 5};

/// Constructor
PolygonTool::PolygonTool(void) :
	PolygonTool(ID_POLYGON_TOOL, ":/cursors/cross.cur")
{
}

/// Constructor for subclasses with their own tool id and cursor.
PolygonTool::PolygonTool(Int toolID, const char *cursorResource) :
	Tool(toolID, cursorResource),
	m_poly_isDraggingPoint(false),
	m_poly_justPicked(false),
	m_poly_mouseUpPlus(false),
	m_poly_plusCursor(Qt::CrossCursor),
	m_poly_mouseUpMove(false),
	m_poly_moveCursor(Qt::SizeAllCursor),
	m_poly_moveUndoable(NULL)
{
}



/// Destructor
PolygonTool::~PolygonTool(void)
{
}

/// Clears it's is active flag.
void PolygonTool::deactivate() 
{
	m_poly_dragPointNdx = -1;
	if (m_poly_curSelectedPolygon) {
		Int numPoints = m_poly_curSelectedPolygon->getNumPoints();
		if (numPoints < 3) {
			deleteSelectedPolygon();
		}
	}
	m_poly_isActive = false;
	m_poly_curSelectedPolygon = NULL;
}

/// Shows the polygon (waypoint) options panel.
void PolygonTool::activate()
{
	CMainFrame::GetMainFrame()->showOptionsDialog(ID_POLYGON_TOOL);
	/// @todo WaypointOptions::update() once the waypoint/polygon panel is ported.
	if (!m_poly_curSelectedPolygon) {
		PointerTool::clearSelection();
		DrawObject::setDoBrushFeedback(false);
		m_poly_isActive = true;
		m_poly_isAdding = false;
		m_poly_mouseUpMove = false;
		m_poly_mouseUpPlus = false;
	}
}

// Pick a polygon.
Bool PolygonTool::poly_pickPoly(PolygonTrigger *pTrig, Coord3D loc, Int tolerance) {
	ICoord3D iDocPt;
	iDocPt.x = floor(loc.x+0.5f);
	iDocPt.y = floor(loc.y+0.5f);
	iDocPt.z = floor(loc.z+0.5f);
	Int i, j;
	ICoord3D ptArray[9];
	for (i=0; i<3; i++) {
		for (j=0; j<3; j++) {
			ptArray[i+3*j].x = iDocPt.x + (1-i)*tolerance;
			ptArray[i+3*j].y = iDocPt.y + (1-j)*tolerance;
			ptArray[i+3*j].z = iDocPt.z;
		}
	}
	int count = 0;
	for (i=0; i<9; i++) {
		if (pTrig->pointInTrigger(ptArray[i])) {
			count++;
		}
	}
	if (count>0 && count<8) {
		return true;
	}		
	return false;
}



// Pick a polygon.
PolygonTrigger *PolygonTool::pickPolygon(Coord3D loc, QPoint viewPt, WbView* pView) {
	// @todo the water tool is not ported, so its water-only filter is disabled.
	for (PolygonTrigger *pTrig=PolygonTrigger::getFirstPolygonTrigger(); pTrig; pTrig = pTrig->getNext()) {
		if (poly_pickPoly(pTrig, loc, SNAP_DISTANCE/2)) {
			return pTrig;
		}
	}
	for (PolygonTrigger *pTrig=PolygonTrigger::getFirstPolygonTrigger(); pTrig; pTrig = pTrig->getNext()) {
		Coord3D docPt = loc;
		Int pt = poly_pickPoint(pTrig, viewPt, pView);
		if (pt>=0) {
			return pTrig;
		}
		if (poly_pickPoly(pTrig, docPt, 3*SNAP_DISTANCE/2)) {
			return pTrig;
		}
	}
	return NULL; 
}

// Snap to other polys.
Bool PolygonTool::poly_snapToPoly(Coord3D *pLoc) {
	for (PolygonTrigger *pTrig=PolygonTrigger::getFirstPolygonTrigger(); pTrig; pTrig = pTrig->getNext()) {
		Int i;
		// Dont' snap to self.
		if (pTrig == m_poly_curSelectedPolygon) continue;
		for (i=0; i<pTrig->getNumPoints(); i++) {
			ICoord3D iLoc = *pTrig->getPoint(i);
			Coord3D cpt = *pLoc;
			cpt.x -= iLoc.x;
			cpt.y -= iLoc.y;
			cpt.z = 0;
			if (cpt.length() < 1.0f*SNAP_DISTANCE) {
				pLoc->x = iLoc.x;
				pLoc->y = iLoc.y;
				return true;
			}
		}
	}
	return false;
}

// Pick a point.
Int PolygonTool::poly_pickPoint(PolygonTrigger *pTrig, QPoint viewPt, WbView* pView) {
	if (pTrig==NULL) return -1;
	Int i;
	const Int PICK_PIXELS = pView->getPickPixels();
	for (i=0; i<pTrig->getNumPoints(); i++) {
		ICoord3D iLoc = *pTrig->getPoint(i);
		Coord3D cpt;
		cpt.x = iLoc.x;
		cpt.y = iLoc.y;
		cpt.z = iLoc.z;
		cpt.z = 0;
		QPoint screenLoc;
		pView->docToViewCoords(cpt, &screenLoc);
		Int dx = screenLoc.x()-viewPt.x();
		Int dy = screenLoc.y() - viewPt.y();
		if (dy<0) dy = -dy;
		if (dx<0) dx = -dx;
		if (dx<=PICK_PIXELS && dy<= PICK_PIXELS) {
			return i;
		}
	}
	return -1;
}

// Get the point to insert closest to loc.
Int PolygonTool::poly_getInsertIndex(PolygonTrigger *pTrig, Coord3D loc) {
	if (pTrig==NULL) return 0;
	static Int tolerance = SNAP_DISTANCE;

	Int i;
	for (i=0; i<pTrig->getNumPoints(); i++) {
		ICoord3D pt1 = *pTrig->getPoint(i);
		ICoord3D pt2;
		Int theNdx = i+1;
		if (i==pTrig->getNumPoints()-1) {
			pt2 = *pTrig->getPoint(0);
			theNdx = 0;
		} else {
			pt2 = *pTrig->getPoint(i+1);
		}
		Int minX = loc.x-tolerance;
		if (pt1.x<minX && pt2.x<minX) continue;
		Int maxX = loc.x+tolerance;
		if (pt1.x>maxX && pt2.x>maxX) continue;
		Int minY = loc.y-tolerance;
		if (pt1.y<minY && pt2.y<minY) continue;
		Int maxY = loc.y+tolerance;
		if (pt1.y>maxY && pt2.y>maxY) continue;
		Int dy = pt2.y-pt1.y;
		Int dx = pt2.x-pt1.x;
		if (abs(dy)>abs(dx)) {
			Real intersectionX = pt1.x + (dx * (loc.y-pt1.y)) / ((Real)dy);
			if (intersectionX >= minX && intersectionX <= maxX) {
				return theNdx;
			}
		} else {
			Real intersectionY = pt1.y + (dy * (loc.x-pt1.x)) / ((Real)dx);
			if (intersectionY >= minY && intersectionY <= maxY) {
				return theNdx;
			}
		}
	}
	return 0;
}

/** Do the pick on poly triggers. */
void PolygonTool::poly_pickOnMouseDown(QPoint viewPt, WbView* pView)
{						 
	m_poly_dragPointNdx = -1;
	if (m_poly_curSelectedPolygon) {
		Bool found = false;
		for (PolygonTrigger *pTrig=PolygonTrigger::getFirstPolygonTrigger(); pTrig; pTrig = pTrig->getNext()) {
			if (m_poly_curSelectedPolygon == pTrig) {
				found = true;
				break;
			}
		}
		if (!found) {
			m_poly_curSelectedPolygon = NULL; // Undo probably made it go away.
		}
	}
	m_poly_justPicked = false;	 // Always false here.  Pointer tool sets to true.
	if (m_poly_curSelectedPolygon == NULL || !m_poly_isAdding) {
		PolygonTrigger *pSel;
		Coord3D docPt = m_poly_unsnappedMouseDownPt;
		if (m_poly_curSelectedPolygon && poly_pickPoly(m_poly_curSelectedPolygon, docPt, SNAP_DISTANCE)) {
			pSel = m_poly_curSelectedPolygon;
		} else {
			pSel = pickPolygon(docPt, viewPt, pView);
		}
		m_poly_curSelectedPolygon = pSel;
		m_poly_isAdding = false;
	}
}



/// Perform the tool behavior on mouse down.
void PolygonTool::mouseDown(TTrackingMode m, QPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m != TRACK_L) return;
	Coord3D docPt;
	pView->viewToDocCoords(viewPt, &docPt);
	m_poly_unsnappedMouseDownPt = docPt;
	poly_pickOnMouseDown(viewPt, pView);
	if (!poly_snapToPoly(&docPt)) {
		pView->snapPoint(&docPt);
	}
	m_poly_mouseDownPt = docPt;
	startMouseDown(m, viewPt, pView, pDoc);
}

/// Perform the tool behavior on mouse down.
void PolygonTool::startMouseDown(TTrackingMode m, QPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	// Clear any selected map objects.
	if (!m_poly_curSelectedPolygon) PointerTool::clearSelection();

	ICoord3D iDocPt;
	iDocPt.x = floor(m_poly_mouseDownPt.x+0.5f);
	iDocPt.y = floor(m_poly_mouseDownPt.y+0.5f);
	iDocPt.z = floor(m_poly_mouseDownPt.z+0.5f);
	if (m_poly_curSelectedPolygon && m_poly_curSelectedPolygon->isWaterArea()) {
		iDocPt.z = m_poly_curSelectedPolygon->getPoint(0)->z;
	}
	m_poly_dragPointNdx = -1;
	m_poly_moveUndoable = NULL;
	if (m_poly_curSelectedPolygon==NULL) {
		// adding a new polygon.
		m_poly_curSelectedPolygon = newInstance(PolygonTrigger)(32); 
		AsciiString name;
		name.format("Area %d", m_poly_curSelectedPolygon->getID());
		m_poly_curSelectedPolygon->setTriggerName(name);
		m_poly_curSelectedPolygon->addPoint(iDocPt);
		m_poly_dragPointNdx = 0;
		AddPolygonUndoable *pUndo = new AddPolygonUndoable(m_poly_curSelectedPolygon);
		pDoc->AddAndDoUndoable(pUndo);
		REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
		m_poly_isAdding = true;
	}	else {
		m_poly_dragPointNdx = poly_pickPoint(m_poly_curSelectedPolygon, viewPt, pView);
		if (m_poly_isAdding) {
			if (m_poly_dragPointNdx == 0) {
				// closing the poly.
				m_poly_isAdding = false;
			} else {
				m_poly_dragPointNdx = -1;
			}
		}
		if (m_poly_isAdding && m_poly_dragPointNdx == -1) {
			// Continuing to add points.
			m_poly_dragPointNdx = m_poly_curSelectedPolygon->getNumPoints();
			AddPolygonPointUndoable *pUndo = new AddPolygonPointUndoable(m_poly_curSelectedPolygon, iDocPt);
			pDoc->AddAndDoUndoable(pUndo);
			REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
		}	else if (m_poly_dragPointNdx != -1) {
			// Moving a point on the polygon.
			ModifyPolygonPointUndoable *pUndo = new ModifyPolygonPointUndoable(m_poly_curSelectedPolygon, m_poly_dragPointNdx);
			pDoc->AddAndDoUndoable(pUndo);
			REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
		}	else if (m_poly_justPicked) {
			// Move the entire polygon.
			m_poly_moveUndoable = new MovePolygonUndoable(m_poly_curSelectedPolygon);
			pDoc->AddAndDoUndoable(m_poly_moveUndoable);
		}	else {
			// Insert a point.
			Coord3D docPt = m_poly_unsnappedMouseDownPt;
			m_poly_dragPointNdx = poly_getInsertIndex(m_poly_curSelectedPolygon, docPt);
			InsertPolygonPointUndoable *pUndo = new InsertPolygonPointUndoable(m_poly_curSelectedPolygon, iDocPt, m_poly_dragPointNdx);
			pDoc->AddAndDoUndoable(pUndo);
			REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
		}
	}
	/// @todo WaypointOptions::update() once the panel is ported.
}

/// Delete the selected polygon or point.
Bool PolygonTool::deleteSelectedPolygon(void) 
{
	if (m_poly_curSelectedPolygon) {
		Bool found = false;
		for (PolygonTrigger *pTrig=PolygonTrigger::getFirstPolygonTrigger(); pTrig; pTrig = pTrig->getNext()) {
			if (m_poly_curSelectedPolygon == pTrig) {
				found = true;
				break;
			}
		}
		if (!found) {
			m_poly_curSelectedPolygon = NULL; // Undo probably made it go away.
		}
	}
	if (m_poly_curSelectedPolygon == NULL) {
		return false;
	}
	CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
	if (!pDoc) return false;
	if (m_poly_dragPointNdx != -1) {
		// Moving a point on the polygon.
		DeletePolygonPointUndoable *pUndo = new DeletePolygonPointUndoable(m_poly_curSelectedPolygon, m_poly_dragPointNdx);
		pDoc->AddAndDoUndoable(pUndo);
		REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
		m_poly_dragPointNdx = -1;
	} else {
		// delete the polygon.
		DeletePolygonUndoable *pUndo = new DeletePolygonUndoable(m_poly_curSelectedPolygon);
		pDoc->AddAndDoUndoable(pUndo);
		REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
		m_poly_curSelectedPolygon = NULL;
		m_poly_dragPointNdx = -1;
	}
	/// @todo WaypointOptions::update() once the panel is ported.
	return true;
}

/** Return the cursor for the current hotspot. */
QCursor PolygonTool::getCursor(void)
{
	if (m_poly_mouseUpPlus || (m_poly_isAdding && m_poly_curSelectedPolygon)) {
		return m_poly_plusCursor;
	} else if (m_poly_mouseUpMove) {
		return m_poly_moveCursor;
	}
	return Tool::getCursor();
}



/// Left button move code.
void PolygonTool::mouseMoved(TTrackingMode m, QPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	Coord3D docPt;
	pView->viewToDocCoords(viewPt, &docPt);
	if (m == TRACK_NONE) {
		PolygonTrigger *pCur = m_poly_curSelectedPolygon;
		Int curPt = m_poly_dragPointNdx;
		m_poly_unsnappedMouseDownPt = docPt;
		poly_pickOnMouseDown(viewPt, pView);
		m_poly_mouseUpPlus = false;
		m_poly_mouseUpMove = false;
		if (m_poly_curSelectedPolygon) {
			Int pt = poly_pickPoint(m_poly_curSelectedPolygon, viewPt, pView);
			if (pt >= 0) {
				m_poly_mouseUpMove = true;
			} else {
				m_poly_mouseUpPlus = true;
			}
		}
		m_poly_curSelectedPolygon = pCur;
		m_poly_dragPointNdx = curPt;
		return;	// setCursor will use the value of m_mouseUpRotate.  jba.
	}

	if (m != TRACK_L) return;
	if (!poly_snapToPoly(&docPt)) {
		pView->snapPoint(&docPt);
	}
	if (m_poly_moveUndoable) {
		ICoord3D iDocPt;
		iDocPt.x = floor(docPt.x+0.5f-m_poly_mouseDownPt.x);
		iDocPt.y = floor(docPt.y+0.5f-m_poly_mouseDownPt.y);
		iDocPt.z = 0;
		m_poly_moveUndoable->SetOffset(iDocPt);
		pView->update();
		return;
	}
	if (m_poly_dragPointNdx >= 0 && m_poly_curSelectedPolygon) {
		ICoord3D iDocPt;
		iDocPt.x = floor(docPt.x+0.5f);
		iDocPt.y = floor(docPt.y+0.5f);
		iDocPt.z = floor(docPt.z+0.5f);
		if (m_poly_curSelectedPolygon->isWaterArea()) {
			iDocPt.z = m_poly_curSelectedPolygon->getPoint(m_poly_dragPointNdx)->z;
		}
		m_poly_curSelectedPolygon->setPoint(iDocPt, m_poly_dragPointNdx);
		pView->update();
	}
}

/** Mouse up - not much. */
void PolygonTool::mouseUp(TTrackingMode m, QPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m != TRACK_L) return;
	REF_PTR_RELEASE(m_poly_moveUndoable); // belongs to pDoc now.
}

