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

// PointerTool.cpp
// Select and move tool for worldbuilder.
// Author: John Ahlquist, April 2001
//
// Qt6 port note: the polygon-trigger editing branch (PolygonTool base) is not
// ported yet; selection, drag-select, move and rotate work.  @todo

#include <QGuiApplication>
#include <QPixmap>
#include <QRect>

#include "PointerTool.h"

#include "CUndoable.h"
#include "MainFrm.h"
#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"
#include "mapobjectprops.h"
#include "wbview.h"

#include "Common/ThingTemplate.h"
#include "GameLogic/SidesList.h"

#include <algorithm>

//
// Static helper functions
// This function spiders out and un/picks all Waypoints that have some form of indirect contact with this point
//
static void helper_pickAllWaypointsInPath( Int sourceID, CWorldBuilderDoc *pDoc, const Int numWaypointLinks, std::vector<Int>& alreadyTouched );

static void pickAllWaypointsInPath( Int sourceID, Bool select )
{
	std::vector<Int> alreadyTouched;
	CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();

	helper_pickAllWaypointsInPath(sourceID, pDoc, pDoc->getNumWaypointLinks(), alreadyTouched);

	// already touched should now be filled with waypointIDs that want to be un/selected
	MapObject *pMapObj = MapObject::getFirstMapObject();
	while (pMapObj) {
		if (pMapObj->isWaypoint()) {
			if (std::find(alreadyTouched.begin(), alreadyTouched.end(), pMapObj->getWaypointID()) != alreadyTouched.end()) {
				pMapObj->setSelected(select);
			}
		}
		pMapObj = pMapObj->getNext();
	}
}

static void helper_pickAllWaypointsInPath( Int sourceID, CWorldBuilderDoc *pDoc, const Int numWaypointLinks, std::vector<Int>& alreadyTouched )
{
	if (std::find(alreadyTouched.begin(), alreadyTouched.end(), sourceID) != alreadyTouched.end() ) {
		return;
	}

	alreadyTouched.push_back(sourceID);
	for (int i = 0; i < numWaypointLinks; ++i) {
		Int way1, way2;
		pDoc->getWaypointLink(i, &way1, &way2);
		if (way1 == sourceID) {
			helper_pickAllWaypointsInPath(way2, pDoc, numWaypointLinks, alreadyTouched);
		}

		if (way2 == sourceID) {
			helper_pickAllWaypointsInPath(way1, pDoc, numWaypointLinks, alreadyTouched);
		}
	}
}

//
// PointerTool class.
//

/// Constructor
PointerTool::PointerTool(void) :
	Tool(ID_POINTER_TOOL, ":/cursors/IDC_POINTER.cur"),
	m_curObject(NULL),
	m_moving(false),
	m_rotating(false),
	m_dragSelect(false),
	m_modifyUndoable(NULL),
	m_mouseUpRotate(false),
	m_mouseUpMove(false)
{
	QPixmap rotatePix(":/cursors/IDC_ROTATE.cur");
	if (!rotatePix.isNull()) m_rotateCursor = QCursor(rotatePix);
	QPixmap movePix(":/cursors/IDC_MOVE_POINTER.cur");
	if (!movePix.isNull()) m_moveCursor = QCursor(movePix);
}

/// Destructor
PointerTool::~PointerTool(void)
{
	REF_PTR_RELEASE(m_modifyUndoable); // belongs to pDoc now.
}

// From ObjectTool - angle of curPt around downPt.
Real PointerTool::calcAngle(Coord3D downPt, Coord3D curPt)
{
	double dx = curPt.x - downPt.x;
	double dy = curPt.y - downPt.y;
	double dist = sqrt(dx*dx+dy*dy);
	double angle = 0;
	if (dist < 0.1) // check for div-by-zero.
	{
		angle = 0;
	}
	else if (fabs(dx) > fabs(dy))
	{
		angle = acos(	(double)dx / dist);
		if (dy<0) angle = -angle;
	}
	else
	{
		angle = asin(	((double)dy) / dist);
		if (dx<0) angle = PI-angle;
	}
	if (angle > PI) angle -= 2*PI;
	return((Real)angle);
}

