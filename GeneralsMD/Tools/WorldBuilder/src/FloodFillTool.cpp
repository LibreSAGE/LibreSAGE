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

// FloodFillTool.cpp
// Texture flood fill tool for worldbuilder.
// Author: John Ahlquist, April 2001

#include <QGuiApplication>
#include <QPixmap>

#include "FloodFillTool.h"
#include "CUndoable.h"
#include "DrawObject.h"
#include "MainFrm.h"
#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"
#include "TerrainMaterial.h"
#include "wbview.h"

Bool FloodFillTool::m_adjustCliffTextures = false;

FloodFillTool::FloodFillTool(void) :
	Tool(ID_FLOOD_FILL_TOOL, ":/cursors/IDC_FLOOD_FILL.cur")
{
	QPixmap cliffPix(":/cursors/IDC_CLIFF.cur");
	if (!cliffPix.isNull()) m_cliffCursor = QCursor(cliffPix);
}

FloodFillTool::~FloodFillTool(void)
{
}


void FloodFillTool::activate()
{
	if (CMainFrame::GetMainFrame())
		CMainFrame::GetMainFrame()->showOptionsDialog(ID_TILE_TOOL);
	TerrainMaterial::setToolOptions(true);
	DrawObject::setDoBrushFeedback(false);
	m_adjustCliffTextures = false;
}

/** The cursor reflects cliff-adjust mode. */
QCursor FloodFillTool::getCursor(void)
{
	if (m_adjustCliffTextures) {
		return m_cliffCursor;
	}
	return Tool::getCursor();
}


/** Creates a copy of the height map, flood fills it at pt with m_textureClassToDraw which
has been set by the calling routine.  Then builds
the command, and passes it to the doc. */
void FloodFillTool::mouseUp(TTrackingMode m, QPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	Coord3D cpt;
	pView->viewToDocCoords(viewPt, &cpt);

	QPoint ndx;
	if (!pDoc->getCellIndexFromCoord(cpt, &ndx)) {
		return;
	}

	if (m == TRACK_L)
		m_textureClassToDraw = TerrainMaterial::getFgTexClass();
	else
		m_textureClassToDraw = TerrainMaterial::getBgTexClass();

	WorldHeightMapEdit *htMapEditCopy = pDoc->GetHeightMap()->duplicate();
	Bool didIt = false;
	Bool shiftKey = (QGuiApplication::queryKeyboardModifiers() & Qt::ShiftModifier) != 0;
	if (m_adjustCliffTextures) {
		didIt = htMapEditCopy->doCliffAdjustment(ndx.x(), ndx.y());
	} else {
		didIt = htMapEditCopy->floodFill(ndx.x(), ndx.y(), m_textureClassToDraw, shiftKey);
	}
	if (didIt) {
		htMapEditCopy->optimizeTiles(); // force to optimize tileset
		IRegion2D partialRange = {0,0,0,0};
		pDoc->updateHeightMap(htMapEditCopy, false, partialRange);
		WBDocUndoable *pUndo = new WBDocUndoable(pDoc, htMapEditCopy);
		pDoc->AddAndDoUndoable(pUndo);
		REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	}
	REF_PTR_RELEASE(htMapEditCopy);
}
