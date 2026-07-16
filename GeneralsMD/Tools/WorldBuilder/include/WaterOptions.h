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

// WaterOptions.h : options panel (Qt6 port of the MFC dialog)

#pragma once

#include <QWidget>

#include "Lib/BaseType.h"

class QLineEdit;
class QSpinBox;
class QCheckBox;
class PolygonTrigger;
class MovePolygonUndoable;

class WaterOptions : public QWidget
{
	Q_OBJECT

public:
	WaterOptions(QWidget *parent = NULL);
	~WaterOptions() override;

	/// Refreshes the panel from the currently selected water polygon (if any).
	static void update(void);

	static Int getHeight(void) { return m_waterHeight; }
	static Int getSpacing(void) { return m_waterPointSpacing; }
	static Bool getCreatingWaterAreas(void) { return m_creatingWaterAreas; }

protected:
	static PolygonTrigger *getSingleSelectedPolygon(void);

	void updateTheUI(void);
	void applyHeight(Int height);
	PolygonTrigger *adjustCount(PolygonTrigger *trigger, Int firstPt, Int lastPt, Int desiredPointCount);

protected:
	QLineEdit	*m_nameEdit;
	QSpinBox	*m_heightSpin;
	QSpinBox	*m_spacingSpin;
	QCheckBox	*m_createPolygonCheck;
	QCheckBox	*m_riverCheck;

	Bool		m_updating; ///< true if the ui is updating itself.

	static WaterOptions *m_staticThis;
	static Int			 m_waterHeight;
	static Int			 m_waterPointSpacing;
	static Bool			 m_creatingWaterAreas; ///< True if we are creating flood fill water polygons, rather than dropping single points.
};
