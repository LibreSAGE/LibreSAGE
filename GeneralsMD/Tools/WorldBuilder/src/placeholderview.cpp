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

// placeholderview.cpp : stub TheTacticalView for the W3DShadowManager.
// Lives in its own (Qt-free) translation unit: GameClient/View.h declares
// "friend class Display", which collides with the X11 Display typedef that
// Qt's qguiapplication.h drags in on Linux.

#include "Lib/BaseType.h"
#include "GameClient/View.h"

// The W3DShadowManager accesses TheTacticalView, so we have to create
// a stub class & object in Worldbuilder for it to access.
class PlaceholderView : public View
{
protected:
	Int m_width, m_height;																			///< Dimensions of the view
	Int m_originX, m_originY;																		///< Location of top/left view corner

protected:
	virtual View *prependViewToList( View *list ) {return NULL;};		///< Prepend this view to the given list, return the new list
	virtual View *getNextView( void ) { return NULL; }				///< Return next view in the set
public:

	virtual void init( void ){};

	virtual UnsignedInt getID( void ) { return 1; }

	virtual Drawable *pickDrawable( const ICoord2D *screen, Bool forceAttack, PickType pickType ){return NULL;};			///< pick drawable given the screen pixel coords

	/// all drawables in the 2D screen region will call the 'callback'
	virtual Int iterateDrawablesInRegion( IRegion2D *screenRegion,
																				Bool (*callback)( Drawable *draw, void *userData ),
																				void *userData ) {return 0;};
	virtual WorldToScreenReturn worldToScreenTriReturn( const Coord3D *w, ICoord2D *s ) { return WTS_INVALID; };	///< Transform world coordinate "w" into screen coordinate "s"
	virtual void screenToWorld( const ICoord2D *s, Coord3D *w ) {};	///< Transform screen coordinate "s" into world coordinate "w"
	virtual void screenToTerrain( const ICoord2D *screen, Coord3D *world ) {};  ///< transform screen coord to a point on the 3D terrain
	virtual void screenToWorldAtZ( const ICoord2D *s, Coord3D *w, Real z ) {};  ///< transform screen point to world point at the specified world Z value
	virtual void getScreenCornerWorldPointsAtZ( Coord3D *topLeft, Coord3D *topRight,
																							Coord3D *bottomLeft, Coord3D *bottomRight,
																							Real z ){};

	virtual void drawView( void ) {};															///< Render the world visible in this view.
	virtual void updateView( void ) {};															///< Render the world visible in this view.

	virtual void setZoomLimited( Bool limit ) {}			///< limit the zoom height
	virtual Bool isZoomLimited( void ) { return TRUE; }							///< get status of zoom limit

	virtual void setWidth( Int width ) { m_width = width; }
	virtual Int getWidth( void ) { return m_width; }
	virtual void setHeight( Int height ) { m_height = height; }
	virtual Int getHeight( void ) { return m_height; }
	virtual void setOrigin( Int x, Int y) { m_originX=x; m_originY=y;}				///< Sets location of top-left view corner on display
	virtual void getOrigin( Int *x, Int *y) { *x=m_originX; *y=m_originY;}			///< Return location of top-left view corner on display

	virtual void forceRedraw() { }

	virtual void lookAt( const Coord3D *o ){};														///< Center the view on the given coordinate
	virtual void initHeightForMap( void ) {};														///<  Init the camera height for the map at the current position.
	virtual void scrollBy( Coord2D *delta ){};														///< Shift the view by the given delta
	virtual void moveCameraTo(const Coord3D *o, Int frames, Int shutter,
														Bool orient, Real easeIn, Real easeOut) {lookAt(o);};
	virtual void moveCameraAlongWaypointPath(Waypoint *way, Int frames, Int shutter,
														Bool orient, Real easeIn, Real easeOut) {};
	virtual Bool isCameraMovementFinished(void) {return true;};
 	virtual void resetCamera(const Coord3D *location, Int frames, Real easeIn, Real easeOut) {}; ///< Move camera to location, and reset to default angle & zoom.
 	virtual void rotateCamera(Real rotations, Int frames, Real easeIn, Real easeOut) {}; ///< Rotate camera about current viewpoint.
	virtual void rotateCameraTowardObject(ObjectID id, Int milliseconds, Int holdMilliseconds, Real easeIn, Real easeOut) {};	///< Rotate camera to face an object, and hold on it
	virtual void cameraModFinalZoom(Real finalZoom, Real easeIn, Real easeOut){};			 ///< Final zoom for current camera movement.
	virtual void cameraModRollingAverage(Int framesToAverage){}; ///< Number of frames to average movement for current camera movement.
	virtual void cameraModFinalTimeMultiplier(Int finalMultiplier){}; ///< Final time multiplier for current camera movement.
	virtual void cameraModFinalPitch(Real finalPitch, Real easeIn, Real easeOut){};		 ///< Final pitch for current camera movement.
	virtual void cameraModFreezeTime(void){}					///< Freezes time during the next camera movement.
	virtual void cameraModFreezeAngle(void){}					///< Freezes time during the next camera movement.
	virtual void cameraModLookToward(Coord3D *pLoc){}			///< Sets a look at point during camera movement.
	virtual void cameraModFinalLookToward(Coord3D *pLoc){}			///< Sets a look at point during camera movement.
	virtual void cameraModFinalMoveTo(Coord3D *pLoc){ };			///< Sets a final move to.
	virtual Bool isTimeFrozen(void){ return false;}					///< Freezes time during the next camera movement.
	virtual Int	 getTimeMultiplier(void) {return 1;};				///< Get the time multiplier.
	virtual void setTimeMultiplier(Int multiple) {}; ///< Set the time multiplier.
	virtual void setDefaultView(Real pitch, Real angle, Real maxHeight) {};
	virtual void zoomCamera( Real finalZoom, Int milliseconds, Real easeIn, Real easeOut ) {};
	virtual void pitchCamera( Real finalPitch, Int milliseconds, Real easeIn, Real easeOut ) {};