/// See if a single obj is selected that has properties.
void PointerTool::checkForPropertiesPanel(void)
{
	/// @todo route waypoint/water/light/road selections to their panels once
	/// those are ported (see the original checkForPropertiesPanel).
	if (CMainFrame::GetMainFrame()) {
		CMainFrame::GetMainFrame()->showOptionsDialog(ID_POINTER_TOOL);
	}
	MapObjectProps::update();
}

/// Clear the selection..
void PointerTool::clearSelection(void) ///< Clears the selected objects selected flags.
{
	// Clear selection.
	MapObject *pObj = MapObject::getFirstMapObject();
	while (pObj) {
		if (pObj->isSelected()) {
			pObj->setSelected(false);
		}
		pObj = pObj->getNext();
	}
	// Clear selected build list items.
	Int i;
	for (i=0; i<TheSidesList->getNumSides(); i++) {
		SidesInfo *pSide = TheSidesList->getSideInfo(i);
		for (BuildListInfo *pBuild = pSide->getBuildList(); pBuild; pBuild = pBuild->getNext()) {
			if (pBuild->isSelected()) {
				pBuild->setSelected(false);
			}
		}
	}
	MapObjectProps::update();
}

/// Activate.
void PointerTool::activate()
{
	Tool::activate();
	m_mouseUpRotate = false;
	m_mouseUpMove = false;
	checkForPropertiesPanel();
}

/// deactivate.
void PointerTool::deactivate()
{
	m_curObject = NULL;
	clearSelection();
}

/** The cursor reflects the hotspot under the mouse. */
QCursor PointerTool::getCursor(void)
{
	if (m_mouseUpRotate) {
		return m_rotateCursor;
	} else if (m_mouseUpMove) {
		return m_moveCursor;
	}
	return Tool::getCursor();
}

Bool PointerTool::allowPick(MapObject* pMapObj, WbView* pView)
{
	EditorSortingType sort = ES_NONE;
	if (!pMapObj) {
		return false;
	}
	const ThingTemplate *tt = pMapObj->getThingTemplate();
	if (tt && tt->getEditorSorting() == ES_AUDIO) {
		if (pView->GetPickConstraint() == ES_NONE || pView->GetPickConstraint() == ES_AUDIO) {
			return true;
		}
	}
	if ((tt && !pView->getShowModels()) || (pMapObj->getFlags() & FLAG_DONT_RENDER)) {
		return false;
	}
	if (pView->GetPickConstraint() != ES_NONE) {
		if (tt) {
			if (!pView->getShowModels()) {
				return false;
			}
			sort = tt->getEditorSorting();
		} else {
			if (pMapObj->isWaypoint()) {
				sort = ES_WAYPOINT;
			}
			if (pMapObj->getFlag(FLAG_ROAD_FLAGS)) {
				sort = ES_ROAD;
			}
		}
		if (sort != ES_NONE && sort != pView->GetPickConstraint()) {
			return false;
		}
	}
	return true;
}

