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
#include "w3dviewwin.h"
#include "ww3d.h"
#include "globals.h"
#include "w3dviewdoc.h"
#include "part_emt.h"
#include "quat.h"
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
#include <QElapsedTimer>
#include <QMouseEvent>
#include <QSize>
#include <QTimer>
#include <QWheelEvent>
#include <QWindow>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <SDL3/SDL.h>

#include "ww3d.h"

//
//	Camera/object orbit state shared between the mouse handlers and the
//	camera-reset helpers.
//
static float minZoomAdjust = 0.0F;
static Vector3 sphereCenter;
static Quaternion rotation;

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
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(sdlWidget);

	// The native surface can be unavailable in the constructor on Wayland.
	sdlWindow->Initialize();
	m_pSDLWindow = sdlWindow->GetSDLWindow();

	setLayout(layout);

	//
	//	Mouse/wheel events are delivered to the embedded native SDL window
	//	rather than to this QWidget, so we filter them off that window.
	//
	m_sdlQWindow = sdlWindow;
	sdlWindow->installEventFilter (this);

	//
	//	Set up the "game loop" timer that continuously re-renders the scene
	//	(advancing animations, particle systems and any active rotation).
	//
	m_renderTimer = new QTimer (this);
	connect (m_renderTimer, &QTimer::timeout, this, [this]() { RepaintView (); });

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

QSize GraphicView::sizeHint() const
{
	// Match the SDL window's default size so the splitter gives the viewport a
	// real width instead of collapsing the (hint-less) window container to 0.
	return QSize(640, 480);
}

SDL_Window *GraphicView::GetSDLWindow() const
{
	return m_pSDLWindow;
}

