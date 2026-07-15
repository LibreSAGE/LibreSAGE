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

#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

// WorldBuilderDoc.cpp : implementation of the CWorldBuilderDoc class (Qt6
// port).  The load/save logic is the port of the original MFC Serialize();
// printing, MRU and the 2d view were dropped.
//

#include "WorldBuilderDoc.h"

#include "CUndoable.h"
#include "MainFrm.h"
#include "WHeightMapEdit.h"
#include "WorldBuilder.h"
#include "wbview3d.h"

#include "Common/Debug.h"
#include "Common/DataChunk.h"
#include "Common/GlobalData.h"
#include "Common/MapObject.h"
#include "Common/MapReaderWriterInfo.h"
#include "Common/WellKnownKeys.h"
#include "Common/NameKeyGenerator.h"
#include "GameClient/GameText.h"
#include "GameLogic/PolygonTrigger.h"
#include "GameLogic/SidesList.h"


#include <stdio.h>

//-------------------------------------------------------------------------------------------------
// Plain stdio replacement for the original MFCFileOutputStream.
//-------------------------------------------------------------------------------------------------
class StdioFileOutputStream : public OutputStream
{
public:
	StdioFileOutputStream(FILE *pFile):m_file(pFile) {};
	virtual Int write(const void *pData, Int numBytes)
	{
		if (m_file == NULL) return 0;
		return (Int)fwrite(pData, 1, numBytes, m_file);
	};
protected:
	FILE *m_file;
};

/////////////////////////////////////////////////////////////////////////////
// CWorldBuilderDoc construction/destruction

CWorldBuilderDoc::CWorldBuilderDoc(QObject *parent) :
	QObject(parent),
	m_heightMap(NULL),
	m_undoList(NULL),
	m_maxUndos(MAX_UNDOS),
	m_curRedo(0),
	m_linkCenters(false),
	m_needAutosave(false),
	m_modified(false),
	m_curWaypointID(0),
	m_3dView(NULL),
	m_numWaypointLinks(0)
{
}

CWorldBuilderDoc::~CWorldBuilderDoc()
{
	REF_PTR_RELEASE(m_undoList);
	REF_PTR_RELEASE(m_heightMap);
}

CWorldBuilderDoc *CWorldBuilderDoc::GetActiveDoc(void)
{
	return WbApp() ? WbApp()->getDocument() : NULL;
}

WbView3d *CWorldBuilderDoc::GetActive3DView(void)
{
	CWorldBuilderDoc *pDoc = GetActiveDoc();
	return pDoc ? pDoc->Get3DView() : NULL;
}

/////////////////////////////////////////////////////////////////////////////
// New / open / save (port of OnNewDocument & Serialize)

Bool CWorldBuilderDoc::newDocument(Int xExtent, Int yExtent, Int initialHeight, Int borderWidth)
{
	// clear out map-specific text
	TheGameText->reset();

	REF_PTR_RELEASE(m_heightMap);
	REF_PTR_RELEASE(m_undoList);
	m_curRedo = 0;
	m_numWaypointLinks = 0;
	m_curWaypointID = 0;
	m_filePath.clear();
	WbApp()->selectPointerTool();
	PolygonTrigger::deleteTriggers();

	/// @todo TheLayersList->resetLayers() once the layers panel is ported.

	TheSidesList->clear();
	TheSidesList->validateSides();

	WbView3d *p3View = Get3DView();
	if (p3View) {
		p3View->resetRenderObjects();
	}
	m_heightMap = NEW_REF(WorldHeightMapEdit,(xExtent, yExtent, initialHeight, borderWidth));
	// note - mHeight map has ref count of 1.

	// Create a default water area.
	PolygonTrigger *pTrig = newInstance(PolygonTrigger)(4);
	ICoord3D loc;
	pTrig->setWaterArea(true);
	pTrig->setTriggerName(AsciiString("Default Water"));
	loc.x = -borderWidth*MAP_XY_FACTOR;
	loc.y = -borderWidth*MAP_XY_FACTOR;
	loc.z = TheGlobalData->m_waterPositionZ;
	pTrig->addPoint(loc);
	loc.x = (xExtent+borderWidth)*MAP_XY_FACTOR;
	pTrig->addPoint(loc);
	loc.y = (yExtent+borderWidth)*MAP_XY_FACTOR;
	pTrig->addPoint(loc);
	loc.x = -borderWidth*MAP_XY_FACTOR;
	pTrig->addPoint(loc);
	PolygonTrigger::addPolygonTrigger(pTrig);
	SetHeightMap(m_heightMap, true);
	/// @todo TerrainMaterial::updateTextures(m_heightMap) once the panel is ported.

	if (p3View) {
		p3View->setCenterInView(m_heightMap->getXExtent()/2-m_heightMap->getBorderSize(),
								m_heightMap->getYExtent()/2-m_heightMap->getBorderSize());
		p3View->setDefaultCamera();
	}
	SetModifiedFlag(false);
	return true;
}

