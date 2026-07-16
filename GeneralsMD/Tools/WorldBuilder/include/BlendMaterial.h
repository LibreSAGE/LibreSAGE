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

// BlendMaterial.h : the auto-blend-edge texture chooser panel (Qt6 port)
//
// Lists only the "blend edge" texture classes (plus a special "Alpha Blend"
// entry for automatic selection), used by AutoEdgeOutTool.
// @todo port the TerrainSwatches texture preview bitmaps.

#pragma once

#include <QWidget>

#include "Lib/BaseType.h"

class QTreeWidget;
class QTreeWidgetItem;

/////////////////////////////////////////////////////////////////////////////
// BlendMaterial panel

class BlendMaterial : public QWidget
{
	Q_OBJECT

public:
	BlendMaterial(QWidget *parent = NULL);
	~BlendMaterial() override;

	static Int getBlendTexClass(void) {return m_currentBlendTexture;}
	static void setBlendTexClass(Int texClass);
	static void updateTextures(void);

protected:
	QTreeWidgetItem *findOrAdd(QTreeWidgetItem *parent, const char *pLabel);
	void addTerrain(const char *pPath, Int terrainNdx);
	void selectTexClassInTree(Int texClass);

protected:
	static BlendMaterial	*m_staticThis;
	Bool					m_updating;
	static Int				m_currentBlendTexture;

	QTreeWidget				*m_terrainTree;
};
