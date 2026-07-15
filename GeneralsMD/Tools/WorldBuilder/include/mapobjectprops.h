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
// Port of the core of the MFC IDD_MAPOBJECT_PROPS property page: name, team,
// initial health, behavior flags and angle of the selected object.
// @todo port the remaining pages (weather/time, veterancy, aggressiveness,
// stopping distance, prebuilt upgrades, sound customization).

#pragma once

#include <QWidget>

#include "Lib/BaseType.h"

class MapObject;
class QLineEdit;
class QComboBox;
class QSpinBox;
class QCheckBox;

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

	QLineEdit *m_nameEdit;
	QComboBox *m_teamCombo;
	QSpinBox *m_healthSpin;
	QCheckBox *m_enabledCheck;
	QCheckBox *m_indestructibleCheck;
	QCheckBox *m_unsellableCheck;
	QCheckBox *m_targetableCheck;
	QCheckBox *m_poweredCheck;
	QSpinBox *m_angleSpin;
	Bool m_updating;

	static MapObjectProps *m_staticThis;
};