Bool CWorldBuilderDoc::openDocument(const char *path)
{
	// clear out map-specific text, then load the map-local strings next to it.
	TheGameText->reset();
	AsciiString s = path;
	while (s.getLength() && s.getCharAt(s.getLength()-1) != '\\' && s.getCharAt(s.getLength()-1) != '/')
		s.removeLastChar();
	s.concat("map.str");
	DEBUG_LOG(("Looking for map-specific text in [%s]\n", s.str()));
	TheGameText->initMapStringFile(s);

	WorldHeightMapEdit *pOldHeightMap = m_heightMap;
	CachedFileInputStream theInputStream;
	if (!theInputStream.open(AsciiString(path))) {
		QMessageBox::warning(NULL, "WorldBuilder",
							 QString("Could not open map file '%1'.").arg(path));
		return false;
	}
	try {
		WbApp()->selectPointerTool();
		PolygonTrigger::deleteTriggers();
		ChunkInputStream *pStrm = &theInputStream;

		// Read the logical data (map objects, waypoints, etc.)
		WorldHeightMap *terrainHeightMap = new WorldHeightMap(pStrm, true);
		REF_PTR_RELEASE(terrainHeightMap);
		pStrm->absoluteSeek(0);
		// Read & keep the graphical data.
		m_heightMap = NEW_REF(WorldHeightMapEdit, (pStrm));
		pStrm->absoluteSeek(0);
		try {
			DataChunkInput file( pStrm );
			if (file.isValidFileType()) {	// Backwards compatible files aren't valid data chunk files.
				// Read the waypoints.
				file.registerParser( AsciiString("WaypointsList"), AsciiString::TheEmptyString, ParseWaypointDataChunk );
				if (!file.parse(this)) {
					throw(ERROR_CORRUPT_FILE_FORMAT);
				}
			}
		} catch(...) {
			// just eat the error - legacy files aren't chunk format.
		}
		theInputStream.close();

		validate();

		WbView3d *p3View = Get3DView();
		if (p3View) {
			p3View->resetRenderObjects();
		}
		m_heightMap->optimizeTiles(); // force to optimize tileset
		SetHeightMap(m_heightMap, true);
		Coord3D center;
		center.x = MAP_XY_FACTOR*m_heightMap->getXExtent()/2;
		center.y = MAP_XY_FACTOR*m_heightMap->getYExtent()/2;
		center.x -= m_heightMap->getBorderSize();
		center.y -= m_heightMap->getBorderSize();
		/* update objects. */
		AsciiString startingCamName = TheNameKeyGenerator->keyToName(TheKey_InitialCameraPosition);

		// always assign unique IDs. The things will still live in the correct layers, so this isn't
		// an especially big deal.
		MapObject::fastAssignAllUniqueIDs();

		MapObject *pMapObj = MapObject::getFirstMapObject();
		while (pMapObj) {
			/// @todo add objects & triggers to TheLayersList once the layers panel is ported.
			/// @todo pMapObj->setColor from ObjectOptions once the objects panel is ported.
			if (pMapObj->isWaypoint()) {
				if (pMapObj->getWaypointID() >= m_curWaypointID) {
					m_curWaypointID = pMapObj->getWaypointID();
				}
				if (startingCamName == pMapObj->getWaypointName()) {
					center = *pMapObj->getLocation();
				}
			}
			pMapObj = pMapObj->getNext();
		}

		REF_PTR_RELEASE(m_undoList);
		m_curRedo = 0;
		if (p3View) {
			p3View->setCenterInView(center.x/MAP_XY_FACTOR, center.y/MAP_XY_FACTOR);
		}
		REF_PTR_RELEASE(pOldHeightMap);
		if (p3View) {
			p3View->setDefaultCamera();
		}
	} catch(...) {
		m_heightMap = pOldHeightMap;
		QMessageBox::warning(NULL, "WorldBuilder",
							 QString("Failed to load map file '%1'.").arg(path));
		return false;
	}

	m_filePath = path;
	SetModifiedFlag(false);
	return true;
}

