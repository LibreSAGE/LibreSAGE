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

// wbview3d.h : the 3d view on the map (Qt6 port over QSdlWindow/W3D)
//

#pragma once

#include "wbview.h"

#include "Common/AsciiString.h"
#include "Common/GameCommon.h"
#include "Common/ModelState.h"
#include "Common/GlobalData.h"
#include "GameClient/Color.h"

#include "rendobj.h"
#include "refcount.h"
#include "lightenvironment.h"
#include "dx8wrapper.h"		// DX8_CleanupHook

class WBHeightMap;
class SkeletonSceneClass;
class CameraClass;
class LayerClass;
class LightClass;
class IntersectionClass;
class W3DAssetManager;
class DrawObject;
class MapObject;
class ThingTemplate;
class QSdlWindow;
class QTimer;
struct SDL_Window;

class WbView3d : public WbView, public DX8_CleanupHook
{
	Q_OBJECT

public:
	WbView3d(QWidget *parent = NULL);
	~WbView3d() override;

	// WbView interface --------------------------------------------------------
	Bool viewToDocCoords(QPoint curPt, Coord3D *newPt, Bool constrain=true) override;
	MapObject *picked3dObjectInView(QPoint viewPt) override;
	Bool docToViewCoords(Coord3D curPt, QPoint* newPt) override;
	void setCenterInView(Real x, Real y) override;
	void updateHeightMapInView(WorldHeightMap *htMap, Bool partial, const IRegion2D &partialRange) override;
	void invalObjectInView(MapObject *pMapObj) override;
	void invalidateCellInView(int xIndex, int yIndex) override;
	void scrollInView(Real x, Real y, Bool end) override;
	void setDefaultCamera() override;
	void rotateCamera(Real delta) override;
	void pitchCamera(Real delta) override;

	// 3d view operations -------------------------------------------------------
	void initWW3D();
	void shutdownWW3D();
	void resetRenderObjects();
	void updateLights();
	void updateTrees();
	void updateScorches();
	void redraw();
	void setCameraPitch(Real absolutePitch);
	Real getCameraPitch(void);
	Real getCurrentZoom(void);
	void zoomIn(void);
	void zoomOut(void);
	void zoomReset(void);
	void stepTimeOfDay();
	void reset3dEngineDisplaySize(Int width, Int height);

	AsciiString getModelNameAndScale(MapObject *pMapObj, Real *scale, BodyDamageType curDamageState);
	AsciiString getBestModelName(const ThingTemplate* tt, const ModelConditionFlags& c);

	Bool getShowWireframe(void) {return m_showWireframe;}
	void setShowWireframe(Bool show) {m_showWireframe = show; redraw();}
	Bool getShowEntireMap(void) {return m_showEntireMap;}
	void setShowEntireMap(Bool show);
	Bool getShowTopDownView(void) {return m_projection;}
	void setShowTopDownView(Bool show);

	// Feedback overlay (DrawObject) toggles -----------------------------------
	const ICoord2D &getActualWinSize(void) const {return m_actualWinSize;}

	Bool getShowObjects(void) {return m_showObjects;}
	void setShowObjects(Bool show) {m_showObjects = show; redraw();}
	Bool getShowWaypoints(void) {return m_showWaypoints;}
	void setShowWaypoints(Bool show) {m_showWaypoints = show; redraw();}
	Bool getShowBoundingBoxes(void) {return m_showBoundingBoxes;}
	void setShowBoundingBoxes(Bool show) {m_showBoundingBoxes = show; redraw();}
	Bool getShowSightRanges(void) {return m_showSightRanges;}
	void setShowSightRanges(Bool show) {m_showSightRanges = show; redraw();}
	Bool getShowWeaponRanges(void) {return m_showWeaponRanges;}
	void setShowWeaponRanges(Bool show) {m_showWeaponRanges = show; redraw();}
	Bool getShowSoundCircles(void) {return m_showSoundCircles;}
	void setShowSoundCircles(Bool show) {m_showSoundCircles = show; redraw();}
	Bool getHighlightTestArt(void) {return m_highlightTestArt;}
	void setHighlightTestArt(Bool show) {m_highlightTestArt = show; redraw();}
	Bool getShowLetterbox(void) {return m_showLetterbox;}
	void setShowLetterbox(Bool show) {m_showLetterbox = show; redraw();}

	/// Show/track the transparent placement-preview object for the object tool.
	void setObjTracking(MapObject *pMapObj, Coord3D pos, Real angle, Bool show);

	// DX8_CleanupHook: release/reacquire the overlay's default-pool D3D
	// resources (and the terrain's) around a device reset, else reset fails.
	void ReleaseResources(void) override;
	void ReAcquireResources(void) override;

protected:
	void wheelEvent(QWheelEvent *event) override;
	void resizeEvent(QResizeEvent *event) override;

private:
	void initAssets();
	void init3dScene();
	void setupCamera();
	void render();

protected:
	W3DAssetManager		*m_assetManager;
	SkeletonSceneClass	*m_scene;
	SkeletonSceneClass	*m_overlayScene;
	SkeletonSceneClass	*m_transparentObjectsScene;
	SkeletonSceneClass	*m_baseBuildScene;
	CameraClass			*m_camera;
	LightClass			*m_globalLight[MAX_GLOBAL_LIGHTS];
	WBHeightMap			*m_heightMapRenderObj;
	RefRenderObjListClass	m_lightList;
	LayerClass			*m_layer;
	LayerClass			*m_buildLayer;
	IntersectionClass	*m_intersector;

	QSdlWindow			*m_sdlWindow;
	SDL_Window			*m_pSDLWindow;
	QTimer				*m_renderTimer;

	Int					m_mouseWheelOffset;
	ICoord2D			m_actualWinSize;
	Real				m_cameraAngle;
	Real				m_FXPitch;
	Real				m_actualHeightAboveGround;
	Vector3				m_cameraSource;
	Vector3				m_cameraTarget;
	Coord3D				m_cameraOffset;
	Real				m_groundLevel;
	Real				m_curTrackingZ;

	Int					m_updateCount;
	Int					m_partialMapSize;
	Bool				m_needToLoadRoads;
	Bool				m_showEntireMap;
	Bool				m_showWireframe;
	Bool				m_projection;
	Bool				m_showShadows;
	Bool				m_ww3dInited;
	UnsignedInt			m_time;
	Real				m_buildRedMultiplier;

	// The feedback overlay (object markers, bounding boxes, ranges, etc.).
	DrawObject			*m_drawObject;
	Bool				m_showObjects;
	Bool				m_showWaypoints;
	Bool				m_showBoundingBoxes;
	Bool				m_showSightRanges;
	Bool				m_showWeaponRanges;
	Bool				m_showSoundCircles;
	Bool				m_highlightTestArt;
	Bool				m_showLetterbox;

	// The transparent placement preview driven by the object tool.
	RenderObjClass		*m_objectToolTrackingObj;
	Bool				m_showObjToolTrackingObj;
	AsciiString			m_objectToolTrackingModelName;
};
