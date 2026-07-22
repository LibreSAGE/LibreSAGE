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

// WaterOptions.cpp : options panel (Qt6 port of the MFC dialog)
//
// Qt6 port note: the original dialog synced against WaypointOptions (not yet
// ported), which tracked the polygon currently selected via the Pointer or
// Polygon tools.  getSingleSelectedPolygon() below reimplements just the
// small piece of that logic this panel needs, rather than depending on the
// unported panel.

#include <QCheckBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QSpinBox>
#include <QVBoxLayout>

#include "WaterOptions.h"

#include "CUndoable.h"
#include "PolygonTool.h"
#include "WorldBuilderDoc.h"
#include "wbview.h"
#include "wbview3d.h"
#include "GameLogic/PolygonTrigger.h"
#include "Common/MapObject.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"

WaterOptions *WaterOptions::m_staticThis = NULL;
Int WaterOptions::m_waterHeight = 7;
Int WaterOptions::m_waterPointSpacing = 2*MAP_XY_FACTOR;
Bool WaterOptions::m_creatingWaterAreas = false;

static const Int MAX_WATER_HEIGHT = (Int)(WorldHeightMap::getMaxHeightValue()*MAP_HEIGHT_SCALE);

WaterOptions::WaterOptions(QWidget *parent) :
	QWidget(parent),
	m_updating(false)
{
	QVBoxLayout *outer = new QVBoxLayout(this);
	QFormLayout *layout = new QFormLayout();
	outer->addLayout(layout);

	m_nameEdit = new QLineEdit(this);
	layout->addRow("Water Area Name:", m_nameEdit);
	connect(m_nameEdit, &QLineEdit::editingFinished, this, [this]() {
		if (m_updating) return;
		PolygonTrigger *theTrigger = getSingleSelectedPolygon();
		if (!theTrigger) return;
		AsciiString name(m_nameEdit->text().toUtf8().constData());
		Bool didMatch = false;
		for (PolygonTrigger *pTrig = PolygonTrigger::getFirstPolygonTrigger(); !didMatch && pTrig; pTrig = pTrig->getNext()) {
			if (pTrig == theTrigger) continue;
			if (name == pTrig->getTriggerName()) {
				didMatch = true;
			}
		}
		if (didMatch) {
			QMessageBox::warning(this, "World Builder", "Name already in use");
			m_nameEdit->setText(QString::fromUtf8(theTrigger->getTriggerName().str()));
		} else {
			theTrigger->setTriggerName(name);
		}
	});

	m_heightSpin = new QSpinBox(this);
	m_heightSpin->setRange(0, MAX_WATER_HEIGHT);
	layout->addRow("Water height:", m_heightSpin);
	connect(m_heightSpin, &QSpinBox::valueChanged, this, [this](int value) {
		if (!m_updating) applyHeight(value);
	});

	m_createPolygonCheck = new QCheckBox("Create Water Polygon", this);
	outer->addWidget(m_createPolygonCheck);
	connect(m_createPolygonCheck, &QCheckBox::toggled, this, [this](bool checked) {
		if (!m_updating) m_creatingWaterAreas = checked;
	});

	m_riverCheck = new QCheckBox("River", this);
	outer->addWidget(m_riverCheck);
	connect(m_riverCheck, &QCheckBox::toggled, this, [this](bool checked) {
		if (m_updating) return;
		PolygonTrigger *theTrigger = getSingleSelectedPolygon();
		if (!theTrigger) return;
		theTrigger->setRiver(checked);
		if (!checked) return;
		Int curPoint = PolygonTool::getSelectedPointNdx();
		if (curPoint < 0) return;
		Real endLen = 0;
		Int newPoint = curPoint;
		if (curPoint>0) curPoint--;
		if (curPoint>0) curPoint--;
		Int i;
		for (i=curPoint; i<theTrigger->getNumPoints()-1 && i<curPoint+4; i++) {
			ICoord3D innerPt = *theTrigger->getPoint(i);
			ICoord3D outerPt = *theTrigger->getPoint(i+1);
			Real dx = innerPt.x-outerPt.x;
			Real dy = innerPt.y-outerPt.y;
			Real curLen = sqrt(dx*dx+dy*dy);
			if (curLen>endLen) {
				newPoint = i;
				endLen = curLen;
			}
		}
		theTrigger->setRiverStart(newPoint);

		// Now find the other end.
		endLen = 0;
		Int endPoint = 0;
		for (i=0; i<theTrigger->getNumPoints()-1; i++) {
			if (i>=newPoint-1 && i<=newPoint+1) continue;
			ICoord3D innerPt = *theTrigger->getPoint(i);
			ICoord3D outerPt = *theTrigger->getPoint(i+1);
			Real dx = innerPt.x-outerPt.x;
			Real dy = innerPt.y-outerPt.y;
			Real curLen = sqrt(dx*dx+dy*dy);
			if (curLen>endLen) {
				endPoint = i;
				endLen = curLen;
			}
		}
		Int pointsOut = endPoint - newPoint;
		Int pointsIn = newPoint - endPoint;
		if (pointsOut<0) pointsOut += theTrigger->getNumPoints();
		if (pointsIn<0) pointsIn += theTrigger->getNumPoints();
		Int delta = pointsIn-pointsOut;
		if (delta<0) delta = -delta;
		if (delta>1) {
			PolygonTrigger *pNew;
			if (pointsOut<pointsIn) {
				pNew = adjustCount(theTrigger, newPoint+1, endPoint, pointsIn-1);
				theTrigger->setRiverStart(pointsIn);
			}	else {
				pNew = adjustCount(theTrigger, endPoint+1, newPoint, pointsOut-1);
				theTrigger->setRiverStart(0);
			}
			while (theTrigger->getNumPoints()) theTrigger->deletePoint(theTrigger->getNumPoints()-1);
			for (i=0; i<pNew->getNumPoints(); i++) {
				theTrigger->addPoint(*pNew->getPoint(i));
			}
			pNew->deleteInstance();
		}
	});

	m_spacingSpin = new QSpinBox(this);
	m_spacingSpin->setRange(1, 4096);
	layout->addRow("Point spacing:", m_spacingSpin);
	connect(m_spacingSpin, &QSpinBox::valueChanged, this, [this](int value) {
		if (!m_updating) m_waterPointSpacing = value;
	});

	outer->addStretch();

	m_staticThis = this;
	m_updating = true;
	m_heightSpin->setValue(m_waterHeight);
	m_spacingSpin->setValue(m_waterPointSpacing);
	m_updating = false;
}

