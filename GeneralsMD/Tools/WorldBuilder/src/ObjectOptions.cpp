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

// ObjectOptions.cpp : the object-placement palette (Qt6 port of the MFC dialog).
//
// Qt6 port note: the object preview render window is not ported yet.  @todo

#define DEFINE_EDITOR_SORTING_NAMES

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QTreeWidget>
#include <QVBoxLayout>

#include "ObjectOptions.h"

#include "addplayerdialog.h"
#include "ObjectPreview.h"
#include "Tool.h"				// MAGIC_GROUND_Z
#include "Common/MapObject.h"	// MAP_HEIGHT_SCALE, MapObject
#include "Common/WellKnownKeys.h"
#include "Common/ThingTemplate.h"
#include "Common/ThingFactory.h"
#include "Common/ThingSort.h"
#include "Common/PlayerTemplate.h"
#include "GameLogic/SidesList.h"

ObjectOptions *ObjectOptions::m_staticThis = NULL;
Bool ObjectOptions::m_updating = false;
char ObjectOptions::m_currentObjectName[NAME_MAX_LEN];
Int ObjectOptions::m_currentObjectIndex = -1;
AsciiString ObjectOptions::m_curOwnerName;

/////////////////////////////////////////////////////////////////////////////
static Int findSideListEntryWithPlayerOfSide(AsciiString side)
{
	for (int i = 0; i < TheSidesList->getNumSides(); i++)
	{
		AsciiString ptname = TheSidesList->getSideInfo(i)->getDict()->getAsciiString(TheKey_playerFaction);
		const PlayerTemplate* pt = ThePlayerTemplateStore->findPlayerTemplate(NAMEKEY(ptname));
		if (pt && pt->getSide() == side)
		{
			return i;
		}
	}
	return -1;
}

/////////////////////////////////////////////////////////////////////////////
// construction / destruction

ObjectOptions::ObjectOptions(QWidget *parent) :
	QWidget(parent),
	m_objectName(NULL),
	m_objectTree(NULL),
	m_heightSpin(NULL),
	m_owningTeam(NULL),
	m_objectsList(NULL)
{
	m_updating = true;
	strcpy(m_currentObjectName, "No Selection");
	m_curOwnerName.clear();

	// Layout mirrors the MFC "Object Selection Opts" panel: the object tree
	// fills the top, then the object name, then a bottom row with the owner /
	// height controls on the left and the 3d preview thumbnail on the right.
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setContentsMargins(4, 4, 4, 4);

	m_objectTree = new QTreeWidget(this);
	m_objectTree->setHeaderHidden(true);
	m_objectTree->setColumnCount(1);
	layout->addWidget(m_objectTree, 1);

	m_objectName = new QLabel("Object Name:", this);
	layout->addWidget(m_objectName);

	QHBoxLayout *bottomRow = new QHBoxLayout();

	QVBoxLayout *ownerCol = new QVBoxLayout();
	QLabel *ownerLabel = new QLabel(
		"Default Owner: ( In skirmish games, Objects assigned to Skirmish "
		"players will be made neutral.)", this);
	ownerLabel->setWordWrap(true);
	ownerCol->addWidget(ownerLabel);

	m_owningTeam = new QComboBox(this);
	ownerCol->addWidget(m_owningTeam);

	QHBoxLayout *heightRow = new QHBoxLayout();
	heightRow->addWidget(new QLabel("Height:", this));
	m_heightSpin = new QSpinBox(this);
	m_heightSpin->setRange(-255, 255);
	m_heightSpin->setValue(MAGIC_GROUND_Z);
	heightRow->addWidget(m_heightSpin);
	heightRow->addStretch(1);
	ownerCol->addLayout(heightRow);
	ownerCol->addStretch(1);

	bottomRow->addLayout(ownerCol, 1);

	m_preview = new ObjectPreview(this);
	m_preview->setFixedSize(72, 72);
	bottomRow->addWidget(m_preview, 0, Qt::AlignTop);

	layout->addLayout(bottomRow);

	buildObjectList();
	buildTree();

	connect(m_objectTree, &QTreeWidget::itemSelectionChanged,
			this, &ObjectOptions::onTreeSelectionChanged);
	connect(m_owningTeam, QOverload<int>::of(&QComboBox::currentIndexChanged),
			this, &ObjectOptions::onOwningTeamChanged);

	m_staticThis = this;
	m_updating = false;

	updateLabel();
}

