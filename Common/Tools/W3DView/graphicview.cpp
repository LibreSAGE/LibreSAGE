/*
**	Command & Conquer Renegade(tm)
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

// GraphicView.cpp : implementation file
//
#include "graphicview.h"
#include "w3dview.h"
#include "ww3d.h"
#include "globals.h"
#include "w3dviewdoc.h"
#include "part_emt.h"
// #include "quat.h"
// #include "mainfrm.h"
#include "utils.h"
#include "hlod.h"
// #include "mmsystem.h"
#include "light.h"
#include "viewerassetmgr.h"

// Extra includes
#include "qsdlwindow.h"
#include "qfilewrapper.h"
#include <QDebug>
#include <QGuiApplication>
#include <QSize>
#include <QWindow>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <SDL3/SDL.h>

#include "ww3d.h"

////////////////////////////////////////////////////////////////////////////
//
//  CGraphicView
//
////////////////////////////////////////////////////////////////////////////
GraphicView::GraphicView(QWidget *parent)
	: QWidget(parent),
	  m_bInitialized (FALSE),		
	  m_pCamera (NULL),
	  m_TimerID (0),
	  m_bMouseDown (FALSE),
	  m_bRMouseDown (FALSE),
	  m_bActive (TRUE),
	  m_animationSpeed (1.0F),
	  m_dwLastFrameUpdate (0),
	  m_iWindowed (1),
	  m_animationState (AnimInvalid),
	  m_objectRotation (NoRotation),
	  m_LightRotation (NoRotation),
	  m_bLightMeshInScene (false),
	  m_pLightMesh (NULL),
	  m_ParticleCountUpdate (0),
	  m_CameraBonePosX (false),
	  m_UpdateCounter (0),
	  m_allowedCameraRotation (FreeRotation),
	  m_ObjectCenter (0.0f, 0.0f, 0.0f)
{
	QSdlWindow *sdlWindow = new QSdlWindow();
	QWidget *sdlWidget = QWidget::createWindowContainer(sdlWindow, this);
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(sdlWidget);

	// The native surface can be unavailable in the constructor on Wayland.
	sdlWindow->Initialize();
	m_pSDLWindow = sdlWindow->GetSDLWindow();

	setLayout(layout);

	// Set the global graphic view pointer to this instance
	W3DViewApp *pApp = qobject_cast<W3DViewApp *>(QApplication::instance());
	WWASSERT(pApp);
	pApp->m_graphicView = this;

	m_dwLastFrameUpdate = timeGetTime ();
}


////////////////////////////////////////////////////////////////////////////
//
//  ~CGraphicView
//
////////////////////////////////////////////////////////////////////////////
GraphicView::~GraphicView()
{
}

SDL_Window *GraphicView::GetSDLWindow() const
{
	return m_pSDLWindow;
}

void GraphicView::paintEvent(QPaintEvent *event)
{
	QWidget::paintEvent(event);
	QApplication::instance();
	qDebug() << "Paint event triggered. Active:" << m_bActive;
	W3DViewDoc *doc = GetCurrentDocument();

	//
	//	Render the background BMP
	//		
	WW3D::Begin_Render (TRUE, TRUE, doc->GetBackgroundColor ());
	WW3D::Render (doc->Get2DScene (), doc->Get2DCamera (), FALSE, FALSE);

	//
	// Render the background scene
	//
	if (!doc->GetBackgroundObjectName ().isEmpty()) {			
		WW3D::Render (doc->GetBackObjectScene (), doc->GetBackObjectCamera (), FALSE, FALSE);
	}

	//
	// Render the main scene
	//
	WW3D::Render (doc->GetScene (), m_pCamera, FALSE, FALSE);

	WW3D::End_Render();
}

////////////////////////////////////////////////////////////////////////////
//
//  OnSize
//
////////////////////////////////////////////////////////////////////////////
void GraphicView::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);

	QSize newSize = event->size();
	int cx = newSize.width();
	int cy = newSize.height();

	if (m_bInitialized)	{

		if (m_iWindowed == 0) {
			cx = g_iWidth;
			cy = g_iHeight;
		}

		// Change the resolution of the rendering device to 
		// match that of the view's current dimensions
		if (m_iWindowed == 1) {
			WW3D::Set_Device_Resolution (cx, cy, g_iBitsPerPixel, m_iWindowed);
		}

		// Force a repaint of the screen
		Reset_FOV ();
		update();
	}
}


////////////////////////////////////////////////////////////////////////////
//
//  InitializeGraphicView
//
////////////////////////////////////////////////////////////////////////////
BOOL
GraphicView::InitializeGraphicView(void)
{
	// Assume failure
	g_iDeviceIndex = 0;
	BOOL bReturn = FALSE;
	if (g_iDeviceIndex < 0)
	{
		return FALSE;
	}

	m_bInitialized = FALSE;

	QSize renderSize = this->size();
	int cx = renderSize.width();
	int cy = renderSize.height();

	bReturn = (WW3D::Set_Render_Device (g_iDeviceIndex,
													cx,
													cy,
													g_iBitsPerPixel,
													m_iWindowed) == WW3D_ERROR_OK);

	WWASSERT (bReturn);
	if (bReturn && (m_pCamera == NULL))
	{
		// Instantiate a new camera class
		m_pCamera = new CameraClass ();
		bReturn = (m_pCamera != NULL); 
		
		// Were we successful in creating a camera?
		WWASSERT (m_pCamera);
		if (m_pCamera)
		{	    
			// Create a transformation matrix
			Matrix3D transform (1);
			transform.Translate (Vector3 (0.0F, 0.0F, 35.0F));

			// Point the camera in this direction (I think)
			m_pCamera->Set_Transform (transform);
		}

		  //
		  //	Attach the 'listener' to the camera
		  //
		//   WWAudioClass::Get_Instance ()->Get_Sound_Scene ()->Attach_Listener_To_Obj (m_pCamera);
	}

	Reset_FOV ();

	if (m_pLightMesh == NULL)
	 {
		QFileWrapper light_mesh_file (":/w3d/Light.w3d");
		WW3DAssetManager::Get_Instance()->Load_3D_Assets (light_mesh_file);

		m_pLightMesh = WW3DAssetManager::Get_Instance()->Create_Render_Obj ("LIGHT");
		WWASSERT (m_pLightMesh != NULL);
		m_bLightMeshInScene = false;
	 }

	// Remember whether or not we are initialized
	 m_bInitialized = bReturn;

	// Return the TRUE/FALSE result code
	 return bReturn;
}

////////////////////////////////////////////////////////////////////////////
//
//  Set_Lowest_LOD
//
////////////////////////////////////////////////////////////////////////////
void
Set_Lowest_LOD (RenderObjClass *render_obj) 
{
	if (render_obj != NULL) {
		for (int index = 0; index < render_obj->Get_Num_Sub_Objects (); index ++) {
			RenderObjClass *psub_obj = render_obj->Get_Sub_Object (index);
			if (psub_obj != NULL) {
				Set_Lowest_LOD (psub_obj);
			}
			MEMBER_RELEASE (psub_obj);
		}

		//
		// Switcht this LOD to its lowest level
		//
		if (render_obj->Class_ID () == RenderObjClass::CLASSID_HLOD) {
			((HLodClass *)render_obj)->Set_LOD_Level (0);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Allow_Update
//
////////////////////////////////////////////////////////////////////////////
void
GraphicView::Allow_Update (bool onoff)
{
	if (onoff) {
		m_UpdateCounter --;
	} else {
		m_UpdateCounter ++;
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Reset_Camera_To_Display_Emitter
//
////////////////////////////////////////////////////////////////////////////
void
GraphicView::Reset_Camera_To_Display_Emitter (ParticleEmitterClass &emitter)
{
	// Get some of the emitter settings
	Vector3 velocity = emitter.Get_Start_Velocity ();
	const Vector3 &acceleration = emitter.Get_Acceleration ();
	float lifetime = emitter.Get_Lifetime ();

	// If the velocity is 0, then use the randomizer as the default velocity
	bool use_vel_rand = false;
	if ((velocity.X == 0) && (velocity.Y == 0) && (velocity.Z == 0)) {
		//velocity.Set (emitter.Get_Velocity_Random (), emitter.Get_Velocity_Random (), emitter.Get_Velocity_Random ());
		//use_vel_rand = true;
	}

	// Determine what the max extent covered by a particle will be.
	Vector3 distance = (velocity * lifetime) + ((acceleration * (lifetime * lifetime)) / 2.0F);

	// Do we need to take into account acceleration?
	Vector3 distance_maxima (0, 0, 0);
	if ((acceleration.X != 0) || (acceleration.Y != 0) || (acceleration.Z != 0)) {
		
		// Determine at what time (for each x,y,z) a maxima will occur.
		Vector3 time_max (0, 0, 0);
		time_max.X = (acceleration.X != 0) ? ((-velocity.X) / acceleration.X) : 0.00F;
		time_max.Y = (acceleration.Y != 0) ? ((-velocity.Y) / acceleration.Y) : 0.00F;
		time_max.Z = (acceleration.Z != 0) ? ((-velocity.Z) / acceleration.Z) : 0.00F;

		// Is there a maxima for the X direction?
		if ((time_max.X >= 0.0F) && (time_max.X < lifetime)) {
			distance_maxima.X = (velocity.X * time_max.X) + ((acceleration.X * (time_max.X * time_max.X)) / 2.0F);
			distance_maxima.X = fabs (distance_maxima.X);
		}

		// Is there a maxima for the Y direction?
		if ((time_max.Y >= 0.0F) && (time_max.Y < lifetime)) {
			distance_maxima.Y = (velocity.Y * time_max.Y) + ((acceleration.Y * (time_max.Y * time_max.Y)) / 2.0F);
			distance_maxima.Y = fabs (distance_maxima.Y);
		}

		// Is there a maxima for the Z direction?
		if ((time_max.Z >= 0.0F) && (time_max.Z < lifetime)) {
			distance_maxima.Z = (velocity.Z * time_max.Z) + ((acceleration.Z * (time_max.Z * time_max.Z)) / 2.0F);
			distance_maxima.Z = fabs (distance_maxima.Z);
		}
	}

	distance.X = fabs (distance.X);
	distance.Y = fabs (distance.Y);
	distance.Z = fabs (distance.Z);

	// Determine what the maximum distance convered in a single direction is
	float max_dist = max (distance.X, distance.Y);
	max_dist = max (max_dist, distance.Z);
	max_dist = max (max_dist, distance_maxima.X);
	max_dist = max (max_dist, distance_maxima.Y);
	max_dist = max (max_dist, distance_maxima.Z);

	Vector3 center = distance / 2.00F;
	center.X = max (center.X, distance_maxima.X / 2.00F);
	center.Y = max (center.Y, distance_maxima.Y / 2.00F);
	center.Z = max (center.Z, distance_maxima.Z / 2.00F);

	if (use_vel_rand) {
		center.Set (0, 0, 0);
	}

	// Build a logical sphere from the emitters settings
	// that should provide a good viewing distance for the emitter.
	SphereClass sphere;
	sphere.Center = center;
	sphere.Radius = max (emitter.Get_Particle_Size () * 5, (max_dist * 3.0F) / 5.0F);

	// View this sphere
	Reset_Camera_To_Display_Sphere (sphere);
	return ;
}

float minZoomAdjust = 0.0F;
Vector3 sphereCenter;
Quaternion rotation;

////////////////////////////////////////////////////////////////////////////
//
//  Reset_Camera_To_Display_Sphere
//
////////////////////////////////////////////////////////////////////////////
void
GraphicView::Reset_Camera_To_Display_Sphere (SphereClass &sphere)
{
	// Calculate a default camera distance to view this sphere
	m_CameraDistance = sphere.Radius * 3.00F;
	m_CameraDistance = (m_CameraDistance < 1.0F) ? 1.0F : m_CameraDistance;

	// Calculate a transform that is the appropriate distance
	// from the sphere center and is looking at the center
	Matrix3D transform (1);
	transform.Look_At (sphere.Center + Vector3 (m_CameraDistance, 0, 0), sphere.Center, 0);

	// Record some variables for later use
	sphereCenter	= sphere.Center;
	m_ObjectCenter	= sphereCenter;
	minZoomAdjust	= m_CameraDistance / 190.0F;
	rotation			= Build_Quaternion (transform);

	// Move the camera back to get a good view of the object
	m_pCamera->Set_Transform (transform);

	// Make the same adjustment for the scene light
	W3DViewDoc* doc = GetCurrentDocument ();
	LightClass *pSceneLight = doc->GetSceneLight ();	
	if ((m_pLightMesh != NULL) && (pSceneLight != NULL)) {

		// Reposition the light and its 'mesh' as appropriate
		transform.Make_Identity ();
		transform.Set_Translation (sphereCenter);
		transform.Translate (0, 0, 0.7F * m_CameraDistance);
		pSceneLight->Set_Transform (transform);
		m_pLightMesh->Set_Transform (transform);

		// Scale the light's mesh appropriately
		static float last_scale = 1.0F;
		m_pLightMesh->Scale (m_CameraDistance / (14 * last_scale));

		last_scale = m_CameraDistance / 14;
	}

	float max_dist = m_CameraDistance * 60.0F;
	float min_dist = max (0.2F, minZoomAdjust / 2);

	// Set the clipping planes so objects are clipped correctly
	if (doc->Are_Clip_Planes_Manual () == false) {
		m_pCamera->Set_Clip_Planes (min_dist, max_dist);

		// Adjust the fog near clipping plane to the new value, but
		// leave the far clip plane alone (since it is scene dependant
		// not camera dependant).
		float fog_near, fog_far;
		doc->GetScene()->Get_Fog_Range(&fog_near, &fog_far);
		doc->GetScene()->Set_Fog_Range(min_dist, fog_far);
		doc->GetScene()->Recalculate_Fog_Planes();
	}

	// Reset the background camera to match the main camera
	doc->GetBackObjectCamera ()->Set_Transform (transform);
	doc->GetBackObjectCamera ()->Set_Position (Vector3 (0.00F, 0.00F, 0.00F));

	// Update the camera distance in the status bar
	// CMainFrame *pCMainWnd = (CMainFrame *)::AfxGetMainWnd ();
	// if (pCMainWnd != NULL) {
	// 	pCMainWnd->UpdateCameraDistance (m_CameraDistance);
	// 	pCMainWnd->UpdateFrameCount (0, 0, 0);
	// }

	// Record the sphere we are viewing for later
	m_ViewedSphere = sphere;
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Reset_Camera_To_Display_Object
//
////////////////////////////////////////////////////////////////////////////
void
GraphicView::Reset_Camera_To_Display_Object (RenderObjClass &render_object)
{
	// Reset the camera to get a good look at this object's bounding sphere
	SphereClass sp = render_object.Get_Bounding_Sphere ();
	Reset_Camera_To_Display_Sphere (sp);		

	// Should we update the camera's position as well?
	int index = render_object.Get_Bone_Index ("CAMERA");
	if (index > 0) {

		// Convert the bone's transform into a camera transform
		Matrix3D	transform = render_object.Get_Bone_Transform (index);												
		if (m_CameraBonePosX) {
			Matrix3D tmp = transform;
			Matrix3D cam_transform (Vector3 (0, -1, 0), Vector3 (0, 0, 1), Vector3 (-1, 0, 0), Vector3 (0, 0, 0));
			transform = tmp * cam_transform;
		}

		// Pass the new transform onto the camera
		CameraClass *camera = GetCamera ();
		camera->Set_Transform (transform);
	}

	// Update the polygon count in the main window
	// CMainFrame *pCMainWnd = (CMainFrame *)::AfxGetMainWnd ();
	// if (pCMainWnd != NULL) {
	// 	pCMainWnd->UpdatePolygonCount (render_object.Get_Num_Polys ());
	// }

	// Load the settings in the default.dat if its in the local directory.
	// Load_Default_Dat ();
	return ;
}

////////////////////////////////////////////////////////////////////////////
//
//  Set_FOV
//
////////////////////////////////////////////////////////////////////////////
void
GraphicView::Set_FOV (double hfov, double vfov, bool force)
{
	// if (force || (doc->Is_FOV_Manual () == false)) {
		m_pCamera->Set_View_Plane (hfov, vfov);
	//}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Reset_FOV
//
////////////////////////////////////////////////////////////////////////////
void
GraphicView::Reset_FOV (void)
{
	int cx = 0;
	int cy = 0;

	if (m_iWindowed == 0) {
		cx = g_iWidth;
		cy = g_iHeight;
	} else {
		QRect rect = geometry();
		cx = rect.width();
		cy = rect.height();
	}

	// update the camera FOV settings
	// take the larger of the two dimensions, give it the
	// full desired FOV, then give the other dimension an
	// FOV proportional to its relative size
	double hfov,vfov;
	if (cy > cx) {
		vfov = DEG_TO_RAD(45.0f);
		hfov = (double)cx / (double)cy * vfov;
	} else  {
		hfov = DEG_TO_RAD(45.0f);
		vfov = (double)cy / (double)cx * hfov;
	}

	// Reset the field of view
	Set_FOV (hfov, vfov);
	return ;
}