/** Execute the tool on mouse down - Pick an object. */
void PointerTool::mouseDown(TTrackingMode m, QPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m != TRACK_L) return;

	Coord3D cpt;
	pView->viewToDocCoords(viewPt, &cpt);
	Coord3D loc;

	m_downPt2d = viewPt;
	m_downPt3d = cpt;
	pView->snapPoint(&m_downPt3d);
	m_moving = false;
	m_rotating = false;
	m_dragSelect = false;
	Qt::KeyboardModifiers mods = QGuiApplication::queryKeyboardModifiers();
	Bool shiftKey = (mods & Qt::ShiftModifier) != 0;
	Bool ctrlKey = (mods & Qt::ControlModifier) != 0;

	/// @todo polygon trigger picking once PolygonTool is ported.

	m_curObject = NULL;
	MapObject *pObj = MapObject::getFirstMapObject();
	MapObject *p3DObj = pView->picked3dObjectInView(viewPt);
	MapObject *pClosestPicked = NULL;
	if (allowPick(p3DObj, pView)) {
		pClosestPicked = p3DObj;
	}
	Real pickDistSqr = 10000*MAP_XY_FACTOR;
	pickDistSqr *= pickDistSqr;

	// Find the closest pick.
	for (pObj = MapObject::getFirstMapObject(); pObj; pObj = pObj->getNext()) {
		if (!allowPick(pObj, pView)) {
			continue;
		}
		Bool picked = (pView->picked(pObj, cpt) != PICK_NONE);
		if (picked) {
			loc = *pObj->getLocation();
			Real dx = m_downPt3d.x-loc.x;
			Real dy = m_downPt3d.y-loc.y;
			Real distSqr = dx*dx+dy*dy;
			if (distSqr < pickDistSqr) {
				pClosestPicked = pObj;
				pickDistSqr = distSqr;
			}
		}
	}

	Bool anySelected = (pClosestPicked!=NULL);
	if (shiftKey) {
		if (pClosestPicked && pClosestPicked->isSelected()) {
			pClosestPicked->setSelected(false);
			if (ctrlKey && pClosestPicked->isWaypoint()) {
				pickAllWaypointsInPath(pClosestPicked->getWaypointID(), false);
			}
		} else if (pClosestPicked) {
			pClosestPicked->setSelected(true);
			if (ctrlKey && pClosestPicked->isWaypoint()) {
				pickAllWaypointsInPath(pClosestPicked->getWaypointID(), true);
			}
		}
	} else if (pClosestPicked && pClosestPicked->isSelected()) {
		// We picked a selected object
			m_curObject = pClosestPicked;
	} else {
		clearSelection();
		if (pClosestPicked) {
			pClosestPicked->setSelected(true);
			if (ctrlKey && pClosestPicked->isWaypoint()) {
				pickAllWaypointsInPath(pClosestPicked->getWaypointID(), true);
			}

		}
	}

	// Grab both ends of a road.
	if (pView->GetPickConstraint() == ES_NONE || pView->GetPickConstraint() == ES_ROAD) {
		if (!shiftKey && pClosestPicked && (pClosestPicked->getFlags()&FLAG_ROAD_FLAGS) ) {
			for (pObj = MapObject::getFirstMapObject(); pObj; pObj = pObj->getNext()) {
				if (pObj->getFlags()&FLAG_ROAD_FLAGS) {
					loc = *pObj->getLocation();
					Real dx = pClosestPicked->getLocation()->x - loc.x;
					Real dy = pClosestPicked->getLocation()->y - loc.y;
					Real dist = sqrt(dx*dx+dy*dy);
					if (dist < MAP_XY_FACTOR/100) {
						pObj->setSelected(true);
					}
				}
			}
		}
	}

	if (anySelected) {
		if (m_curObject) {
			// See if we are picking on the arrow.
			if (pView->picked(m_curObject, cpt) == PICK_ARROW) {
				m_rotating = true;
			}
		}	else {
			pObj = MapObject::getFirstMapObject();
			while (pObj) {
				if (pObj->isSelected()) {
					m_curObject = pObj;
					break;
				}
				pObj = pObj->getNext();
			}
		}
		if (m_curObject) {
			// adjust the starting point so if we are snapping, the object snaps as well.
			loc = *m_curObject->getLocation();
			Coord3D snapLoc = loc;
			pView->snapPoint(&snapLoc);
			m_downPt3d.x += (loc.x-snapLoc.x);
			m_downPt3d.y += (loc.y-snapLoc.y);
		}
	}	else {
		m_dragSelect = true;
	}
	MapObjectProps::update();
}

