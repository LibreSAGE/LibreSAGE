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

// mapobjectprops.h : the Object Properties panel (Qt6 port)
//
// Port of the MFC IDD_MAPOBJECT_PROPS property page.  Laid out in the same
// General / Logical / Visual / Sound / Pre-built-upgrades groups as the
// original.  @todo: the pre-built-upgrades list and the attached-sound name
// picker are display-only for now.

#pragma once

#include <QWidget>

#include "Lib/BaseType.h"

class MapObject;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QSpinBox;

extern const char* NEUTRAL_TEAM_UI_STR;
extern const char* NEUTRAL_TEAM_INTERNAL_STR;

class MapObjectProps : public QWidget
{
	Q_OBJECT

public:
	MapObjectProps(QWidget *parent = NULL);
	~MapObjectProps() override;

	/// Refresh the panel from the current selection (called by the tools).
	static void update(void);

	/// The object shown in the panel: exactly one selected map object, else NULL.
	static MapObject *getSingleSelectedMapObject(void);

protected:
	void refresh(void);
	MapObject *m_selectedObject;
	Bool m_updating;

	// General ----------------------------------------------------------------
	QLineEdit *m_nameEdit;
	QComboBox *m_teamCombo;

	// Logical ----------------------------------------------------------------
	QSpinBox  *m_healthSpin;
	QComboBox *m_maxHPCombo;
	QComboBox *m_aggressivenessCombo;
	QComboBox *m_veterancyCombo;
	QLineEdit *m_stoppingEdit;
	QLineEdit *m_targetingEdit;
	QLineEdit *m_shroudEdit;
	QCheckBox *m_enabledCheck;
	QCheckBox *m_unsellableCheck;
	QCheckBox *m_targetableCheck;
	QCheckBox *m_indestructibleCheck;
	QCheckBox *m_aiRecruitableCheck;
	QCheckBox *m_poweredCheck;
	QCheckBox *m_selectableCheck;

	// Visual -----------------------------------------------------------------
	QLineEdit      *m_xyPosEdit;
	QLineEdit      *m_zEdit;
	QComboBox      *m_weatherCombo;
	QDoubleSpinBox *m_angleSpin;
	QComboBox      *m_timeCombo;

	// Sound ------------------------------------------------------------------
	QComboBox *m_soundCombo;
	QCheckBox *m_customizeCheck;
	QCheckBox *m_soundEnabledCheck;
	QCheckBox *m_loopingCheck;
	QSpinBox  *m_loopCountSpin;
	QComboBox *m_priorityCombo;
	QSpinBox  *m_volumeSpin;
	QSpinBox  *m_minVolumeSpin;
	QSpinBox  *m_minRangeSpin;
	QSpinBox  *m_maxRangeSpin;

	// Pre-built upgrades -----------------------------------------------------
	QListWidget *m_prebuiltList;

	static MapObjectProps *m_staticThis;
};