	virtual void setAngle( Real angle ){};																///< Rotate the view around the up axis to the given angle
	virtual Real getAngle( void ) { return 0; }
	virtual void setPitch( Real angle ){};																	///< Rotate the view around the horizontal axis to the given angle
	virtual Real getPitch( void ) { return 0; }							///< Return current camera pitch
	virtual void setAngleAndPitchToDefault( void ){};													///< Set the view angle back to default
	virtual void getPosition(Coord3D *pos)	{ ;}											///< Return camera position

	virtual Real getHeightAboveGround() { return 1; }
	virtual void setHeightAboveGround(Real z) { }
	virtual Real getZoom() { return 1; }
	virtual void setZoom(Real z) { }
	virtual void zoomIn( void ) {  }																			///< Zoom in, closer to the ground, limit to min
	virtual void zoomOut( void ) {  }																		///< Zoom out, farther away from the ground, limit to max
	virtual void setZoomToDefault( void ) { }														///< Set zoom to default value
	virtual Real getMaxZoom( void ) { return 0.0f; }
	virtual void setOkToAdjustHeight( Bool val ) { }						///< Set this to adjust camera height

	virtual Real getTerrainHeightUnderCamera() { return 0.0f; }
	virtual void setTerrainHeightUnderCamera(Real z) { }
	virtual Real getCurrentHeightAboveGround() { return 0.0f; }
	virtual void setCurrentHeightAboveGround(Real z) { }

	virtual void getLocation ( ViewLocation *location ) {};								///< write the view's current location in to the view location object
	virtual void setLocation ( const ViewLocation *location ){};								///< set the view's current location from to the view location object

	virtual ObjectID getCameraLock() const { return INVALID_ID; }
	virtual void setCameraLock(ObjectID id) {  }
	virtual void snapToCameraLock( void ) {  }
	virtual void setSnapMode( CameraLockType lockType, Real lockDist ) {  }

	virtual Drawable *getCameraLockDrawable() const { return NULL; }
	virtual void setCameraLockDrawable(Drawable *drawable) { }

	virtual void setMouseLock( Bool mouseLocked ) {}					///< lock/unlock the mouse input to the tactical view
	virtual Bool isMouseLocked( void ) { return FALSE; }			///< is the mouse input locked to the tactical view?

	virtual void setFieldOfView( Real angle ) {};							///< Set the horizontal field of view angle
	virtual Real getFieldOfView( void ) {return 0;};										///< Get the horizontal field of view angle

	virtual Bool setViewFilterMode(enum FilterModes) {return FALSE;}	///<stub
	virtual void setViewFilterPos(const Coord3D *pos) {};	///<stub
	virtual void setFadeParameters(Int fadeFrames, Int direction) {};	///<stub
	virtual void set3DWireFrameMode(Bool enable) { }; ///<stub
	virtual Bool setViewFilter(		enum FilterTypes m_viewFilterMode) { return FALSE;}	///<stub
	virtual enum FilterModes	 getViewFilterMode(void) {return (enum FilterModes)0;}			///< Turns on viewport special effect (black & white mode)
	virtual enum FilterTypes	 getViewFilterType(void) {return (enum FilterTypes)0;}			///< Turns on viewport special effect (black & white mode)

	virtual void shake( const Coord3D *epicenter, CameraShakeType shakeType ) {};

	virtual Real getFXPitch( void ) const { return 1.0f; }
	virtual void forceCameraConstraintRecalc(void) { }
	virtual void rotateCameraTowardPosition(const Coord3D *pLoc, Int milliseconds, Real easeIn, Real easeOut, Bool reverseRotation) {};	///< Rotate camera to face an object, and hold on it

	virtual const Coord3D& get3DCameraPosition() const { static Coord3D dummy; return dummy; }							///< Returns the actual camera position

	virtual void setGuardBandBias( const Coord2D *gb ) {};

};

static PlaceholderView bogusTacticalView;

void WbInstallPlaceholderTacticalView()
{
	TheTacticalView = &bogusTacticalView;
}

void WbPlaceholderViewSetSize(Int width, Int height)
{
	bogusTacticalView.setWidth(width);
	bogusTacticalView.setHeight(height);
	bogusTacticalView.setOrigin(0,0);
}