WaterOptions::~WaterOptions()
{
	if (m_staticThis == this) {
		m_staticThis = NULL;
	}
}

/// Finds the polygon trigger currently selected via the Polygon/Water tool.
PolygonTrigger *WaterOptions::getSingleSelectedPolygon(void)
{
	CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
	if (pDoc==NULL) return NULL;
	WbView3d *p3View = pDoc->GetActive3DView();
	Bool showPoly = false;
	if (p3View) {
		showPoly = p3View->isPolygonTriggerVisible();
	}
	if (showPoly || PolygonTool::isActive()) {
		for (PolygonTrigger *pTrig=PolygonTrigger::getFirstPolygonTrigger(); pTrig; pTrig = pTrig->getNext()) {
			if (PolygonTool::isSelected(pTrig)) {
				return pTrig;
			}
		}
	}
	return NULL;
}

void WaterOptions::updateTheUI(void)
{
	m_updating = true;
	PolygonTrigger *theTrigger = getSingleSelectedPolygon();
	if (theTrigger) {
		m_nameEdit->setText(QString::fromUtf8(theTrigger->getTriggerName().str()));
		m_waterHeight = theTrigger->getPoint(0)->z;
		m_heightSpin->setValue(m_waterHeight);
	}
	m_createPolygonCheck->setChecked(m_creatingWaterAreas);
	m_riverCheck->setChecked(theTrigger ? theTrigger->isRiver() : false);
	m_riverCheck->setEnabled(theTrigger != NULL);
	m_spacingSpin->setValue(m_waterPointSpacing);
	m_updating = false;
}