void GraphicView::paintEvent(QPaintEvent *event)
{
	QWidget::paintEvent(event);

	// Repaint without advancing the animation/rotation - this is just an
	// expose/refresh, the timer drives the actual animated updates.
	RepaintView (FALSE);
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

	//
	//	Kick off the render loop (~60 Hz) now that we are initialized.
	//
	if (m_bInitialized && (m_renderTimer != NULL) && !m_renderTimer->isActive ()) {
		m_dwLastFrameUpdate = timeGetTime ();
		m_renderTimer->start (16);
	}

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
//  SetActiveUpdate
//
////////////////////////////////////////////////////////////////////////////
void
GraphicView::SetActiveUpdate (BOOL bActive)
{
	m_bActive = bActive;

	if (m_renderTimer != NULL) {
		if (bActive) {
			if (m_bInitialized && !m_renderTimer->isActive ()) {
				m_dwLastFrameUpdate = timeGetTime ();
				m_renderTimer->start (16);
			}
		} else {
			m_renderTimer->stop ();
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  RepaintView
//
////////////////////////////////////////////////////////////////////////////
void
GraphicView::RepaintView (BOOL bUpdateAnimation, DWORD ticks_to_use)
{
	//
	//	Simple check to avoid re-entrance
	//
	static bool _already_painting = false;
	if (_already_painting) return;
	_already_painting = true;

	//
	// Are we in a valid state?
	//
	W3DViewDoc *doc = GetCurrentDocument ();
	if ((doc != NULL) && doc->Is_Initialized () && doc->GetScene () && (m_UpdateCounter == 0)) {

		// Advance the engine's sync time by the elapsed wall-clock time so that
		// time-based effects (particle emitters, etc.) animate.
		DWORD cur_ticks = timeGetTime ();
		int ticks_elapsed = cur_ticks - m_dwLastFrameUpdate;
		m_dwLastFrameUpdate = cur_ticks;

		if (ticks_to_use == 0) {
			WW3D::Sync (WW3D::Get_Sync_Time () + (DWORD)(ticks_elapsed * m_animationSpeed));
		} else {
			WW3D::Sync (WW3D::Get_Sync_Time () + ticks_to_use);
		}

		// Do we need to update the current (skeletal) animation?
		if ((m_animationState == AnimPlaying) && bUpdateAnimation) {
			// Advance the displayed animation by the elapsed wall-clock time
			// (in seconds), scaled by the current playback speed.
			float anim_speed = (((float)ticks_elapsed) / 1000.0F) * m_animationSpeed;
			doc->UpdateFrame (anim_speed);
		}

		// Perform the object rotation if necessary
		if ((m_objectRotation != NoRotation) && (bUpdateAnimation == TRUE)) {
			Rotate_Object ();
		}

		// Perform the light rotation if necessary
		if ((m_LightRotation != NoRotation) && (bUpdateAnimation == TRUE)) {
			Rotate_Light ();
		}

		// Reset the current LOD to be the lowest possible level so it can
		// re-evaluate which LOD to display this frame.
		RenderObjClass *prender_obj = doc->GetDisplayedObject ();
		if ((prender_obj != NULL) && (doc->GetScene ()->Are_LODs_Switching ())) {
			Set_Lowest_LOD (prender_obj);
		}

		//
		//	Render the background BMP
		//
		WW3D::Begin_Render (TRUE, TRUE, doc->GetBackgroundColor ());
		WW3D::Render (doc->Get2DScene (), doc->Get2DCamera (), FALSE, FALSE);

		//
		// Render the background scene
		//
		if (!doc->GetBackgroundObjectName ().isEmpty ()) {
			WW3D::Render (doc->GetBackObjectScene (), doc->GetBackObjectCamera (), FALSE, FALSE);
		}

		//
		// Render the main scene
		//
		QElapsedTimer timer;
    	timer.start();

		WW3D::Render (doc->GetScene (), m_pCamera, FALSE, FALSE);

		// TODO(animation): doc->Render_Dazzles (m_pCamera); once ported

		WW3D::End_Render ();
		
		qint64 milliseconds = timer.elapsed();
				//
		//	Update the count of particles and polys in the status bar
		//
		if ((cur_ticks - m_ParticleCountUpdate > 250)) {
			m_ParticleCountUpdate = cur_ticks;
			doc->Update_Particle_Count ();
			
			int polys = (prender_obj != NULL) ? prender_obj->Get_Num_Polys () : 0;
			Get_Main_Window()->UpdatePolygonCount (polys);
		}

		//
		//	Update the frame time in the status bar
		//
		Get_Main_Window()->Update_Frame_Time (milliseconds);
	}

	_already_painting = false;
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  SetAnimationState
//
////////////////////////////////////////////////////////////////////////////
void
GraphicView::SetAnimationState (ANIMATION_STATE animationState)
{
	m_animationState = animationState;

	// When (re)entering the playing state, reset the elapsed-time baseline so
	// the first advance after a pause/idle isn't the entire accumulated gap.
	if (animationState == AnimPlaying) {
		m_dwLastFrameUpdate = timeGetTime ();
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Rotate_Object
//
////////////////////////////////////////////////////////////////////////////
void
GraphicView::Rotate_Object (void)
{
	W3DViewDoc *doc = GetCurrentDocument ();
	if (doc == NULL) return;

	// Get the currently displayed object
	RenderObjClass *prender_obj = doc->GetDisplayedObject ();
	if (prender_obj != NULL) {

		// Get the current transform for the object
		Matrix3D transform = prender_obj->Get_Transform ();

		if ((m_objectRotation & RotateX) == RotateX) {
			transform.Rotate_X (0.05F);
		} else if ((m_objectRotation & RotateXBack) == RotateXBack) {
			transform.Rotate_X (-0.05F);
		}

		if ((m_objectRotation & RotateY) == RotateY) {
			transform.Rotate_Y (-0.05F);
		} else if ((m_objectRotation & RotateYBack) == RotateYBack) {
			transform.Rotate_Y (0.05F);
		}

		if ((m_objectRotation & RotateZ) == RotateZ) {
			transform.Rotate_Z (0.05F);
		} else if ((m_objectRotation & RotateZBack) == RotateZBack) {
			transform.Rotate_Z (-0.05F);
		}

		if (!transform.Is_Orthogonal ()) {
			transform.Re_Orthogonalize ();
		}

		// Set the new transform for the object
		prender_obj->Set_Transform (transform);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Rotate_Light
//
////////////////////////////////////////////////////////////////////////////
void
GraphicView::Rotate_Light (void)
{
	W3DViewDoc *doc = GetCurrentDocument ();
	if (doc == NULL) return;

	LightClass *pscene_light = doc->GetSceneLight ();
	RenderObjClass *prender_obj = doc->GetDisplayedObject ();
	if ((pscene_light != NULL) && (prender_obj != NULL)) {
		Matrix3D rotation_matrix (1);

		// Build a rotation matrix that contains the x,y,z
		// rotations we want to apply to the light
		if ((m_LightRotation & RotateX) == RotateX) {
			rotation_matrix.Rotate_X (0.05F);
		} else if ((m_LightRotation & RotateXBack) == RotateXBack) {
			rotation_matrix.Rotate_X (-0.05F);
		}

		if ((m_LightRotation & RotateY) == RotateY) {
			rotation_matrix.Rotate_Y (-0.05F);
		} else if ((m_LightRotation & RotateYBack) == RotateYBack) {
			rotation_matrix.Rotate_Y (0.05F);
		}

		if ((m_LightRotation & RotateZ) == RotateZ) {
			rotation_matrix.Rotate_Z (0.05F);
		} else if ((m_LightRotation & RotateZBack) == RotateZBack) {
			rotation_matrix.Rotate_Z (-0.05F);
		}

		//
		//	Now, use the rotation matrix to rotate the light 'around'
		//	the displayed object (in its coordinate system)
		//
		Matrix3D coord_inv;
		Matrix3D coord_to_obj;
		Matrix3D coord_system = prender_obj->Get_Transform ();
		coord_system.Get_Orthogonal_Inverse (coord_inv);

		Matrix3D transform = pscene_light->Get_Transform ();
		Matrix3D::Multiply (coord_inv, transform, &coord_to_obj);

		Matrix3D::Multiply (coord_system, rotation_matrix, &transform);
		Matrix3D::Multiply (transform, coord_to_obj, &transform);

		// Ensure the matrix hasn't degenerated
		if (!transform.Is_Orthogonal ()) {
			transform.Re_Orthogonalize ();
		}

		// Pass the new transform onto the light
		m_pLightMesh->Set_Transform (transform);
		pscene_light->Set_Transform (transform);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  eventFilter
//
////////////////////////////////////////////////////////////////////////////
bool
GraphicView::eventFilter (QObject *watched, QEvent *event)
{
	if (watched == m_sdlQWindow) {
		switch (event->type ()) {
			case QEvent::MouseButtonPress:
				Handle_Mouse_Press (static_cast<QMouseEvent *>(event));
				return true;
			case QEvent::MouseButtonRelease:
				Handle_Mouse_Release (static_cast<QMouseEvent *>(event));
				return true;
			case QEvent::MouseMove:
				Handle_Mouse_Move (static_cast<QMouseEvent *>(event));
				return true;
			case QEvent::Wheel:
				Handle_Wheel (static_cast<QWheelEvent *>(event));
				return true;
			default:
				break;
		}
	}

	return QWidget::eventFilter (watched, event);
}


////////////////////////////////////////////////////////////////////////////
//
//  Handle_Mouse_Press
//
////////////////////////////////////////////////////////////////////////////
void
GraphicView::Handle_Mouse_Press (QMouseEvent *event)
{
	m_lastPoint = event->position ().toPoint ();

	if (event->button () == Qt::LeftButton) {
		m_bMouseDown = TRUE;
	} else if (event->button () == Qt::RightButton) {
		m_bRMouseDown = TRUE;
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Handle_Mouse_Release
//
////////////////////////////////////////////////////////////////////////////
void
GraphicView::Handle_Mouse_Release (QMouseEvent *event)
{
	if (event->button () == Qt::LeftButton) {
		m_bMouseDown = FALSE;
	} else if (event->button () == Qt::RightButton) {
		m_bRMouseDown = FALSE;
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Handle_Mouse_Move
//
//  Left drag        : orbit the camera around the object
//  Right drag       : zoom (dolly) the camera in/out
//  Left+Right drag  : pan the camera
//  Ctrl + Left drag : rotate the scene light around the object
//  Ctrl + Right drag: move the scene light toward/away from the object
//
////////////////////////////////////////////////////////////////////////////
void
GraphicView::Handle_Mouse_Move (QMouseEvent *event)
{
	QPoint point = event->position ().toPoint ();
	int iDeltaY = m_lastPoint.y () - point.y ();

	W3DViewDoc *doc = GetCurrentDocument ();
	if ((doc == NULL) || (m_pCamera == NULL)) {
		m_lastPoint = point;
		return ;
	}

	bool ctrl = (event->modifiers () & Qt::ControlModifier) != 0;

	// Add/remove the light 'mesh' from the scene depending on the Ctrl key
	if (!ctrl && m_bLightMeshInScene) {
		m_pLightMesh->Remove ();
		m_bLightMeshInScene = false;
	} else if (ctrl && (m_bLightMeshInScene == false) && (m_pLightMesh != NULL)) {
		m_pLightMesh->Add (doc->GetScene ());
		m_bLightMeshInScene = true;
	}

	int cx = m_sdlQWindow->width ();
	int cy = m_sdlQWindow->height ();
	if ((cx <= 0) || (cy <= 0)) {
		m_lastPoint = point;
		return ;
	}

	float midPointX = float (cx >> 1);
	float midPointY = float (cy >> 1);
	float lastPointX = ((float)m_lastPoint.x () - midPointX) / midPointX;
	float lastPointY = (midPointY - (float)m_lastPoint.y ()) / midPointY;
	float pointX = ((float)point.x () - midPointX) / midPointX;
	float pointY = (midPointY - (float)point.y ()) / midPointY;

	// Pan the camera (both buttons)
	if (m_bMouseDown && m_bRMouseDown) {

		Matrix3D transform = m_pCamera->Get_Transform ();
		Vector3 cameraPan = Vector3 (-1.0F * m_CameraDistance * (pointX - lastPointX),
											  -1.0F * m_CameraDistance * (pointY - lastPointY),
											  0.0F);
		transform.Translate (cameraPan);

		Matrix3x3 view = Build_Matrix3 (rotation);
		Vector3 move = view * cameraPan;
		sphereCenter += move;

		m_pCamera->Set_Transform (transform);
	}
	// Rotate the scene light around the object (Ctrl + left)
	else if (ctrl && m_bMouseDown) {

		LightClass *pSceneLight = doc->GetSceneLight ();
		if ((pSceneLight != NULL) && (m_pLightMesh != NULL)) {

			Quaternion mouse_motion = Inverse (Trackball (lastPointX, lastPointY, pointX, pointY, 0.8F));
			Quaternion camera = Build_Quaternion (m_pCamera->Get_Transform ());
			Quaternion cur_light = Build_Quaternion (pSceneLight->Get_Transform ());

			Quaternion light_orientation = camera;
			light_orientation = light_orientation * mouse_motion;
			light_orientation = light_orientation * Inverse (camera);
			light_orientation = light_orientation * cur_light;
			light_orientation.Normalize ();

			Vector3 to_center;
			Matrix3D matrix = pSceneLight->Get_Transform ();
			Matrix3D::Inverse_Transform_Vector (matrix, sphereCenter, &to_center);

			Matrix3D light_tm (light_orientation, sphereCenter);
			light_tm.Translate (-to_center);

			m_pLightMesh->Set_Transform (light_tm);
			pSceneLight->Set_Transform (light_tm);
		}
	}
	// Move the light toward/away from the object (Ctrl + right)
	else if (ctrl && m_bRMouseDown) {

		LightClass *pscene_light = doc->GetSceneLight ();
		RenderObjClass *prender_obj = doc->GetDisplayedObject ();
		if ((pscene_light != NULL) && (prender_obj != NULL)) {

			float deltay = ((float)iDeltaY) / (float)cy;
			float adjustment = deltay * (m_ViewedSphere.Radius * 3.0F);

			Matrix3D transform = pscene_light->Get_Transform ();
			transform.Translate (Vector3 (0, 0, adjustment));

			Vector3 light_pos = transform.Get_Translation ();
			Vector3 obj_pos = prender_obj->Get_Position ();
			float distance = (light_pos - obj_pos).Length ();

			if (distance > m_ViewedSphere.Radius) {
				m_pLightMesh->Set_Transform (transform);
				pscene_light->Set_Transform (transform);
			}
		}
	}
	// Orbit the camera around the object (left only)
	else if (m_bMouseDown) {

		RenderObjClass *pCRenderObj = doc->GetDisplayedObject ();
		if (m_bInitialized && doc->GetScene () && pCRenderObj) {

			// Rotate around the object (orbit) using the mouse coordinates
			rotation = Trackball (lastPointX, lastPointY, pointX, pointY, 0.8F);

			// Optionally 'lock-out' all but a single rotation axis
			if (m_allowedCameraRotation == OnlyRotateX) {
				Matrix3D tempMatrix; Build_Matrix3D (rotation, tempMatrix);
				Matrix3D tempMatrix2 (1);
				tempMatrix2.Rotate_X (tempMatrix.Get_X_Rotation ());
				tempMatrix2.Set_Translation (tempMatrix.Get_Translation ());
				rotation = Build_Quaternion (tempMatrix2);
			} else if (m_allowedCameraRotation == OnlyRotateY) {
				Matrix3D tempMatrix; Build_Matrix3D (rotation, tempMatrix);
				Matrix3D tempMatrix2 (1);
				tempMatrix2.Rotate_Y (tempMatrix.Get_Y_Rotation ());
				tempMatrix2.Set_Translation (tempMatrix.Get_Translation ());
				rotation = Build_Quaternion (tempMatrix2);
			} else if (m_allowedCameraRotation == OnlyRotateZ) {
				Matrix3D tempMatrix; Build_Matrix3D (rotation, tempMatrix);
				Matrix3D tempMatrix2 (1);
				tempMatrix2.Rotate_Z (tempMatrix.Get_Z_Rotation ());
				tempMatrix2.Set_Translation (tempMatrix.Get_Translation ());
				rotation = Build_Quaternion (tempMatrix2);
			}

			// Get the transformation matrix for the camera and its inverse
			Matrix3D transform = m_pCamera->Get_Transform ();
			Matrix3D inverseMatrix;
			transform.Get_Orthogonal_Inverse (inverseMatrix);

			Vector3 to_object;
			Matrix3D::Transform_Vector (inverseMatrix, sphereCenter, &to_object);
			transform.Translate (to_object);

			Matrix3D rot_matrix; Build_Matrix3D (rotation, rot_matrix);
			Matrix3D::Multiply (transform, rot_matrix, &transform);

			transform.Translate (-to_object);

			// Rotate and translate the camera
			m_pCamera->Set_Transform (transform);

			doc->GetBackObjectCamera ()->Set_Transform (transform);
			doc->GetBackObjectCamera ()->Set_Position (Vector3 (0.0F, 0.0F, 0.0F));
		}
	}
	// Zoom (dolly) the camera (right only)
	else if (m_bRMouseDown) {

		Matrix3D transform = m_pCamera->Get_Transform ();
		if (iDeltaY != 0) {

			float deltay = ((float)iDeltaY) / (float)cy;
			float adjustment = deltay * m_CameraDistance * 3.0F;

			if ((adjustment < minZoomAdjust) && (adjustment >= 0.0F)) {
				adjustment = minZoomAdjust;
			}
			if ((adjustment > -minZoomAdjust) && (adjustment <= 0.0F)) {
				adjustment = -minZoomAdjust;
			}

			if ((m_CameraDistance + adjustment) > 0.0F) {
				m_CameraDistance += adjustment;
				transform.Translate (Vector3 (0.0F, 0.0F, adjustment));
				m_pCamera->Set_Transform (transform);

				doc->GetBackObjectCamera ()->Set_Transform (transform);
				doc->GetBackObjectCamera ()->Set_Position (Vector3 (0.0F, 0.0F, 0.0F));
			}
		}
	}

	m_lastPoint = point;
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Handle_Wheel
//
//  Mouse wheel zooms the camera in/out (a modern convenience the original
//  MFC tool lacked - it only zoomed via right-drag).
//
////////////////////////////////////////////////////////////////////////////
void
GraphicView::Handle_Wheel (QWheelEvent *event)
{
	if (m_pCamera == NULL) return;

	float notches = (float)event->angleDelta ().y () / 120.0F;
	if (notches == 0.0F) return;

	float adjustment = -notches * (m_CameraDistance * 0.1F);
	if ((m_CameraDistance + adjustment) <= 0.0F) return;

	m_CameraDistance += adjustment;

	Matrix3D transform = m_pCamera->Get_Transform ();
	transform.Translate (Vector3 (0.0F, 0.0F, adjustment));
	m_pCamera->Set_Transform (transform);

	W3DViewDoc *doc = GetCurrentDocument ();
	if (doc != NULL) {
		doc->GetBackObjectCamera ()->Set_Transform (transform);
		doc->GetBackObjectCamera ()->Set_Position (Vector3 (0.0F, 0.0F, 0.0F));
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
			Matrix3D::Multiply (tmp, cam_transform, &transform);
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
