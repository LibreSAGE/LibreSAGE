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

// wbview.h : common view base class (Qt6 port of the MFC CView subclass)
//

#pragma once

#include <QWidget>

#include "Lib/BaseType.h"
#include "Tool.h"
#include "WorldBuilderDoc.h"

#include "vector3.h"
#include "Common/ThingSort.h"
#include "Common/MapObject.h"

class WorldHeightMapEdit;

enum TPickedStatus {
	PICK_NONE,
	PICK_CENTER,
	PICK_ARROW
};
class WorldHeightMap;

/////////////////////////////////////////////////////////////////////////////
// WbView view

class WbView : public QWidget
{
	Q_OBJECT

public:
	WbView(QWidget *parent = NULL);

protected:
	TTrackingMode					m_trackingMode;
	Vector3								m_centerPt;

	void mouseDownInView(TTrackingMode m, QPoint viewPt);
	void mouseMoveInView(TTrackingMode m, QPoint viewPt);
	void mouseUpInView(TTrackingMode m, QPoint viewPt);

	void constrainCenterPt();

	static Bool					m_snapToGrid;

	Bool					m_showObjects;			  ///< Flag whether object icons are drawn in the view.
	Bool					m_showModels;					///< Flag whether models are drawn in the view.
	Bool					m_showGarrisoned;
	Bool					m_showWaypoints;
	Bool					m_showPolygonTriggers;
	Bool					m_showTerrain;			  ///< Flag whether terrain is rendered or not. (Useful for debugging)

	Bool		m_lockAngle;			///< Reflects the ui button.
	Bool		m_doLockAngle;			///< True if we are currently locking.
	Real		m_hysteresis;
	EditorSortingType	m_pickConstraint;
	QPoint	m_mouseDownPoint;
	Coord3D	m_mouseDownDocPoint;

// Attributes
public:
	virtual Bool viewToDocCoords(QPoint curPt, Coord3D *newPt, Bool constrained=true) { DEBUG_CRASH(("should not call")); newPt->zero(); return false; }
	virtual Bool docToViewCoords(Coord3D curPt, QPoint* newPt) { DEBUG_CRASH(("should not call")); return false; }

	virtual Bool viewToDocCoordZ(QPoint curPt, Coord3D *newPt, Real Z) { return viewToDocCoords(curPt, newPt, false); }

	/// Set the center for display.
	virtual void setCenterInView(Real x, Real y) { }

	/// Update the height map in the 3d window.
	virtual void updateHeightMapInView(WorldHeightMap *htMap, Bool partial, const IRegion2D &partialRange) { }

	/// Invalidate an object (or all when NULL) in the view.
	virtual void invalObjectInView(MapObject *pMapObj) { }

	/// Invalidate a single terrain cell.
	virtual void invalidateCellInView(int xIndex, int yIndex) { }

	/// Scroll by the given amount (in cells).
	virtual void scrollInView(Real x, Real y, Bool end) { }

	virtual void setDefaultCamera() { }
	virtual void rotateCamera(Real delta) { }
	virtual void pitchCamera(Real delta) { }

	void snapPoint(Coord3D *thePt) {if (m_snapToGrid || m_lockAngle) {thePt->x = MAP_XY_FACTOR*floor(thePt->x/MAP_XY_FACTOR+0.5); thePt->y = MAP_XY_FACTOR*floor(thePt->y/MAP_XY_FACTOR+0.5);};};

	virtual TPickedStatus picked(MapObject *pObj, Coord3D docPt);
	virtual MapObject *picked3dObjectInView(QPoint viewPt) {return NULL;};

	EditorSortingType GetPickConstraint(void) {return m_pickConstraint;}
	void SetPickConstraint(EditorSortingType constraint) {m_pickConstraint = constraint;}
	Bool isPolygonTriggerVisible(void) {return m_showPolygonTriggers;};
	Bool isWaypointVisible(void) {return m_showWaypoints;};

	/// @todo draw the rubber band once DrawObject is ported.
	void doRectFeedback(Bool doFeedback, const QRect &rect) { }

	Bool getShowObjects(void) {return m_showObjects;}
	Bool getShowModels(void) {return m_showModels;}
	Bool getShowGarrisoned(void) {return m_showGarrisoned;}
	static Bool isSnapToGrid(void) {return m_snapToGrid;}

	void setShowModels(Bool show) {m_showModels = show;}
	void setShowGarrisoned(Bool show) {m_showGarrisoned = show;}

protected:
	// Qt event handlers route mouse input into the tool framework the same
	// way the MFC OnLButtonDown & friends did.
	void mousePressEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
};
