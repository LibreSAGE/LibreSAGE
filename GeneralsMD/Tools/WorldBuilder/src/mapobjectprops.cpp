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

// mapobjectprops.cpp : the Object Properties panel (Qt6 port)

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>

#include "mapobjectprops.h"

#include "CUndoable.h"
#include "WorldBuilderDoc.h"

#include "Common/MapObject.h"
#include "Common/WellKnownKeys.h"
#include "GameLogic/SidesList.h"

const char* NEUTRAL_TEAM_UI_STR = "(neutral)";
const char* NEUTRAL_TEAM_INTERNAL_STR = "team";

MapObjectProps *MapObjectProps::m_staticThis = NULL;

//-------------------------------------------------------------------------------------------------
// Apply a single dict item change to all selected objects through the undo system.
static void changeSelectedDictItems(const Dict &data, NameKeyType key)
{
	CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
	if (pDoc == NULL) {
		return;
	}
	// Collect the dicts of the selected objects.
	std::vector<Dict *> dicts;
	for (MapObject *pObj = MapObject::getFirstMapObject(); pObj; pObj = pObj->getNext()) {
		if (pObj->isSelected()) {
			dicts.push_back(pObj->getProperties());
		}
	}
	if (dicts.empty()) {
		return;
	}
	DictItemUndoable *pUndo = new DictItemUndoable(&dicts[0], data, key, (Int)dicts.size(), pDoc, true);
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
}

//-------------------------------------------------------------------------------------------------
MapObjectProps::MapObjectProps(QWidget *parent) :
	QWidget(parent),
	m_selectedObject(NULL),
	m_updating(false)
{
	QFormLayout *layout = new QFormLayout(this);

	m_nameEdit = new QLineEdit(this);
	layout->addRow("Name:", m_nameEdit);

	m_teamCombo = new QComboBox(this);
	layout->addRow("Team:", m_teamCombo);

	m_healthSpin = new QSpinBox(this);
	m_healthSpin->setRange(0, 100);
	m_healthSpin->setSuffix("%");
	layout->addRow("Initial Health:", m_healthSpin);

	m_enabledCheck = new QCheckBox("Enabled", this);
	layout->addRow(m_enabledCheck);
	m_indestructibleCheck = new QCheckBox("Indestructible", this);
	layout->addRow(m_indestructibleCheck);
	m_unsellableCheck = new QCheckBox("Unsellable", this);
	layout->addRow(m_unsellableCheck);
	m_targetableCheck = new QCheckBox("Targetable", this);
	layout->addRow(m_targetableCheck);
	m_poweredCheck = new QCheckBox("Powered", this);
	layout->addRow(m_poweredCheck);

	m_angleSpin = new QSpinBox(this);
	m_angleSpin->setRange(-180, 180);
	m_angleSpin->setSuffix("\xc2\xb0");
	layout->addRow("Angle:", m_angleSpin);

	connect(m_nameEdit, &QLineEdit::editingFinished, this, [this]() {
		if (m_updating || m_selectedObject == NULL) return;
		Dict data;
		data.setAsciiString(TheKey_objectName, AsciiString(m_nameEdit->text().toUtf8().constData()));
		changeSelectedDictItems(data, TheKey_objectName);
	});
	connect(m_teamCombo, &QComboBox::activated, this, [this](int index) {
		if (m_updating || index < 0) return;
		QString team = m_teamCombo->itemText(index);
		AsciiString name(team.toUtf8().constData());
		if (team == NEUTRAL_TEAM_UI_STR) {
			name = NEUTRAL_TEAM_INTERNAL_STR;
		}
		Dict data;
		data.setAsciiString(TheKey_originalOwner, name);
		changeSelectedDictItems(data, TheKey_originalOwner);
	});
	connect(m_healthSpin, &QSpinBox::valueChanged, this, [this](int value) {
		if (m_updating || m_selectedObject == NULL) return;
		Dict data;
		data.setInt(TheKey_objectInitialHealth, value);
		changeSelectedDictItems(data, TheKey_objectInitialHealth);
	});
	struct { QCheckBox *box; NameKeyType key; } flagRows[] = {
		{ m_enabledCheck, TheKey_objectEnabled },
		{ m_indestructibleCheck, TheKey_objectIndestructible },
		{ m_unsellableCheck, TheKey_objectUnsellable },
		{ m_targetableCheck, TheKey_objectTargetable },
		{ m_poweredCheck, TheKey_objectPowered },
	};
	for (size_t i = 0; i < sizeof(flagRows)/sizeof(flagRows[0]); ++i) {
		QCheckBox *box = flagRows[i].box;
		NameKeyType key = flagRows[i].key;
		connect(box, &QCheckBox::toggled, this, [this, key](bool checked) {
			if (m_updating || m_selectedObject == NULL) return;
			Dict data;
			data.setBool(key, checked);
			changeSelectedDictItems(data, key);
		});
	}
	connect(m_angleSpin, &QSpinBox::valueChanged, this, [this](int value) {
		if (m_updating || m_selectedObject == NULL) return;
		CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
		if (pDoc == NULL) return;
		ModifyObjectUndoable *pUndo = new ModifyObjectUndoable(pDoc);
		pUndo->RotateTo(value * PI / 180.0f);
		pDoc->AddAndDoUndoable(pUndo);
		REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	});

	m_staticThis = this;
	refresh();
}

