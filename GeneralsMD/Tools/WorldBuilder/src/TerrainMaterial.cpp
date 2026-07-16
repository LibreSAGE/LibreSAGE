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

// TerrainMaterial.cpp : the terrain texture chooser panel (Qt6 port of the
// IDD_TERRAIN_MATERIAL dialog).  The texture tree is grouped by the terrain
// class from INI (like the MFC tree control); swatch previews are @todo.

#include <QCheckBox>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QFormLayout>

#include "TerrainMaterial.h"

#include "TileTool.h"
#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"

#define DEFINE_TERRAIN_TYPE_NAMES
#include "Common/TerrainTypes.h"

TerrainMaterial *TerrainMaterial::m_staticThis = NULL;
Int TerrainMaterial::m_currentFgTexture = 0;
Int TerrainMaterial::m_currentBgTexture = 0;
Bool TerrainMaterial::m_paintingPathingInfo = false;
Bool TerrainMaterial::m_paintingPassable = false;

//-------------------------------------------------------------------------------------------------
TerrainMaterial::TerrainMaterial(QWidget *parent) :
	QWidget(parent),
	m_updating(false)
{
	QVBoxLayout *layout = new QVBoxLayout(this);

	m_terrainTree = new QTreeWidget(this);
	m_terrainTree->setHeaderHidden(true);
	layout->addWidget(m_terrainTree, 1);

	QFormLayout *form = new QFormLayout();
	m_fgLabel = new QLabel(this);
	form->addRow("Left click:", m_fgLabel);
	m_bgLabel = new QLabel(this);
	form->addRow("Right click:", m_bgLabel);

	m_widthSpin = new QSpinBox(this);
	m_widthSpin->setRange(1, MAX_TILE_SIZE);
	form->addRow("Brush width:", m_widthSpin);
	layout->addLayout(form);

	QPushButton *swapButton = new QPushButton("Swap left/right textures", this);
	layout->addWidget(swapButton);

	m_passabilityCheck = new QCheckBox("Paint passability instead", this);
	layout->addWidget(m_passabilityCheck);
	m_passableRadio = new QRadioButton("Passable", this);
	m_impassableRadio = new QRadioButton("Impassable", this);
	m_impassableRadio->setChecked(true);
	m_passableRadio->setEnabled(false);
	m_impassableRadio->setEnabled(false);
	layout->addWidget(m_passableRadio);
	layout->addWidget(m_impassableRadio);

	connect(m_terrainTree, &QTreeWidget::currentItemChanged, this,
			[this](QTreeWidgetItem *item, QTreeWidgetItem *) {
		if (m_updating || item == NULL) return;
		Int texClass = item->data(0, Qt::UserRole).toInt();
		if (item->data(0, Qt::UserRole).isNull() || texClass < 0) return;
		CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
		if (!pDoc) return;
		WorldHeightMapEdit *pMap = pDoc->GetHeightMap();
		if (!pMap) return;
		if (!pMap->canFitTexture(texClass) && m_currentFgTexture != texClass) {
			QMessageBox::warning(NULL, "WorldBuilder",
								 "There is not enough room in the texture set for this texture.");
		}
		m_currentFgTexture = texClass;
		updateLabel();
	});
	connect(swapButton, &QPushButton::clicked, this, [this]() {
		Int tmp = m_currentFgTexture;
		m_currentFgTexture = m_currentBgTexture;
		m_currentBgTexture = tmp;
		updateTextureSelection();
	});
	connect(m_widthSpin, &QSpinBox::valueChanged, this, [this](int value) {
		if (!m_updating) BigTileTool::setWidth(value);
	});
	connect(m_passabilityCheck, &QCheckBox::toggled, this, [this](bool checked) {
		m_paintingPathingInfo = checked;
		m_passableRadio->setEnabled(checked);
		m_impassableRadio->setEnabled(checked);
	});
	connect(m_passableRadio, &QRadioButton::toggled, this, [this](bool checked) {
		m_paintingPassable = checked;
	});

	m_staticThis = this;
}

TerrainMaterial::~TerrainMaterial()
{
	if (m_staticThis == this) {
		m_staticThis = NULL;
	}
}

//-------------------------------------------------------------------------------------------------
QTreeWidgetItem *TerrainMaterial::findOrAdd(QTreeWidgetItem *parent, const char *pLabel)
{
	QString label = QString::fromUtf8(pLabel);
	if (parent) {
		for (int i = 0; i < parent->childCount(); ++i) {
			if (parent->child(i)->text(0) == label) {
				return parent->child(i);
			}
		}
		QTreeWidgetItem *item = new QTreeWidgetItem(parent, QStringList(label));
		item->setData(0, Qt::UserRole, -1);
		return item;
	}
	for (int i = 0; i < m_terrainTree->topLevelItemCount(); ++i) {
		if (m_terrainTree->topLevelItem(i)->text(0) == label) {
			return m_terrainTree->topLevelItem(i);
		}
	}
	QTreeWidgetItem *item = new QTreeWidgetItem(m_terrainTree, QStringList(label));
	item->setData(0, Qt::UserRole, -1);
	return item;
}

