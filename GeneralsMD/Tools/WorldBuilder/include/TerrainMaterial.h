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

// TerrainMaterial.h : the terrain texture chooser panel (Qt6 port)
//
// @todo port the TerrainSwatches texture preview bitmaps.

#pragma once

#include <QWidget>

#include "Lib/BaseType.h"

class WorldHeightMapEdit;
class QTreeWidget;
class QTreeWidgetItem;
class QLabel;
class QSpinBox;
class QCheckBox;
class QRadioButton;

/////////////////////////////////////////////////////////////////////////////
// TerrainMaterial panel

class TerrainMaterial : public QWidget
{
	Q_OBJECT

public:
	enum {MIN_TILE_SIZE=2, MAX_TILE_SIZE = 100};

	TerrainMaterial(QWidget *parent = NULL);
	~TerrainMaterial() override;

	static Int getFgTexClass(void) {return m_currentFgTexture;}
	static Int getBgTexClass(void) {return m_currentBgTexture;}

	static void setFgTexClass(Int texClass);
	static void setBgTexClass(Int texClass);
	static void updateTextures(WorldHeightMapEdit *pMap);
	static void updateTextureSelection(void);
	static void setToolOptions(Bool singleCell);
	static void setWidth(Int width);

	static Bool isPaintingPathingInfo(void) {return m_paintingPathingInfo;}
	static Bool isPaintingPassable(void) {return m_paintingPassable;}

protected:
	QTreeWidgetItem *findOrAdd(QTreeWidgetItem *parent, const char *pLabel);
	void addTerrain(const char *pPath, Int terrainNdx);
	void updateLabel(void);
	void selectTexClassInTree(Int texClass);

protected:
	static TerrainMaterial	*m_staticThis;
	Bool										m_updating;
	static Int							m_currentFgTexture;
	static Int							m_currentBgTexture;

	static Bool m_paintingPathingInfo;	 // If true, we are painting passable/impassable.  If false, normal texture painting.
	static Bool m_paintingPassable;

	QTreeWidget			*m_terrainTree;
	QLabel				*m_fgLabel;
	QLabel				*m_bgLabel;
	QSpinBox			*m_widthSpin;
	QCheckBox			*m_passabilityCheck;
	QRadioButton		*m_passableRadio;
	QRadioButton		*m_impassableRadio;
};
