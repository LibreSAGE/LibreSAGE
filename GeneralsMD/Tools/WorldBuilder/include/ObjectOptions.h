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

// ObjectOptions.h : the object-placement palette (Qt6 port)
//

#pragma once

#include <QWidget>

#include "Lib/BaseType.h"
#include "Common/AsciiString.h"

class QComboBox;
class QLabel;
class QSpinBox;
class QTreeWidget;
class QTreeWidgetItem;
class MapObject;
class ObjectPreview;

/////////////////////////////////////////////////////////////////////////////
/// ObjectOptions panel - pick an object template, owning team and height, and
/// place it in the world with the object tool.

class ObjectOptions : public QWidget
{
	Q_OBJECT

public:
	enum { NAME_MAX_LEN = 64 };

	ObjectOptions(QWidget *parent = NULL);
	~ObjectOptions() override;

	// Accessed by ObjectTool / the document to drive placement.
	static const char *getCurObjectName(void) {return m_currentObjectName;}
	static MapObject *duplicateCurMapObjectForPlace(const Coord3D* loc, Real angle, Bool checkPlayers = true);
	static MapObject *getObjectNamed(AsciiString name);
	static Int getObjectNamedIndex(const AsciiString& name);
	static void selectObject(const MapObject* pObj);
	static Real getCurObjectHeight(void);
	static void update();
	static AsciiString getCurGdfName(void);

private slots:
	void onTreeSelectionChanged();
	void onOwningTeamChanged(int index);

private:
	void buildObjectList();
	void buildTree();
	QTreeWidgetItem *findOrAdd(QTreeWidgetItem *parent, const char *label);
	QTreeWidgetItem *findLeaf(const char *label);
	void updateLabel();
	static MapObject *getCurMapObject(void);

	QLabel			*m_objectName;
	QTreeWidget		*m_objectTree;
	QSpinBox		*m_heightSpin;
	QComboBox		*m_owningTeam;
	ObjectPreview	*m_preview;

	MapObject		*m_objectsList;		///< linked list of one template each.

	static ObjectOptions	*m_staticThis;
	static Bool				m_updating;
	static Int				m_currentObjectIndex;
	static char				m_currentObjectName[NAME_MAX_LEN];
	static AsciiString		m_curOwnerName;
};
