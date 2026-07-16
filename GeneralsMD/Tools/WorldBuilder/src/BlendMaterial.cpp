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

// BlendMaterial.cpp : the auto-blend-edge texture chooser panel (Qt6 port of
// the IDD_BLEND_MATERIAL dialog).  Only "blend edge" texture classes are
// listed, plus a synthetic "Alpha Blend" entry (index -1) meaning "let
// autoBlendOut() pick automatically" - the tool's default.

#include <QTreeWidget>
#include <QVBoxLayout>

#include "BlendMaterial.h"

#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"

#include "Common/TerrainTypes.h"

BlendMaterial *BlendMaterial::m_staticThis = NULL;
Int BlendMaterial::m_currentBlendTexture = -1;

static const Int ALPHA_BLEND_INDEX = -1;

//-------------------------------------------------------------------------------------------------
BlendMaterial::BlendMaterial(QWidget *parent) :
	QWidget(parent),
	m_updating(false)
{
	QVBoxLayout *layout = new QVBoxLayout(this);

	m_terrainTree = new QTreeWidget(this);
	m_terrainTree->setHeaderHidden(true);
	layout->addWidget(m_terrainTree, 1);

	connect(m_terrainTree, &QTreeWidget::currentItemChanged, this,
			[this](QTreeWidgetItem *item, QTreeWidgetItem *) {
		if (m_updating || item == NULL) return;
		QVariant data = item->data(0, Qt::UserRole);
		if (data.isNull()) return;
		m_currentBlendTexture = data.toInt();
	});

	m_staticThis = this;
	updateTextures();
}

BlendMaterial::~BlendMaterial()
{
	if (m_staticThis == this) {
		m_staticThis = NULL;
	}
}

//-------------------------------------------------------------------------------------------------
QTreeWidgetItem *BlendMaterial::findOrAdd(QTreeWidgetItem *parent, const char *pLabel)
{
	QString label = QString::fromUtf8(pLabel);
	if (parent) {
		for (int i = 0; i < parent->childCount(); ++i) {
			if (parent->child(i)->text(0) == label) {
				return parent->child(i);
			}
		}
		QTreeWidgetItem *item = new QTreeWidgetItem(parent, QStringList(label));
		item->setData(0, Qt::UserRole, QVariant());
		return item;
	}
	for (int i = 0; i < m_terrainTree->topLevelItemCount(); ++i) {
		if (m_terrainTree->topLevelItem(i)->text(0) == label) {
			return m_terrainTree->topLevelItem(i);
		}
	}
	QTreeWidgetItem *item = new QTreeWidgetItem(m_terrainTree, QStringList(label));
	item->setData(0, Qt::UserRole, QVariant());
	return item;
}

//-------------------------------------------------------------------------------------------------
/** Add a blend-edge texture class to the tree (or the synthetic "Alpha
Blend" entry when terrainNdx == -1); legacy eval blend-edge textures land in
a "**EVAL**" leaf, same as the original. */
void BlendMaterial::addTerrain(const char *pPath, Int terrainNdx)
{
	AsciiString className;
	if (terrainNdx >= 0) {
		className = WorldHeightMapEdit::getTexClassName( terrainNdx );
	}
	TerrainType *terrain = TheTerrainTypes->findTerrain( className );
	QTreeWidgetItem *parent = NULL;
	QString label;
	Bool doAdd = false;

	if( terrain )
	{
		if (!terrain->isBlendEdge()) {
			return;	 // Only do blend edges to the blend material list.
		}
		label = QString::fromUtf8(terrain->getName().str());
		doAdd = true;
	} else if (terrainNdx == ALPHA_BLEND_INDEX) {
		label = QString::fromUtf8(pPath);
		doAdd = true;
	} else if (WorldHeightMapEdit::getTexClassIsBlendEdge(terrainNdx)) {
		parent = findOrAdd( NULL, "**EVAL**" );
		label = QString::fromUtf8(pPath);
		doAdd = true;
	}

	if (doAdd) {
		QTreeWidgetItem *item = parent ? new QTreeWidgetItem(parent, QStringList(label))
									   : new QTreeWidgetItem(m_terrainTree, QStringList(label));
		item->setData(0, Qt::UserRole, terrainNdx);
	}
}

//-------------------------------------------------------------------------------------------------
void BlendMaterial::selectTexClassInTree(Int texClass)
{
	for (QTreeWidgetItemIterator it(m_terrainTree); *it; ++it) {
		QVariant data = (*it)->data(0, Qt::UserRole);
		if (!data.isNull() && data.toInt() == texClass) {
			m_terrainTree->setCurrentItem(*it);
			m_terrainTree->scrollToItem(*it);
			return;
		}
	}
}

//-------------------------------------------------------------------------------------------------
void BlendMaterial::updateTextures(void)
{
	if (m_staticThis == NULL) {
		return;
	}
	m_staticThis->m_updating = true;
	m_staticThis->m_terrainTree->clear();
	m_staticThis->addTerrain("Alpha Blend", ALPHA_BLEND_INDEX);
	for (Int i = WorldHeightMapEdit::getNumTexClasses() - 1; i >= 0; i--) {
		AsciiString uiName = WorldHeightMapEdit::getTexClassUiName(i);
		m_staticThis->addTerrain(uiName.str(), i);
	}
	m_staticThis->m_updating = false;
	m_currentBlendTexture = ALPHA_BLEND_INDEX;
	m_staticThis->selectTexClassInTree(m_currentBlendTexture);
}

//-------------------------------------------------------------------------------------------------
void BlendMaterial::setBlendTexClass(Int texClass)
{
	m_currentBlendTexture = texClass;
	if (m_staticThis) {
		m_staticThis->selectTexClassInTree(texClass);
	}
}