void WaterOptions::update(void)
{
	if (m_staticThis) {
		m_staticThis->updateTheUI();
	}
}

/// Applies a new water height to the selected water polygon (if any) as a single undo step.
void WaterOptions::applyHeight(Int height)
{
	m_waterHeight = height;
	PolygonTrigger *theTrigger = getSingleSelectedPolygon();
	if (!theTrigger || !theTrigger->isWaterArea()) return;
	Int originalHeight = theTrigger->getPoint(0)->z;
	Int dz = height - originalHeight;
	if (dz == 0) return;
	MovePolygonUndoable *pUndo = new MovePolygonUndoable(theTrigger);
	CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
	pDoc->AddAndDoUndoable(pUndo);
	ICoord3D offset = {0, 0, dz};
	pUndo->SetOffset(offset);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	WbView3d *pView = CWorldBuilderDoc::GetActive3DView();
	if (pView) pView->update();
}

/// Resamples [firstPt..lastPt] of trigger down to desiredPointCount points, evenly spaced.
PolygonTrigger *WaterOptions::adjustCount(PolygonTrigger *trigger, Int firstPt, Int lastPt, Int desiredPointCount)
{
	PolygonTrigger *pNew = newInstance(PolygonTrigger)(trigger->getNumPoints());
	Real totalLen = 0;
	Int curPoint = lastPt;
	ICoord3D pt;
	while (curPoint != firstPt) {
		pt = *trigger->getPoint(curPoint);
		pNew->addPoint(pt);
		curPoint++;
		if (curPoint>=trigger->getNumPoints()) {
			curPoint = 0;
		}
	}

	curPoint = firstPt;
	while (curPoint != lastPt) {
		Int nextPoint = curPoint;
		nextPoint++;
		if (nextPoint>=trigger->getNumPoints()) {
			nextPoint = 0;
		}
		ICoord3D curPt = *trigger->getPoint(curPoint);
		ICoord3D nextPt = *trigger->getPoint(nextPoint);
		Real dx = nextPt.x-curPt.x;
		Real dy = nextPt.y-curPt.y;
		Real curLen = sqrt(dx*dx+dy*dy);
		totalLen += curLen;
		curPoint = nextPoint;
	}
	Real spacing = totalLen/(desiredPointCount-1);

	Bool didCurPoint = true;

	curPoint = firstPt;
	pt = *trigger->getPoint(curPoint);
	pNew->addPoint(pt);
	Real curSpacingLen = spacing;
	while (curPoint != lastPt) {
		Int nextPoint = curPoint;
		nextPoint++;
		if (nextPoint>=trigger->getNumPoints()) {
			nextPoint = 0;
		}
		ICoord3D curPt = *trigger->getPoint(curPoint);
		ICoord3D nextPt = *trigger->getPoint(nextPoint);
		Real dx = nextPt.x-curPt.x;
		Real dy = nextPt.y-curPt.y;
		Real curLen = sqrt(dx*dx+dy*dy);
		if (curLen > 4*MAP_XY_FACTOR && curLen>2*spacing) {
			if (!didCurPoint) pNew->addPoint(curPt);
			else pNew->setPoint(curPt, pNew->getNumPoints()-1);
			pNew->addPoint(nextPt);
			didCurPoint = true;
			curSpacingLen = spacing;
		} else if (curSpacingLen>curLen) {
			curSpacingLen -= curLen;
		} else {
			while (curLen >= curSpacingLen) {
				Real factor = curSpacingLen/curLen;
				curPt.x += dx*factor;
				curPt.y += dy*factor;
				pNew->addPoint(curPt);
				didCurPoint = false;
				dx = nextPt.x-curPt.x;
				dy = nextPt.y-curPt.y;
				curLen -= curSpacingLen;
				curSpacingLen = spacing;
			}
			curSpacingLen -= curLen;
			if ((curLen)<MAP_XY_FACTOR/2) {
				didCurPoint = true;
			}
		}
		curPoint = nextPoint;
	}
	return pNew;
}