/// Left button move code.
void PointerTool::mouseMoved(TTrackingMode m, QPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	Coord3D cpt;
	pView->viewToDocCoords(viewPt, &cpt, false);
	if (m == TRACK_NONE) {
		// See if the cursor is over an object.
		MapObject *pObj = MapObject::getFirstMapObject();
		m_mouseUpRotate = false;
		m_mouseUpMove = false;
		while (pObj) {
			if (allowPick(pObj, pView)) {
				TPickedStatus stat = pView->picked(pObj, cpt);
				if (stat==PICK_ARROW) {
					m_mouseUpRotate = true;
					break;
				}
				if (stat==PICK_CENTER) {
					m_mouseUpMove = true;
					break;
				}
			}
			pObj = pObj->getNext();
		}
		if (!m_mouseUpRotate) {
			pObj = pView->picked3dObjectInView(viewPt);
			if (allowPick(pObj, pView)) {
				m_mouseUpMove = true;
			}
		}
		return;	// getCursor will use the value of m_mouseUpRotate.  jba.
	}

	if (m != TRACK_L) return;

	if (m_dragSelect) {
		QRect box = QRect(m_downPt2d, viewPt).normalized();
		pView->doRectFeedback(true, box);
		pView->update();
		return;
	}

	if (m_curObject == NULL) {
		return;
	}
	pView->viewToDocCoords(viewPt, &cpt, !m_rotating);
	if (!m_moving) {
		// always use view coords (not doc coords) for hysteresis
		Int dx = viewPt.x()-m_downPt2d.x();
		Int dy = viewPt.y()-m_downPt2d.y();
		if (abs(dx)>HYSTERESIS || abs(dy)>HYSTERESIS) {
			m_moving = true;
			m_modifyUndoable = new ModifyObjectUndoable(pDoc);
		}
	}
	if (!m_moving || !m_modifyUndoable) return;

	MapObject *curMapObj = MapObject::getFirstMapObject();
	while (curMapObj) {
		if (curMapObj->isSelected()) {
			pView->invalObjectInView(curMapObj);
		}
		curMapObj = curMapObj->getNext();
	}

	if (m_rotating) {
		Coord3D center = *m_curObject->getLocation();
		m_modifyUndoable->RotateTo(calcAngle(center, cpt));
	} else {
		pView->snapPoint(&cpt);
		Real xOffset = (cpt.x-m_downPt3d.x);
		Real yOffset = (cpt.y-m_downPt3d.y);
		m_modifyUndoable->SetOffset(xOffset, yOffset);
	}

	curMapObj = MapObject::getFirstMapObject();
	while (curMapObj) {
		if (curMapObj->isSelected()) {
			pView->invalObjectInView(curMapObj);
		}
		curMapObj = curMapObj->getNext();
	}

	pDoc->updateAllViews();

}


/** Execute the tool on mouse up - if modifying, do the modify,
else update the selection. */
void PointerTool::mouseUp(TTrackingMode m, QPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m != TRACK_L) return;

	Coord3D cpt;
	pView->viewToDocCoords(viewPt, &cpt);

	if (m_curObject && m_moving) {
		pDoc->AddAndDoUndoable(m_modifyUndoable);
		REF_PTR_RELEASE(m_modifyUndoable); // belongs to pDoc now.
	}	else if (m_dragSelect) {
		QRect box = QRect(m_downPt2d, viewPt).normalized();
		pView->doRectFeedback(false, box);
		pView->update();

		Bool shiftKey = (QGuiApplication::queryKeyboardModifiers() & Qt::ShiftModifier) != 0;
		MapObject *pObj;
		for (pObj = MapObject::getFirstMapObject(); pObj; pObj = pObj->getNext()) {
			// Don't pick on invisible waypoints
			if (pObj->isWaypoint() && !pView->isWaypointVisible()) {
				continue;
			}
			if (!allowPick(pObj, pView)) {
				continue;
			}
			Bool picked;
			Coord3D loc = *pObj->getLocation();
			QPoint objViewPt;
			if (pView->docToViewCoords(loc, &objViewPt)){
				picked = box.contains(objViewPt);
				if (picked) {
					if (shiftKey) {
						pObj->setSelected(!pObj->isSelected());
					}	else {
						pObj->setSelected(true);
					}
					pDoc->invalObject(pObj);
				}
			}
		}

	}
	m_moving = false;
	m_rotating = false;
	m_dragSelect = false;
	checkForPropertiesPanel();
}