Bool CWorldBuilderDoc::saveDocument(const char *path)
{
	if (m_heightMap == NULL) {
		return false;
	}

	FILE *fp = fopen(path, "wb");
	if (fp == NULL) {
		QMessageBox::warning(NULL, "WorldBuilder",
							 QString("Could not write map file '%1'.").arg(path));
		return false;
	}
	try {
		Int i;
		/// @todo MapPreview generation once ported (original wrote a map preview).

		StdioFileOutputStream theStream(fp);
		DataChunkOutput *chunkWriter = new DataChunkOutput(&theStream);

		m_heightMap->saveToFile(*chunkWriter);
		/***************WAYPOINTS DATA ***************/
		chunkWriter->openDataChunk("WaypointsList", 	K_WAYPOINTS_VERSION_1);
		chunkWriter->writeInt(this->m_numWaypointLinks);
		for (i=0; i<m_numWaypointLinks; i++) {
			chunkWriter->writeInt(this->m_waypointLinks[i].waypoint1);
			chunkWriter->writeInt(this->m_waypointLinks[i].waypoint2);
		}
		chunkWriter->closeDataChunk();

		delete chunkWriter;
	} catch(...) {
		fclose(fp);
		QMessageBox::warning(NULL, "WorldBuilder", "The height map file write failed.");
		return false;
	}
	fclose(fp);

	m_filePath = path;
	SetModifiedFlag(false);
	m_needAutosave = false;
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// Waypoints

/**
* CWorldBuilderDoc::ParseWaypointDataChunk - read waypoint data chunk.
*/
Bool CWorldBuilderDoc::ParseWaypointDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	CWorldBuilderDoc *pThis = (CWorldBuilderDoc *)userData;
	return pThis->ParseWaypointData(file, info, userData);
}

/**
* CWorldBuilderDoc::ParseWaypointData - read waypoint data chunk.
* Format is the newer CHUNKY format.
*/
Bool CWorldBuilderDoc::ParseWaypointData(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	m_numWaypointLinks = file.readInt();
	Int i;
	for (i=0; i<m_numWaypointLinks; i++) {
		this->m_waypointLinks[i].waypoint1 = file.readInt();
		this->m_waypointLinks[i].waypoint2 = file.readInt();
	}
	DEBUG_ASSERTCRASH(file.atEndOfChunk(), ("Unexpected data left over."));
	return true;
}

void CWorldBuilderDoc::getWaypointLink(Int ndx, Int *waypoint1, Int *waypoint2)
{
	*waypoint1 = m_waypointLinks[ndx].waypoint1;
	*waypoint2 = m_waypointLinks[ndx].waypoint2;
}

/////////////////////////////////////////////////////////////////////////////
// Autosave