//-------------------------------------------------------------------------------------------------
/** Sort a texture class into the tree, grouped by its INI terrain class
(legacy eval textures land in an **Eval leaf like the original). */
void TerrainMaterial::addTerrain(const char *pPath, Int terrainNdx)
{
	TerrainType *terrain = TheTerrainTypes->findTerrain( WorldHeightMapEdit::getTexClassName( terrainNdx ) );
	QTreeWidgetItem *parent = NULL;
	QString leafName;

	if( terrain )
	{
		if (terrain->isBlendEdge()) {
			return;	 // Don't add blend edges to the materials list.
		}
		for( TerrainClass i = TERRAIN_NONE; i < TERRAIN_NUM_CLASSES; i = (TerrainClass)(i + 1) )
		{
			if( terrain->getClass() == i )
			{
				parent = findOrAdd( NULL, terrainTypeNames[ i ] );
				break;
			}
		}
		leafName = QString::fromUtf8(terrain->getName().str());
	}
	else if (!WorldHeightMapEdit::getTexClassIsBlendEdge(terrainNdx))
	{
		// all these old entries we will put in a tree for eval textures
		parent = findOrAdd( NULL, "**Eval" );
		QString path = QString::fromUtf8(pPath).replace('\\', '/');
		leafName = path.section('/', -1);
	}
	else
	{
		return;
	}

	QTreeWidgetItem *item = parent ? new QTreeWidgetItem(parent, QStringList(leafName))
								   : new QTreeWidgetItem(m_terrainTree, QStringList(leafName));
	item->setData(0, Qt::UserRole, terrainNdx);
}

//-------------------------------------------------------------------------------------------------
void TerrainMaterial::updateLabel(void)
{
	CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
	if (!pDoc || !pDoc->GetHeightMap()) return;

	AsciiString fg = pDoc->GetHeightMap()->getTexClassUiName(m_currentFgTexture);
	AsciiString bg = pDoc->GetHeightMap()->getTexClassUiName(m_currentBgTexture);
	m_fgLabel->setText(QString::fromUtf8(fg.str()).replace('\\','/').section('/', -1));
	m_bgLabel->setText(QString::fromUtf8(bg.str()).replace('\\','/').section('/', -1));
}

//-------------------------------------------------------------------------------------------------
void TerrainMaterial::selectTexClassInTree(Int texClass)
{
	for (QTreeWidgetItemIterator it(m_terrainTree); *it; ++it) {
		if ((*it)->data(0, Qt::UserRole).toInt() == texClass) {
			m_terrainTree->setCurrentItem(*it);
			m_terrainTree->scrollToItem(*it);
			return;
		}
	}
}

//-------------------------------------------------------------------------------------------------
void TerrainMaterial::updateTextures(WorldHeightMapEdit *pMap)
{
	if (m_staticThis) {
		m_staticThis->m_updating = true;
		m_staticThis->m_terrainTree->clear();
		Int i;
		for (i=0; i<pMap->getNumTexClasses(); i++) {
			AsciiString uiName = pMap->getTexClassUiName(i);
			m_staticThis->addTerrain(uiName.str(), i);
		}
		m_staticThis->m_terrainTree->sortItems(0, Qt::AscendingOrder);
		m_staticThis->m_updating = false;
		updateTextureSelection();
	}
}

//-------------------------------------------------------------------------------------------------
void TerrainMaterial::updateTextureSelection(void)
{
	if (m_staticThis) {
		m_staticThis->m_updating = true;
		m_staticThis->selectTexClassInTree(m_currentFgTexture);
		m_staticThis->m_updating = false;
		m_staticThis->updateLabel();
	}
}

//-------------------------------------------------------------------------------------------------
void TerrainMaterial::setFgTexClass(Int texClass)
{
	m_currentFgTexture = texClass;
	updateTextureSelection();
}

void TerrainMaterial::setBgTexClass(Int texClass)
{
	m_currentBgTexture = texClass;
	updateTextureSelection();
}

//-------------------------------------------------------------------------------------------------
void TerrainMaterial::setToolOptions(Bool singleCell)
{
	if (m_staticThis) {
		m_staticThis->m_updating = true;
		m_staticThis->m_widthSpin->setEnabled(!singleCell);
		if (singleCell) {
			m_staticThis->m_widthSpin->setValue(1);
		}
		m_staticThis->m_updating = false;
	}
}

void TerrainMaterial::setWidth(Int width)
{
	if (m_staticThis) {
		m_staticThis->m_updating = true;
		m_staticThis->m_widthSpin->setValue(width);
		m_staticThis->m_updating = false;
	}
}
