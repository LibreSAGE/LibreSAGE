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

#include <QPixmap>

// Tool.cpp
// Texture tiling tool for worldbuilder.
// Author: John Ahlquist, April 2001

#include "MainFrm.h"
#include "WorldBuilderDoc.h"
#include "Common/MapObject.h"

#include "Tool.h"


//
/// Tool class.
//
/// Constructor
Tool::Tool(Int toolID, const char *cursorResource)
{
	m_toolID = toolID;
	QPixmap pixmap(QString::fromUtf8(cursorResource));
	if (!pixmap.isNull()) {
		m_cursor = QCursor(pixmap);
	}
}


/// Destructor
Tool::~Tool(void)
{
}


/// Shows the "no options"  options panel.
void Tool::activate()
{
	if (CMainFrame::GetMainFrame())
		CMainFrame::GetMainFrame()->showOptionsDialog(ID_NO_TOOL);
	/// @todo DrawObject::setDoBrushFeedback(false); once DrawObject is ported.
}

/// Calculate the round blend factor.
/** Calculates the blend amount of the brush.  1.0 means the brush sets the
height, 0.0 means no change, and between blends proportionally. */
Real Tool::calcRoundBlendFactor(QPoint center, Int x, Int y, Int brushWidth, Int featherWidth)
{
	Real offset = 0;
	if (brushWidth&1) offset = 0.5;
	const Real CLOSE_ENOUGH = 0.05f;	 // We are working on an integer grid.

	Real dx = abs(center.x()+offset-x);
	Real dy = abs(center.y()+offset-y);

	Real dist = (Real)sqrt(dx*dx+dy*dy);

	if (dist <= (brushWidth/2.0f)+CLOSE_ENOUGH) return (1.0f);

	dist -= (brushWidth/2.0f);

	if (featherWidth < 1) {
		return(0);
	}

	if (dist <= featherWidth) {
		return (featherWidth-dist)/featherWidth;
	}

	return(0);
}

/// Calculate the square blend factor.
/** Calculates the blend amount of the brush.  1.0 means the brush sets the
height, 0.0 means no change, and between blends proportionally. */
Real Tool::calcSquareBlendFactor(QPoint center, Int x, Int y, Int brushWidth, Int featherWidth)
{
	Real offset = 0;
	if (brushWidth&1) offset = 0.5;

	Real dx = abs(center.x()+offset-x);
	Real dy = abs(center.y()+offset-y);

	Real dist = dx;
	if (dy>dist) dist = dy;

	if (dist <= (brushWidth/2.0f)) return (1.0f);

	dist -= (brushWidth/2.0f);

	if (featherWidth < 1) {
		return(0);
	}

	if (dist <= featherWidth) {
		return (featherWidth-dist)/featherWidth;
	}

	return(0);
}

/// Gets the cell index for the center of the brush.
/** Converts from document coordinates to cell index coordinates. */
void Tool::getCenterIndex(Coord3D *docLocP, Int brushWidth, QPoint *center, CWorldBuilderDoc *pDoc)
{
	Coord3D cpt = *docLocP;
	// center on half pixel for even widths.
	if (!(brushWidth&1)) {
		cpt.x += MAP_XY_FACTOR/2;
		cpt.y += MAP_XY_FACTOR/2;
	}
	if (!pDoc->getCellIndexFromCoord(cpt, center)) {
		return;
	}
}

void Tool::getAllIndexesIn(const Coord3D *bl, const Coord3D *br,
													 const Coord3D *tl, const Coord3D *tr,
													 Int widthOutside, CWorldBuilderDoc *pDoc,
													 VecHeightMapIndexes* allIndices)
{
	if (!(bl && br && tl && tr && pDoc && allIndices)) {
		return;
	}

	pDoc->getAllIndexesInRect(bl, br, tl, tr, widthOutside, allIndices);
}