void CWorldBuilderDoc::autoSave(void)
{
	// srj sez: put autosave into our user data folder, not the ap dir
	QString autosave1 = QString("%1WorldBuilderAutoSave1.map").arg(TheGlobalData->getPath_UserData().str());
	QString autosave2 = QString("%1WorldBuilderAutoSave2.map").arg(TheGlobalData->getPath_UserData().str());
	QString autosave3 = QString("%1WorldBuilderAutoSave3.map").arg(TheGlobalData->getPath_UserData().str());

	if (m_heightMap == NULL) {
		return;
	}

	QFile::remove(autosave3);
	QFile::rename(autosave2, autosave3);
	QFile::rename(autosave1, autosave2);

	AsciiString savedPath = m_filePath;
	Bool savedModified = m_modified;
	saveDocument(autosave1.toUtf8().constData());
	m_filePath = savedPath;
	m_modified = savedModified;
	m_needAutosave = false;
}

/////////////////////////////////////////////////////////////////////////////
// Height map & undo

void CWorldBuilderDoc::SetHeightMap(WorldHeightMapEdit *pMap, Bool doUpdate)
{
	REF_PTR_SET(m_heightMap, pMap);
	if (doUpdate && m_3dView) {
		IRegion2D partialRange = {0,0,0,0};
		m_3dView->updateHeightMapInView(m_heightMap, false, partialRange);
	}
}

void CWorldBuilderDoc::AddAndDoUndoable(Undoable *pUndo)
{
	Undoable *pCurUndo = m_undoList;
	Int count = m_curRedo;
	while(count>0 && pCurUndo != NULL) {
		count--;
		pCurUndo = pCurUndo->GetNext();
	}
	m_needAutosave = true;
	m_curRedo = 0;
	pUndo->LinkNext(pCurUndo);
	REF_PTR_SET(m_undoList, pUndo);
	pUndo->Do();
	SetModifiedFlag();
	pCurUndo = m_undoList;
	count = 0;
	while (pCurUndo) {
		count++;
		if (count >= MAX_UNDOS) {
			pCurUndo->LinkNext(NULL);
			break;
		}
		pCurUndo = pCurUndo->GetNext();
	}
}

Bool CWorldBuilderDoc::canUndo(void)
{
	Undoable *pUndo = m_undoList;
	Int count = m_curRedo;
	while(count>0 && pUndo != NULL) {
		count--;
		pUndo = pUndo->GetNext();
	}
	return pUndo != NULL;
}

void CWorldBuilderDoc::OnEditUndo()
{
	Undoable *pUndo = m_undoList;
	m_needAutosave = true;
	Int count = m_curRedo;
	while(count>0 && pUndo != NULL) {
		count--;
		pUndo = pUndo->GetNext();
	}
	if (pUndo != NULL) {
		pUndo->Undo();
		SetModifiedFlag();
		m_curRedo++;
	}
}

