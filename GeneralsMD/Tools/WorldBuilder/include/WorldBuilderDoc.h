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

// WorldBuilderDoc.h : interface of the CWorldBuilderDoc class (Qt6 port)
//

#pragma once

#include <QObject>
#include <QPoint>

#include "Lib/BaseType.h"
#include "Common/AsciiString.h"
#include "Tool.h"

class MapObject;
class WorldHeightMapEdit;
class WorldHeightMap;
class Undoable;
class WbView3d;
class DataChunkInput;
struct DataChunkInfo;

#define MAX_UNDOS 15

#define MIN_CELL_SIZE 1
#define MAX_CELL_SIZE 64

class CWorldBuilderDoc : public QObject
{
	Q_OBJECT

	enum {MAX_WAYPOINTS=16000}; ///@todo - make it dynamic.  jba.

public:
	CWorldBuilderDoc(QObject *parent = NULL);
	~CWorldBuilderDoc() override;

protected:
	WorldHeightMapEdit	*m_heightMap;
	Undoable						*m_undoList;  ///< Head of undo/redo list.
	int									m_maxUndos;
	int									m_curRedo;		///< 0 means no redos available.
	Bool								m_linkCenters;				///< Flag whether the centers of the 2d and 3d views track together.
	Bool								m_needAutosave;			///< True if changes have been made since last autosave.
	Bool								m_modified;
	Int									m_curWaypointID;

	AsciiString					m_filePath;						///< Full path of the current map file (empty for new maps).

	WbView3d						*m_3dView;						///< The (single) 3d view on this document.

protected:
	std::vector<ICoord2D> m_boundaries;

protected:	// waypoint stuff.
	struct {
		Int waypoint1;
		Int waypoint2;
		Bool processedFlag;
	} m_waypointLinks[MAX_WAYPOINTS];
	Int									m_numWaypointLinks;

public:
	static Bool ParseWaypointDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData);
	Bool ParseWaypointData(DataChunkInput &file, DataChunkInfo *info, void *userData);

	Int getNumWaypointLinks(void) {return m_numWaypointLinks;};
	void getWaypointLink(Int ndx, Int *waypoint1, Int *waypointID2);

	// Boundary stuff
	Int getNumBoundaries(void) const {return (Int)m_boundaries.size();}
	void getBoundary(Int ndx, ICoord2D* border) const {*border = m_boundaries[ndx];}
	void addBoundary(ICoord2D* boundaryToAdd) {m_boundaries.push_back(*boundaryToAdd);}

// Attributes
public:
	WorldHeightMapEdit *GetHeightMap() {return m_heightMap;}
	void SetHeightMap(WorldHeightMapEdit *pMap, Bool doUpdate);

	// The 3d view registers itself here (MainFrm creates it).
	void attach3DView(WbView3d *pView) {m_3dView = pView;}
	WbView3d *Get3DView(void) {return m_3dView;}

	static CWorldBuilderDoc *GetActiveDoc(void);
	static WbView3d *GetActive3DView(void);

	AsciiString getFilePath(void) const {return m_filePath;}
	Bool isModified(void) const {return m_modified;}
	void SetModifiedFlag(Bool modified = true) {m_modified = modified;}

// Operations
public:
	/// Create a fresh flat map (port of OnNewDocument, dialog-less).
	Bool newDocument(Int xExtent, Int yExtent, Int initialHeight, Int borderWidth);
	/// Load a .map file (port of OnOpenDocument + Serialize load branch).
	Bool openDocument(const char *path);
	/// Save to a .map file (port of the Serialize store branch).
	Bool saveDocument(const char *path);

	Bool needAutoSave(void) {return m_needAutosave;}
	void autoSave(void);

	void AddAndDoUndoable(Undoable *pUndo);
	void OnEditUndo();
	void OnEditRedo();
	Bool canUndo(void);
	Bool canRedo(void) {return m_undoList!=NULL && m_curRedo>0;}

	// View update forwarding.
	void updateHeightMap(WorldHeightMap *htMap, Bool partial, const IRegion2D &partialRange);
	void invalObject(MapObject *pMapObj);
	void invalCell(int xIndex, int yIndex);
	void syncViewCenters(Real x, Real y);
	void updateAllViews();

	// Cell/coordinate helpers.
	Bool getCellIndexFromCoord(Coord3D cpt, QPoint *ndxP);
	void getCoordFromCellIndex(QPoint ndx, Coord3D* pt);
	Bool getCellPositionFromCoord(Coord3D cpt, Coord3D *locP);
	Bool getAllIndexesInRect(const Coord3D* bl, const Coord3D* br,
													 const Coord3D* tl, const Coord3D* tr,
													 Int widthOutside, VecHeightMapIndexes* allIndices);

	void validate(void);
};

inline CWorldBuilderDoc *WbDoc() { return CWorldBuilderDoc::GetActiveDoc(); }
