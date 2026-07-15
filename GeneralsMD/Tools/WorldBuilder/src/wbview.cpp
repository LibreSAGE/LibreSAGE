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

#include <QMessageBox>
#include <QMouseEvent>

// wbview.cpp : common view behavior (Qt6 port).  Routes mouse input into the
// tool framework and generates the status bar text.

#include "wbview.h"

#include "MainFrm.h"
#include "WHeightMapEdit.h"
#include "WorldBuilder.h"
#include "WorldBuilderDoc.h"

#include "Common/MapObject.h"
#include "Common/WellKnownKeys.h"
#include "W3DDevice/GameClient/HeightMap.h"


Bool WbView::m_snapToGrid = false;

// ----------------------------------------------------------------------------
WbView::WbView(QWidget *parent) :
	QWidget(parent),
	m_trackingMode(TRACK_NONE),
	m_centerPt(0, 0, 0),
	m_showObjects(true),
	m_showModels(true),
	m_showGarrisoned(false),
	m_showWaypoints(true),
	m_showPolygonTriggers(true),
	m_showTerrain(true),
	m_lockAngle(false),
	m_doLockAngle(false),
	m_hysteresis(0),
	m_pickConstraint(ES_NONE)
{
	setMouseTracking(true);
}

// ----------------------------------------------------------------------------
void WbView::mouseDownInView(TTrackingMode m, QPoint viewPt)
{
	// can happen if you press 2 mouse buttons.
	if (m_trackingMode != TRACK_NONE)
		return;

	m_mouseDownPoint = viewPt;
	viewToDocCoords(viewPt, &m_mouseDownDocPoint);
	m_trackingMode = m;
	WbApp()->updateCurTool(m == TRACK_R || m == TRACK_M);
	WbApp()->lockCurTool();
	// If we have a tool, invoke it's mouse down method.
	if (WbApp()->getCurTool()) {
		WbApp()->getCurTool()->mouseDown(m, viewPt, this, WbDoc());
	}
}

// ----------------------------------------------------------------------------
void WbView::mouseMoveInView(TTrackingMode m, QPoint viewPt)
{
	if (m_trackingMode == TRACK_NONE) {
		// Don't lock while the mouse is up.
		m_doLockAngle = false;
	} else {
		m_doLockAngle = m_lockAngle;
	}
	if (m_trackingMode == m) {
		WbApp()->updateCurTool(false);
		if (WbApp()->getCurTool()) {
			WbApp()->getCurTool()->mouseMoved(m, viewPt, this, WbDoc());
		}
	}
	if (CMainFrame::GetMainFrame()->isAutoSaving()) {
		return;
	}

	// Generate the status text display with coordinates and height.
	Coord3D cpt;
	viewToDocCoords(viewPt, &cpt);

	WorldHeightMapEdit *pMap = WbDoc()->GetHeightMap();
	if (pMap == NULL || TheTerrainRenderObject == NULL) {
		return;
	}
	MapObject *pObj = MapObject::getFirstMapObject();
	Int totalObjects = 0;
	Int totalWaypoints = 0;
	Int numSelected = 0;
	while(pObj) {
		if (pObj->isSelected()) {
			numSelected++;
		}
		const Int flagsWeDontWant = FLAG_ROAD_FLAGS | FLAG_ROAD_CORNER_ANGLED | FLAG_BRIDGE_FLAGS | FLAG_ROAD_CORNER_TIGHT | FLAG_ROAD_JOIN;
		Int flags = pObj->getFlags();
		if (!(pObj->isWaypoint() || (flags & flagsWeDontWant) != 0))
			++totalObjects;
		else
			++totalWaypoints;

		pObj = pObj->getNext();
	}

	Real height = TheTerrainRenderObject->getHeightMapHeight(cpt.x, cpt.y, NULL);
	QString str = QString("%1 object(s), %2 waypoint(s), (%3,%4), height %5")
		.arg(totalObjects).arg(totalWaypoints)
		.arg(cpt.x, 0, 'f', 2).arg(cpt.y, 0, 'f', 2).arg(height, 0, 'f', 2);
	if (numSelected) {
		str += QString(" (%1 selected)").arg(numSelected);
	}
	CMainFrame::GetMainFrame()->SetMessageText(str.toUtf8().constData());
}

