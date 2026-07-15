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

// mapobjectprops.cpp : the Object Properties panel (Qt6 port).  Layout mirrors
// the MFC IDD_MAPOBJECT_PROPS page: General / Logical / Visual / Sound /
// Pre-built upgrades groups.

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QSpinBox>
#include <QVBoxLayout>

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

static void writeInt(NameKeyType key, Int value)
{
	Dict data;
	data.setInt(key, value);
	changeSelectedDictItems(data, key);
}

static void writeReal(NameKeyType key, Real value)
{
	Dict data;
	data.setReal(key, value);
	changeSelectedDictItems(data, key);
}

static void writeBool(NameKeyType key, Bool value)
{
	Dict data;
	data.setBool(key, value);
	changeSelectedDictItems(data, key);
}

//-------------------------------------------------------------------------------------------------
MapObjectProps::MapObjectProps(QWidget *parent) :
	QWidget(parent),
	m_selectedObject(NULL),
	m_updating(false)
{
	QVBoxLayout *root = new QVBoxLayout(this);
	root->setContentsMargins(4, 4, 4, 4);
	root->setSpacing(4);

	// --- General ------------------------------------------------------------
	{
		QGroupBox *box = new QGroupBox("General", this);
		QFormLayout *form = new QFormLayout(box);
		m_nameEdit = new QLineEdit(box);
		form->addRow("Name:", m_nameEdit);
		m_teamCombo = new QComboBox(box);
		form->addRow("Team:", m_teamCombo);
		root->addWidget(box);
	}

	// --- Logical ------------------------------------------------------------
	{
		QGroupBox *box = new QGroupBox("Logical", this);
		QGridLayout *grid = new QGridLayout(box);

		// Left column: value controls.
		grid->addWidget(new QLabel("Initial Health %:", box), 0, 0);
		m_healthSpin = new QSpinBox(box);
		m_healthSpin->setRange(0, 100);
		m_healthSpin->setSuffix("%");
		grid->addWidget(m_healthSpin, 0, 1);

		grid->addWidget(new QLabel("Max HP:", box), 1, 0);
		m_maxHPCombo = new QComboBox(box);
		m_maxHPCombo->setEditable(true);
		m_maxHPCombo->addItem("Default For Unit");
		grid->addWidget(m_maxHPCombo, 1, 1);

		grid->addWidget(new QLabel("Aggressiveness:", box), 2, 0);
		m_aggressivenessCombo = new QComboBox(box);
		m_aggressivenessCombo->addItem("Sleep", -2);
		m_aggressivenessCombo->addItem("Passive", -1);
		m_aggressivenessCombo->addItem("Normal", 0);
		m_aggressivenessCombo->addItem("Alert", 1);
		m_aggressivenessCombo->addItem("Aggressive", 2);
		grid->addWidget(m_aggressivenessCombo, 2, 1);

		grid->addWidget(new QLabel("Veterancy:", box), 3, 0);
		m_veterancyCombo = new QComboBox(box);
		m_veterancyCombo->addItems(QStringList() << "Normal" << "Veteran" << "Elite" << "Heroic");
		grid->addWidget(m_veterancyCombo, 3, 1);

		// Distance row.
		QHBoxLayout *dist = new QHBoxLayout();
		dist->addWidget(new QLabel("Stopping:", box));
		m_stoppingEdit = new QLineEdit(box);
		m_stoppingEdit->setMaximumWidth(48);
		dist->addWidget(m_stoppingEdit);
		dist->addWidget(new QLabel("Targeting:", box));
		m_targetingEdit = new QLineEdit(box);
		m_targetingEdit->setMaximumWidth(48);
		dist->addWidget(m_targetingEdit);
		dist->addWidget(new QLabel("Shroud:", box));
		m_shroudEdit = new QLineEdit(box);
		m_shroudEdit->setMaximumWidth(48);
		dist->addWidget(m_shroudEdit);
		grid->addLayout(dist, 4, 0, 1, 2);

		// Right column: flag checkboxes.
		QVBoxLayout *flags = new QVBoxLayout();
		m_enabledCheck = new QCheckBox("Enabled", box);
		m_unsellableCheck = new QCheckBox("Unsellable", box);
		m_targetableCheck = new QCheckBox("Targetable", box);
		m_indestructibleCheck = new QCheckBox("Indestructible", box);
		m_aiRecruitableCheck = new QCheckBox("AI Recruitable", box);
		m_poweredCheck = new QCheckBox("Powered", box);
		m_selectableCheck = new QCheckBox("Selectable", box);
		flags->addWidget(m_enabledCheck);
		flags->addWidget(m_unsellableCheck);
		flags->addWidget(m_targetableCheck);
		flags->addWidget(m_indestructibleCheck);
		flags->addWidget(m_aiRecruitableCheck);
		flags->addWidget(m_poweredCheck);
		flags->addWidget(m_selectableCheck);
		grid->addLayout(flags, 0, 2, 5, 1);

		root->addWidget(box);
	}

	// --- Visual -------------------------------------------------------------
	{
		QGroupBox *box = new QGroupBox("Visual", this);
		QGridLayout *grid = new QGridLayout(box);

		grid->addWidget(new QLabel("XY Pos:", box), 0, 0);
		m_xyPosEdit = new QLineEdit(box);
		m_xyPosEdit->setReadOnly(true);
		grid->addWidget(m_xyPosEdit, 0, 1);
		grid->addWidget(new QLabel("Z:", box), 0, 2);
		m_zEdit = new QLineEdit(box);
		m_zEdit->setReadOnly(true);
		m_zEdit->setMaximumWidth(64);
		grid->addWidget(m_zEdit, 0, 3);

		grid->addWidget(new QLabel("Weather:", box), 1, 0);
		m_weatherCombo = new QComboBox(box);
		m_weatherCombo->addItems(QStringList() << "Use Map Weather" << "Normal" << "Snowy");
		grid->addWidget(m_weatherCombo, 1, 1);
		grid->addWidget(new QLabel("Angle:", box), 1, 2);
		m_angleSpin = new QDoubleSpinBox(box);
		m_angleSpin->setRange(-180.0, 180.0);
		m_angleSpin->setDecimals(2);
		m_angleSpin->setSuffix("\xc2\xb0");
		grid->addWidget(m_angleSpin, 1, 3);

		grid->addWidget(new QLabel("Time:", box), 2, 0);
		m_timeCombo = new QComboBox(box);
		m_timeCombo->addItems(QStringList() << "Use Map Time" << "Morning" << "Afternoon" << "Evening" << "Night");
		grid->addWidget(m_timeCombo, 2, 1);

		root->addWidget(box);
	}

	// --- Sound --------------------------------------------------------------
	{
		QGroupBox *box = new QGroupBox("Sound", this);
		QGridLayout *grid = new QGridLayout(box);

		grid->addWidget(new QLabel("Attached Sound:", box), 0, 0);
		m_soundCombo = new QComboBox(box);
		m_soundCombo->addItem("Default <None>");	/// @todo populate from TheAudio events.
		grid->addWidget(m_soundCombo, 0, 1, 1, 3);

		m_customizeCheck = new QCheckBox("Customize", box);
		m_soundEnabledCheck = new QCheckBox("Enabled", box);
		m_loopingCheck = new QCheckBox("Looping", box);
		grid->addWidget(m_customizeCheck, 1, 0);
		grid->addWidget(m_soundEnabledCheck, 1, 1);
		grid->addWidget(m_loopingCheck, 1, 2);

		grid->addWidget(new QLabel("Loop Count:", box), 2, 0);
		m_loopCountSpin = new QSpinBox(box);
		m_loopCountSpin->setRange(0, 100000);
		grid->addWidget(m_loopCountSpin, 2, 1);
		grid->addWidget(new QLabel("Priority:", box), 2, 2);
		m_priorityCombo = new QComboBox(box);
		m_priorityCombo->addItems(QStringList() << "LOWEST" << "LOW" << "NORMAL" << "HIGH" << "CRITICAL");
		grid->addWidget(m_priorityCombo, 2, 3);

		grid->addWidget(new QLabel("Volume:", box), 3, 0);
		m_volumeSpin = new QSpinBox(box);
		m_volumeSpin->setRange(0, 100);
		grid->addWidget(m_volumeSpin, 3, 1);
		grid->addWidget(new QLabel("Min Volume:", box), 3, 2);
		m_minVolumeSpin = new QSpinBox(box);
		m_minVolumeSpin->setRange(0, 100);
		grid->addWidget(m_minVolumeSpin, 3, 3);

		grid->addWidget(new QLabel("Min Range:", box), 4, 0);
		m_minRangeSpin = new QSpinBox(box);
		m_minRangeSpin->setRange(0, 100000);
		grid->addWidget(m_minRangeSpin, 4, 1);
		grid->addWidget(new QLabel("Max Range:", box), 4, 2);
		m_maxRangeSpin = new QSpinBox(box);
		m_maxRangeSpin->setRange(0, 100000);
		grid->addWidget(m_maxRangeSpin, 4, 3);

		root->addWidget(box);
	}

	// --- Pre-built upgrades -------------------------------------------------
	{
		QGroupBox *box = new QGroupBox("Pre-built upgrades", this);
		QVBoxLayout *v = new QVBoxLayout(box);
		m_prebuiltList = new QListWidget(box);
		m_prebuiltList->setMaximumHeight(56);
		v->addWidget(m_prebuiltList);
		root->addWidget(box);
	}

	root->addWidget(new QLabel(
		"Hint: Use SHIFT key to select or deselect multiple units", this));
	root->addStretch(1);

	// --- Wiring -------------------------------------------------------------
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
		if (team == NEUTRAL_TEAM_UI_STR) name = NEUTRAL_TEAM_INTERNAL_STR;
		Dict data;
		data.setAsciiString(TheKey_originalOwner, name);
		changeSelectedDictItems(data, TheKey_originalOwner);
	});
	connect(m_healthSpin, &QSpinBox::valueChanged, this, [this](int v) {
		if (m_updating || m_selectedObject == NULL) return;
		writeInt(TheKey_objectInitialHealth, v);
	});
	connect(m_maxHPCombo, &QComboBox::currentTextChanged, this, [this](const QString &text) {
		if (m_updating || m_selectedObject == NULL) return;
		bool ok = false;
		int hp = text.toInt(&ok);
		writeInt(TheKey_objectMaxHPs, ok ? hp : 0);
	});
	connect(m_aggressivenessCombo, &QComboBox::activated, this, [this](int index) {
		if (m_updating || index < 0) return;
		writeInt(TheKey_objectAggressiveness, m_aggressivenessCombo->itemData(index).toInt());
	});
	connect(m_veterancyCombo, &QComboBox::activated, this, [this](int index) {
		if (m_updating || index < 0) return;
		writeInt(TheKey_objectVeterancy, index);
	});
	connect(m_stoppingEdit, &QLineEdit::editingFinished, this, [this]() {
		if (m_updating || m_selectedObject == NULL) return;
		writeReal(TheKey_objectStoppingDistance, (Real)m_stoppingEdit->text().toDouble());
	});
	connect(m_targetingEdit, &QLineEdit::editingFinished, this, [this]() {
		if (m_updating || m_selectedObject == NULL) return;
		writeInt(TheKey_objectVisualRange, m_targetingEdit->text().toInt());
	});
	connect(m_shroudEdit, &QLineEdit::editingFinished, this, [this]() {
		if (m_updating || m_selectedObject == NULL) return;
		writeInt(TheKey_objectShroudClearingDistance, m_shroudEdit->text().toInt());
	});

	struct { QCheckBox *box; NameKeyType key; } flagRows[] = {
		{ m_enabledCheck, TheKey_objectEnabled },
		{ m_unsellableCheck, TheKey_objectUnsellable },
		{ m_targetableCheck, TheKey_objectTargetable },
		{ m_indestructibleCheck, TheKey_objectIndestructible },
		{ m_aiRecruitableCheck, TheKey_objectRecruitableAI },
		{ m_poweredCheck, TheKey_objectPowered },
		{ m_selectableCheck, TheKey_objectSelectable },
	};
	for (size_t i = 0; i < sizeof(flagRows)/sizeof(flagRows[0]); ++i) {
		QCheckBox *cb = flagRows[i].box;
		NameKeyType key = flagRows[i].key;
		connect(cb, &QCheckBox::toggled, this, [this, key](bool checked) {
			if (m_updating || m_selectedObject == NULL) return;
			writeBool(key, checked);
		});
	}

	connect(m_weatherCombo, &QComboBox::activated, this, [this](int index) {
		if (m_updating || index < 0) return;
		writeInt(TheKey_objectWeather, index);
	});
	connect(m_timeCombo, &QComboBox::activated, this, [this](int index) {
		if (m_updating || index < 0) return;
		writeInt(TheKey_objectTime, index);
	});
	connect(m_angleSpin, &QDoubleSpinBox::valueChanged, this, [this](double value) {
		if (m_updating || m_selectedObject == NULL) return;
		CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
		if (pDoc == NULL) return;
		ModifyObjectUndoable *pUndo = new ModifyObjectUndoable(pDoc);
		pUndo->RotateTo(value * PI / 180.0f);
		pDoc->AddAndDoUndoable(pUndo);
		REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	});

	// Sound customization fields.
	connect(m_customizeCheck, &QCheckBox::toggled, this, [this](bool c) {
		if (m_updating || m_selectedObject == NULL) return; writeBool(TheKey_objectSoundAmbientCustomized, c); });
	connect(m_soundEnabledCheck, &QCheckBox::toggled, this, [this](bool c) {
		if (m_updating || m_selectedObject == NULL) return; writeBool(TheKey_objectSoundAmbientEnabled, c); });
	connect(m_loopingCheck, &QCheckBox::toggled, this, [this](bool c) {
		if (m_updating || m_selectedObject == NULL) return; writeBool(TheKey_objectSoundAmbientLooping, c); });
	connect(m_loopCountSpin, &QSpinBox::valueChanged, this, [this](int v) {
		if (m_updating || m_selectedObject == NULL) return; writeInt(TheKey_objectSoundAmbientLoopCount, v); });
	connect(m_priorityCombo, &QComboBox::activated, this, [this](int index) {
		if (m_updating || index < 0) return; writeInt(TheKey_objectSoundAmbientPriority, index); });
	connect(m_volumeSpin, &QSpinBox::valueChanged, this, [this](int v) {
		if (m_updating || m_selectedObject == NULL) return; writeInt(TheKey_objectSoundAmbientVolume, v); });
	connect(m_minVolumeSpin, &QSpinBox::valueChanged, this, [this](int v) {
		if (m_updating || m_selectedObject == NULL) return; writeInt(TheKey_objectSoundAmbientMinVolume, v); });
	connect(m_minRangeSpin, &QSpinBox::valueChanged, this, [this](int v) {
		if (m_updating || m_selectedObject == NULL) return; writeInt(TheKey_objectSoundAmbientMinRange, v); });
	connect(m_maxRangeSpin, &QSpinBox::valueChanged, this, [this](int v) {
		if (m_updating || m_selectedObject == NULL) return; writeInt(TheKey_objectSoundAmbientMaxRange, v); });

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
static void setEditInt(QLineEdit *edit, Dict *props, NameKeyType key)
{
	Bool exists;
	Int value = props->getInt(key, &exists);
	edit->setText(exists ? QString::number(value) : QString());
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
		if (name == NEUTRAL_TEAM_INTERNAL_STR) name = NEUTRAL_TEAM_UI_STR;
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

	// General.
	m_nameEdit->setText(QString::fromUtf8(props->getAsciiString(TheKey_objectName, &exists).str()));
	AsciiString team = props->getAsciiString(TheKey_originalOwner, &exists);
	if (team == NEUTRAL_TEAM_INTERNAL_STR) team = NEUTRAL_TEAM_UI_STR;
	m_teamCombo->setCurrentIndex(m_teamCombo->findText(QString::fromUtf8(team.str())));

	// Logical.
	Int health = props->getInt(TheKey_objectInitialHealth, &exists);
	m_healthSpin->setValue(exists ? health : 100);

	Int maxHP = props->getInt(TheKey_objectMaxHPs, &exists);
	m_maxHPCombo->setCurrentText(exists && maxHP > 0 ? QString::number(maxHP) : QString("Default For Unit"));

	Int aggr = props->getInt(TheKey_objectAggressiveness, &exists);
	m_aggressivenessCombo->setCurrentIndex(m_aggressivenessCombo->findData(exists ? aggr : 0));

	Int vet = props->getInt(TheKey_objectVeterancy, &exists);
	m_veterancyCombo->setCurrentIndex(exists ? vet : 0);

	Real stopping = props->getReal(TheKey_objectStoppingDistance, &exists);
	m_stoppingEdit->setText(exists ? QString::number(stopping) : QString());
	setEditInt(m_targetingEdit, props, TheKey_objectVisualRange);
	setEditInt(m_shroudEdit, props, TheKey_objectShroudClearingDistance);

	Bool val;
	val = props->getBool(TheKey_objectEnabled, &exists);       m_enabledCheck->setChecked(!exists || val);
	val = props->getBool(TheKey_objectUnsellable, &exists);    m_unsellableCheck->setChecked(exists && val);
	val = props->getBool(TheKey_objectTargetable, &exists);    m_targetableCheck->setChecked(!exists || val);
	val = props->getBool(TheKey_objectIndestructible, &exists);m_indestructibleCheck->setChecked(exists && val);
	val = props->getBool(TheKey_objectRecruitableAI, &exists); m_aiRecruitableCheck->setChecked(!exists || val);
	val = props->getBool(TheKey_objectPowered, &exists);       m_poweredCheck->setChecked(!exists || val);
	val = props->getBool(TheKey_objectSelectable, &exists);    m_selectableCheck->setChecked(!exists || val);

	// Visual.
	const Coord3D *loc = pObj->getLocation();
	m_xyPosEdit->setText(QString("%1, %2").arg(loc->x, 0, 'f', 2).arg(loc->y, 0, 'f', 2));
	m_zEdit->setText(QString::number(loc->z, 'f', 2));
	Int weather = props->getInt(TheKey_objectWeather, &exists);
	m_weatherCombo->setCurrentIndex(exists ? weather : 0);
	Int time = props->getInt(TheKey_objectTime, &exists);
	m_timeCombo->setCurrentIndex(exists ? time : 0);
	m_angleSpin->setValue(pObj->getAngle() * 180.0f / PI);

	// Sound.
	m_soundCombo->setCurrentText(QString::fromUtf8(props->getAsciiString(TheKey_objectSoundAmbient, &exists).str()));
	val = props->getBool(TheKey_objectSoundAmbientCustomized, &exists); m_customizeCheck->setChecked(exists && val);
	val = props->getBool(TheKey_objectSoundAmbientEnabled, &exists);    m_soundEnabledCheck->setChecked(exists && val);
	val = props->getBool(TheKey_objectSoundAmbientLooping, &exists);    m_loopingCheck->setChecked(exists && val);
	m_loopCountSpin->setValue(props->getInt(TheKey_objectSoundAmbientLoopCount, &exists));
	Int prio = props->getInt(TheKey_objectSoundAmbientPriority, &exists);
	m_priorityCombo->setCurrentIndex(exists ? prio : 0);
	m_volumeSpin->setValue(props->getInt(TheKey_objectSoundAmbientVolume, &exists));
	m_minVolumeSpin->setValue(props->getInt(TheKey_objectSoundAmbientMinVolume, &exists));
	m_minRangeSpin->setValue(props->getInt(TheKey_objectSoundAmbientMinRange, &exists));
	m_maxRangeSpin->setValue(props->getInt(TheKey_objectSoundAmbientMaxRange, &exists));

	// Pre-built upgrades (display-only for now).
	m_prebuiltList->clear();
	m_prebuiltList->addItem("Single Selection Only");

	m_updating = false;
}