void CWorldBuilderDoc::OnEditRedo()
{
	Undoable *pUndo = m_undoList;
	m_needAutosave = true;
	if (m_curRedo>0) {
		Int count = m_curRedo-1;
		while(count>0) {
			count--;
			pUndo = pUndo->GetNext();
		}
		DEBUG_ASSERTCRASH((pUndo != NULL),("oops"));
		if (pUndo) {
			pUndo->Redo();
			SetModifiedFlag();
			m_curRedo--;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// View update forwarding

void CWorldBuilderDoc::updateHeightMap(WorldHeightMap *htMap, Bool partial, const IRegion2D &partialRange)
{
	if (m_3dView) {
		m_3dView->updateHeightMapInView(htMap, partial, partialRange);
	}
}

void CWorldBuilderDoc::invalObject(MapObject *pMapObj)
{
	if (m_3dView) {
		m_3dView->invalObjectInView(pMapObj);
	}
}

void CWorldBuilderDoc::invalCell(int xIndex, int yIndex)
{
	if (m_3dView) {
		m_3dView->invalidateCellInView(xIndex, yIndex);
	}
}

void CWorldBuilderDoc::syncViewCenters(Real x, Real y)
{
	if (!m_linkCenters)
		return;
	if (m_3dView) {
		m_3dView->setCenterInView(x, y);
	}
}

void CWorldBuilderDoc::updateAllViews()
{
	if (m_3dView) {
		m_3dView->update();
	}
}

/////////////////////////////////////////////////////////////////////////////
// Cell/coordinate helpers

//=============================================================================
/** Given a cursor location, return the x and y index into the height map.
If the location is outside the height map, returns false. */
//=============================================================================
Bool CWorldBuilderDoc::getCellIndexFromCoord(Coord3D cpt, QPoint *ndxP)
{
	// Set up default return value.
	ndxP->setX(-1);
	ndxP->setY(-1);
	Bool inMap = true;

	WorldHeightMapEdit *pMap = GetHeightMap();
	if (pMap == NULL) return false;

	Int xIndex = floor(cpt.x/MAP_XY_FACTOR);
	xIndex += pMap->getBorderSize();

	// If negative, outside of map so return false.
	if (xIndex<0) {
		inMap = false;
		xIndex = 0;
	}
	// If larger than the map, return default.
	if (xIndex >= pMap->getXExtent()) {
		inMap = false;
		xIndex = pMap->getXExtent()-1;
	}
	Int yIndex = floor(cpt.y/MAP_XY_FACTOR);

	yIndex += pMap->getBorderSize();

	// If negative, outside of map so return default.
	if (yIndex<0) {
		inMap = false;
		yIndex = 0;
	}

	// If larger than the map, return default.
	if (yIndex >= pMap->getYExtent())  {
		inMap = false;
		yIndex = pMap->getYExtent()-1;
	}

	ndxP->setX(xIndex);
	ndxP->setY(yIndex);

	return inMap;
}

void CWorldBuilderDoc::getCoordFromCellIndex(QPoint ndx, Coord3D* pt)
{
	if (!pt) {
		return;
	}
	WorldHeightMap* hm = GetHeightMap();
	if (!hm) {
		return;
	}

	(*pt).x = (ndx.x() - hm->getBorderSize()) * MAP_XY_FACTOR;
	(*pt).y = (ndx.y() - hm->getBorderSize()) * MAP_XY_FACTOR;
}

//=============================================================================
/** Given a pixel position, returns the x/y location in the height map. */
//=============================================================================
Bool CWorldBuilderDoc::getCellPositionFromCoord(Coord3D cpt,  Coord3D *locP)
{
	// Set up default values.
	locP->x = -1;
	locP->y = -1;
	WorldHeightMapEdit *pMap = GetHeightMap();
	if (pMap == NULL) return(false);
	QPoint curNdx;
	if (getCellIndexFromCoord(cpt, &curNdx)) {
		locP->x = cpt.x;
		locP->y = cpt.y;
		return true;
	}
	return false;
}

//=============================================================================
/** Collect all cell indexes within widthOutside of the given rectangle.
Simplified port - the original walked out from seed indexes; a bounding box
scan gives the same set for the tools that use it. */
//=============================================================================
Bool CWorldBuilderDoc::getAllIndexesInRect(const Coord3D* bl, const Coord3D* br,
																					 const Coord3D* tl, const Coord3D* tr,
																					 Int widthOutside, VecHeightMapIndexes* allIndices)
{
	if (!(bl && br && tl && tr && allIndices)) {
		return false;
	}

	allIndices->clear();

	Real minX = min(min(bl->x, br->x), min(tl->x, tr->x));
	Real maxX = max(max(bl->x, br->x), max(tl->x, tr->x));
	Real minY = min(min(bl->y, br->y), min(tl->y, tr->y));
	Real maxY = max(max(bl->y, br->y), max(tl->y, tr->y));

	Coord3D lo = {minX, minY, 0};
	Coord3D hi = {maxX, maxY, 0};
	QPoint loNdx, hiNdx;
	getCellIndexFromCoord(lo, &loNdx);
	getCellIndexFromCoord(hi, &hiNdx);

	for (Int i = loNdx.x()-widthOutside; i <= hiNdx.x()+widthOutside; ++i) {
		for (Int j = loNdx.y()-widthOutside; j <= hiNdx.y()+widthOutside; ++j) {
			allIndices->push_back(QPoint(i, j));
		}
	}

	return (allIndices->size() > 0);
}

void CWorldBuilderDoc::validate(void)
{
	TheSidesList->validateSides();
	/// @todo port the remaining validation (team fixups etc.) from the MFC doc.
}