ObjectOptions::~ObjectOptions(void)
{
	if (m_staticThis == this)
		m_staticThis = NULL;
	if (m_objectsList) {
		m_objectsList->deleteInstance();
	}
	m_objectsList = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// object list / tree construction

void ObjectOptions::buildObjectList(void)
{
	// One MapObject per available thing-factory template, kept in a linked list.
	const ThingTemplate *tTemplate;
	for (tTemplate = TheThingFactory->firstTemplate();
			 tTemplate;
			 tTemplate = tTemplate->friend_getNextTemplate())
	{
		Coord3D loc = { 0, 0, 0 };
		MapObject *pMap = newInstance(MapObject)(loc, tTemplate->getName(), 0.0f, 0, NULL, tTemplate);
		pMap->setNextMap(m_objectsList);
		m_objectsList = pMap;

		Color cc = tTemplate->getDisplayColor();
		pMap->setColor(cc);
	}
}

/** Find the child of parent with the given label, adding it if absent. */
QTreeWidgetItem *ObjectOptions::findOrAdd(QTreeWidgetItem *parent, const char *label)
{
	QString text = QString::fromUtf8(label);
	int count = parent ? parent->childCount() : m_objectTree->topLevelItemCount();
	for (int i = 0; i < count; i++) {
		QTreeWidgetItem *child = parent ? parent->child(i) : m_objectTree->topLevelItem(i);
		if (child->text(0) == text && child->data(0, Qt::UserRole).toInt() < 0) {
			return child;
		}
	}
	QTreeWidgetItem *child = parent ? new QTreeWidgetItem(parent)
									: new QTreeWidgetItem(m_objectTree);
	child->setText(0, text);
	child->setData(0, Qt::UserRole, -1);	// -1 marks an interior (grouping) node.
	return child;
}

void ObjectOptions::buildTree(void)
{
	Int index = 0;
	for (MapObject *pMap = m_objectsList; pMap; pMap = pMap->getNext(), index++) {
		const ThingTemplate *tt = pMap->getThingTemplate();
		QTreeWidgetItem *parent = NULL;

		if (tt) {
			if (tt->getEditorSorting() == ES_TEST)
				parent = findOrAdd(parent, "TEST");

			AsciiString side = tt->getDefaultOwningSide();
			parent = findOrAdd(parent, side.isEmpty() ? "(neutral)" : side.str());

			EditorSortingType sorting = tt->getEditorSorting();
			if (sorting >= ES_FIRST && sorting < ES_NUM_SORTING_TYPES)
				parent = findOrAdd(parent, EditorSortingNames[sorting]);
			else
				parent = findOrAdd(parent, "UNSORTED");

			QTreeWidgetItem *leaf = new QTreeWidgetItem(parent);
			leaf->setText(0, QString::fromUtf8(tt->getName().str()));
			leaf->setData(0, Qt::UserRole, index);
		} else {
			parent = findOrAdd(parent, "**TEST MODELS");
			QTreeWidgetItem *leaf = new QTreeWidgetItem(parent);
			leaf->setText(0, QString::fromUtf8(pMap->getName().str()));
			leaf->setData(0, Qt::UserRole, index);
		}
	}
	m_objectTree->sortItems(0, Qt::AscendingOrder);
}

/////////////////////////////////////////////////////////////////////////////
// selection handling

void ObjectOptions::onTreeSelectionChanged(void)
{
	if (m_updating) return;
	QList<QTreeWidgetItem*> sel = m_objectTree->selectedItems();
	if (sel.isEmpty()) return;
	QTreeWidgetItem *item = sel.first();
	int index = item->data(0, Qt::UserRole).toInt();
	if (index >= 0) {
		m_currentObjectIndex = index;
		strncpy(m_currentObjectName, item->text(0).toUtf8().constData(), NAME_MAX_LEN - 1);
		m_currentObjectName[NAME_MAX_LEN - 1] = 0;
	} else if (item->childCount() > 0) {
		strcpy(m_currentObjectName, "No Selection");
		m_currentObjectIndex = -1;
	}
	updateLabel();
}

void ObjectOptions::onOwningTeamChanged(int index)
{
	if (m_updating) return;
	m_curOwnerName.clear();
	if (index >= 0 && index < TheSidesList->getNumTeams()) {
		// Note: get the team name from the dict, not the (possibly renamed) popup.
		Dict *d = TheSidesList->getTeamInfo(index)->getDict();
		m_curOwnerName = d->getAsciiString(TheKey_teamName);
	}
}

QTreeWidgetItem *ObjectOptions::findLeaf(const char *label)
{
	// Find a leaf whose text matches the last path segment of label.
	const char *leaf = strrchr(label, '/');
	QString want = QString::fromUtf8(leaf ? leaf + 1 : label);
	QTreeWidgetItemIterator it(m_objectTree);
	while (*it) {
		if ((*it)->childCount() == 0 && (*it)->text(0) == want) {
			return *it;
		}
		++it;
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// data access

/*static*/ void ObjectOptions::update()
{
	if (m_staticThis)
		m_staticThis->updateLabel();
}

void ObjectOptions::updateLabel()
{
	if (m_objectName)
		m_objectName->setText(QString("Object Name: %1").arg(QString::fromUtf8(m_currentObjectName)));

	if (!m_owningTeam)
		return;

	bool wasUpdating = m_updating;
	m_updating = true;
	m_owningTeam->clear();

	AsciiString defTeamName;
	MapObject *pCur = getCurMapObject();

	// Refresh the 3d thumbnail for the current selection.
	if (m_preview)
		m_preview->setThingTemplate(pCur ? pCur->getThingTemplate() : NULL);

	if (pCur)
	{
		const ThingTemplate* tt = pCur->getThingTemplate();
		if (tt)
		{
			AsciiString defPlayerSide = tt->getDefaultOwningSide();
			Int i = findSideListEntryWithPlayerOfSide(defPlayerSide);
			if (i >= 0)
			{
				defTeamName.set("team");
				defTeamName.concat(TheSidesList->getSideInfo(i)->getDict()->getAsciiString(TheKey_playerName));
			}
		}
		else
		{
			defTeamName.set("team");	// neutral
		}
	}

	Int neutral = -1;
	Int sel = -1;
	for (int i = 0; i < TheSidesList->getNumTeams(); i++)
	{
		Dict *d = TheSidesList->getTeamInfo(i)->getDict();
		AsciiString name = d->getAsciiString(TheKey_teamName);

		if (name == defTeamName)
			sel = i;

		if (name == "team")
		{
			name = "(neutral)";
			neutral = i;
		}
		m_owningTeam->addItem(QString::fromUtf8(name.str()));
	}
	if (sel == -1)
		sel = neutral;
	if (sel == -1)
		m_curOwnerName.clear();
	else
		m_curOwnerName = TheSidesList->getTeamInfo(sel)->getDict()->getAsciiString(TheKey_teamName);
	m_owningTeam->setCurrentIndex(sel);

	m_updating = wasUpdating;
}

MapObject *ObjectOptions::getCurMapObject(void)
{
	if (m_staticThis && m_currentObjectIndex >= 0) {
		MapObject *pObj = m_staticThis->m_objectsList;
		int count = 0;
		while (pObj) {
			if (count == m_currentObjectIndex) {
				return pObj;
			}
			count++;
			pObj = pObj->getNext();
		}
	}
	return NULL;
}

AsciiString ObjectOptions::getCurGdfName(void)
{
	MapObject *pCur = getCurMapObject();
	if (pCur) {
		return pCur->getName();
	}
	return AsciiString::TheEmptyString;
}

MapObject *ObjectOptions::duplicateCurMapObjectForPlace(const Coord3D* loc, Real angle, Bool checkPlayers)
{
	MapObject *pCur = getCurMapObject();
	if (pCur)
	{
		Bool found = false;
		const ThingTemplate* tt = pCur->getThingTemplate();
		if (checkPlayers)
		{
			AsciiString defaultTeam("team");
			AsciiString objectTeamName = m_curOwnerName;
			if (objectTeamName != defaultTeam) {
				TeamsInfo *teamInfo = TheSidesList->findTeamInfo(objectTeamName);
				if (teamInfo) {
					AsciiString teamOwner = teamInfo->getDict()->getAsciiString(TheKey_teamOwner);
					SidesInfo* pSide = TheSidesList->findSideInfo(teamOwner);
					if (pSide) {
						found = true;
					}
				}
			}
			if (!found && tt)
			{
				AsciiString defPlayerSide = tt->getDefaultOwningSide();
				Int si = findSideListEntryWithPlayerOfSide(defPlayerSide);
				found = (si >= 0);
				if (!found) {
					// The map has no player for this object's side; offer to add one.
					AddPlayerDialog addPlyr(tt->getDefaultOwningSide());
					if (addPlyr.exec() == QDialog::Accepted) {
						for (int i = 0; i < TheSidesList->getNumSides(); i++) {
							AsciiString playerTmplName = TheSidesList->getSideInfo(i)->getDict()->getAsciiString(TheKey_playerFaction);
							if (playerTmplName == addPlyr.getAddedSide()) {
								m_curOwnerName.set("team");
								m_curOwnerName.concat(TheSidesList->getSideInfo(i)->getDict()->getAsciiString(TheKey_playerName));
								found = true;
								break;
							}
						}
					}
				}
			}
			else if (!found)
			{
				m_curOwnerName.set("team");	// neutral
				found = true;
			}
		} else {
			found = true;
		}
		if (found)
		{
			MapObject *pNew = newInstance(MapObject)(*loc, pCur->getName(), angle,
															 pCur->getFlags(), pCur->getProperties(),
															 pCur->getThingTemplate());
			pNew->getProperties()->setAsciiString(TheKey_originalOwner, m_curOwnerName);
			pNew->setColor(pCur->getColor());
			return pNew;
		}
	}
	return NULL;
}

Real ObjectOptions::getCurObjectHeight(void)
{
	if (m_staticThis && m_staticThis->m_heightSpin) {
		Real height = m_staticThis->m_heightSpin->value();
		if (height > 0) {
			height *= MAP_HEIGHT_SCALE;
		}
		return height;
	}
	return MAGIC_GROUND_Z;
}

MapObject *ObjectOptions::getObjectNamed(AsciiString name)
{
	if (m_staticThis) {
		MapObject *pObj = m_staticThis->m_objectsList;
		while (pObj) {
			if (name == pObj->getName()) {
				return pObj;
			}
			const char *curName = pObj->getName().str();
			const char *leaf = curName;
			while (*curName) {
				if (*curName == '/') {
					leaf = curName + 1;
				}
				curName++;
			}
			if (0 == strcmp(leaf, name.str())) {
				return pObj;
			}
			pObj = pObj->getNext();
		}
	}
	return NULL;
}

Int ObjectOptions::getObjectNamedIndex(const AsciiString& name)
{
	if (m_staticThis) {
		MapObject *pObj = m_staticThis->m_objectsList;
		int count = 0;
		while (pObj) {
			if (name == pObj->getName()) {
				return count;
			}
			const char *curName = pObj->getName().str();
			const char *leaf = curName;
			while (*curName) {
				if (*curName == '/') {
					leaf = curName + 1;
				}
				curName++;
			}
			if (0 == strcmp(leaf, name.str())) {
				return count;
			}
			count++;
			pObj = pObj->getNext();
		}
	}
	return 0;
}

void ObjectOptions::selectObject(const MapObject* pObj)
{
	if (!m_staticThis || !pObj)
		return;

	QTreeWidgetItem *item = m_staticThis->findLeaf(pObj->getName().str());
	if (item == NULL)
		return;

	m_staticThis->m_objectTree->setCurrentItem(item);
	int index = item->data(0, Qt::UserRole).toInt();
	if (index >= 0) {
		m_staticThis->m_currentObjectIndex = index;
		strncpy(m_staticThis->m_currentObjectName, item->text(0).toUtf8().constData(), NAME_MAX_LEN - 1);
		m_staticThis->m_currentObjectName[NAME_MAX_LEN - 1] = 0;
	}
}