// ----------------------------------------------------------------------------
void WbView::mouseUpInView(TTrackingMode m, QPoint viewPt)
{
	if (m_trackingMode == TRACK_NONE)
		return;

	// If we were tracking a tool, call its mouseup method.
	if (WbApp()->getCurTool()) {
		WbApp()->getCurTool()->mouseUp(m, viewPt, this, WbDoc());
		// Give warning messages at this point.
		WorldHeightMapEdit *pMap = WbDoc()->GetHeightMap();
		if (pMap) {
			if (pMap->tooManyTextures()) {
				QMessageBox::warning(NULL, "WorldBuilder", "There are too many tiles in this map.");
			}
			if (pMap->tooManyBlends()) {
				QMessageBox::warning(NULL, "WorldBuilder", "There are too many blended tiles in this map.");
			}
			pMap->clearStatus();
		}
	}
	WbApp()->unlockCurTool();
	// We don't update the tool while tracking down, wouldn't want the eyedropper
	// to suddenly turn into the flood fill tool, so now we update in case a key
	// was released while tracking.
	WbApp()->updateCurTool(false);
	m_trackingMode = TRACK_NONE;
}

// ----------------------------------------------------------------------------
/** Standard pick test: center hotspot moves, arrow ring rotates. */
TPickedStatus WbView::picked(MapObject *pObj, Coord3D docPt)
{
	Coord3D cloc = *pObj->getLocation();
	if (!m_showObjects && !pObj->isWaypoint()) {
		return PICK_NONE;
	}
	if (!m_showWaypoints && pObj->isWaypoint()) {
		return PICK_NONE;
	}

	Bool doArrow = pObj->isSelected();
	// Check and see if we are within 1/2 cell size of the center.
	Coord3D cpt = docPt;
	cpt.x -= cloc.x;
	cpt.y -= cloc.y;
	cpt.z = 0;
	if (cpt.length() < 0.5f*MAP_XY_FACTOR+m_hysteresis) {
		return PICK_CENTER;
	}
	if (pObj->getFlag(FLAG_ROAD_FLAGS) ||  pObj->getFlag(FLAG_BRIDGE_FLAGS) || pObj->isWaypoint()) {
		doArrow = false;
	}
	// Check and see if we are within 1 cell size of the center.
	if (doArrow && cpt.length() < 1.5f*MAP_XY_FACTOR+m_hysteresis) {
		return PICK_ARROW;
	}
	return PICK_NONE;
}

// ----------------------------------------------------------------------------
void WbView::constrainCenterPt()
{
	// The original allowed scrolling anywhere; kept for parity.
}

// ----------------------------------------------------------------------------
static TTrackingMode modeForButton(Qt::MouseButton button)
{
	switch (button) {
	case Qt::LeftButton: return TRACK_L;
	case Qt::MiddleButton: return TRACK_M;
	case Qt::RightButton: return TRACK_R;
	default: return TRACK_NONE;
	}
}

void WbView::mousePressEvent(QMouseEvent *event)
{
	TTrackingMode m = modeForButton(event->button());
	if (m != TRACK_NONE) {
		mouseDownInView(m, event->pos());
	}
	// Apply the current tool's cursor while tracking.
	if (WbApp()->getCurTool()) {
		setCursor(WbApp()->getCurTool()->getCursor());
	}
}

void WbView::mouseReleaseEvent(QMouseEvent *event)
{
	TTrackingMode m = modeForButton(event->button());
	if (m != TRACK_NONE) {
		mouseUpInView(m, event->pos());
	}
}

void WbView::mouseMoveEvent(QMouseEvent *event)
{
	TTrackingMode m = m_trackingMode;
	mouseMoveInView(m, event->pos());
	// Keep the cursor in sync (tools change it depending on what is under
	// the mouse, e.g. the pointer's move/rotate hotspots).
	if (WbApp()->getCurTool()) {
		setCursor(WbApp()->getCurTool()->getCursor());
	}
}