MapObjectProps::~MapObjectProps()
{
	if (m_staticThis == this) {
		m_staticThis = NULL;
	}
}

//-------------------------------------------------------------------------------------------------
MapObject *MapObjectProps::getSingleSelectedMapObject(void)
{
	MapObject *selected = NULL;
	for (MapObject *pObj = MapObject::getFirstMapObject(); pObj; pObj = pObj->getNext()) {
		if (pObj->isSelected()) {
			if (selected) {
				return NULL; // more than one.
			}
			selected = pObj;
		}
	}
	return selected;
}

//-------------------------------------------------------------------------------------------------
void MapObjectProps::update(void)
{
	if (m_staticThis) {
		m_staticThis->refresh();
	}
}

//-------------------------------------------------------------------------------------------------
void MapObjectProps::refresh(void)
{
	m_updating = true;
	m_selectedObject = getSingleSelectedMapObject();

	// Team list from the sides list.
	m_teamCombo->clear();
	for (Int i=0; i<TheSidesList->getNumTeams(); i++) {
		AsciiString name = TheSidesList->getTeamInfo(i)->getDict()->getAsciiString(TheKey_teamName);
		if (name == NEUTRAL_TEAM_INTERNAL_STR) {
			name = NEUTRAL_TEAM_UI_STR;
		}
		m_teamCombo->addItem(QString::fromUtf8(name.str()));
	}

	MapObject *pObj = m_selectedObject;
	setEnabled(pObj != NULL);
	if (pObj == NULL) {
		m_nameEdit->clear();
		m_updating = false;
		return;
	}

	Bool exists;
	Dict *props = pObj->getProperties();

	m_nameEdit->setText(QString::fromUtf8(props->getAsciiString(TheKey_objectName, &exists).str()));

	AsciiString team = props->getAsciiString(TheKey_originalOwner, &exists);
	if (team == NEUTRAL_TEAM_INTERNAL_STR) {
		team = NEUTRAL_TEAM_UI_STR;
	}
	Int teamNdx = m_teamCombo->findText(QString::fromUtf8(team.str()));
	m_teamCombo->setCurrentIndex(teamNdx);

	Int health = props->getInt(TheKey_objectInitialHealth, &exists);
	m_healthSpin->setValue(exists ? health : 100);

	Bool val;
	val = props->getBool(TheKey_objectEnabled, &exists);
	m_enabledCheck->setChecked(!exists || val);
	val = props->getBool(TheKey_objectIndestructible, &exists);
	m_indestructibleCheck->setChecked(exists && val);
	val = props->getBool(TheKey_objectUnsellable, &exists);
	m_unsellableCheck->setChecked(exists && val);
	val = props->getBool(TheKey_objectTargetable, &exists);
	m_targetableCheck->setChecked(exists && val);
	val = props->getBool(TheKey_objectPowered, &exists);
	m_poweredCheck->setChecked(!exists || val);

	m_angleSpin->setValue((Int)(pObj->getAngle() * 180.0f / PI));

	m_updating = false;
}
