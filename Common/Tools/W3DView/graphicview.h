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

#pragma once
// GraphicView.h : header file
#include <QWidget>
#include <QPoint>
//

// Qt forward declarations
class QTimer;
class QWindow;
class QMouseEvent;
class QWheelEvent;

/////////////////////////////////////////////////////////////////
//
//  Constants
//
#define ROTATION_X      0x01
#define ROTATION_Y      0x02
#define ROTATION_Z      0x04
#define ROTATION_X_BACK 0x08
#define ROTATION_Y_BACK 0x10
#define ROTATION_Z_BACK 0x20


// Forward declarations
class ParticleEmitterClass;


/////////////////////////////////////////////////////////////////////////////
// CGraphicView view

#include "camera.h"

struct SDL_Window;
class GraphicView : public QWidget
{
    Q_OBJECT
public:
    GraphicView(QWidget *parent = nullptr);
    ~GraphicView();

    QSize sizeHint() const override;

protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    // Mouse/wheel events arrive on the embedded SDL QWindow, so we filter them.
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

    public:

        /////////////////////////////////////////////////
        //
        //  Public Data Types
        //
        typedef enum
        {
            AnimInvalid = -1,
            AnimPlaying = 0,
            AnimStopped = 1,
            AnimPaused = 2
        } ANIMATION_STATE;

        typedef enum
        {
            CameraFront = -1,
            CameraBack = 0,
            CameraTop = 1,
            CameraBottom = 2,
            CameraLeft = 3,
            CameraRight = 4
        } CAMERA_POS;

        typedef enum
        {
            NoRotation = 0,
            RotateX = ROTATION_X,
            RotateY = ROTATION_Y,
            RotateZ = ROTATION_Z,
				RotateXBack = ROTATION_X_BACK,
				RotateYBack = ROTATION_Y_BACK,
				RotateZBack = ROTATION_Z_BACK
        } OBJECT_ROTATION;

        typedef enum
        {
            FreeRotation = 0,
            OnlyRotateX = ROTATION_X,
            OnlyRotateY = ROTATION_Y,
            OnlyRotateZ = ROTATION_Z
        } CAMERA_ROTATION;


        /////////////////////////////////////////////////
        //
        //  Public Methods
        //

        BOOL			InitializeGraphicView (void);
        SDL_Window * GetSDLWindow() const;

        //
		  //	Initial display methods
		  //
		  void			Reset_Camera_To_Display_Sphere (SphereClass &sphere);
		  void			Reset_Camera_To_Display_Object (RenderObjClass &physObject);
		  void			Reset_Camera_To_Display_Emitter (ParticleEmitterClass &emitter);
		  void			Load_Default_Dat (void);

        void			UpdateDisplay (void);
        void			RepaintView (BOOL bUpdateAnimation = TRUE, DWORD ticks_to_use = 0);
        void			SetActiveUpdate (BOOL bActive);

			void			Allow_Update (bool onoff);

        //
        // Animation methods
        //
        float					GetAnimationSpeed (void) const				{ return m_animationSpeed; }
        void					SetAnimationSpeed (float animationSpeed)	{ m_animationSpeed = animationSpeed; }
        ANIMATION_STATE		GetAnimationState (void) const				{ return m_animationState; }
        void					SetAnimationState (ANIMATION_STATE animationState);

        //
        // Camera Methods
        //
        void					SetAllowedCameraRotation (CAMERA_ROTATION cameraRotation);
        CAMERA_ROTATION		GetAllowedCameraRotation () const			{ return m_allowedCameraRotation; }
        void					SetCameraPos (CAMERA_POS cameraPos);
        class CameraClass *GetCamera (void) const							{ return m_pCamera; }

		  float					Get_Camera_Distance (void) const				{ return m_CameraDistance; }
		  void					Set_Camera_Distance (float dist);

		  void					Set_Camera_Bone_Pos_X (bool onoff)			{ m_CameraBonePosX = onoff; }
		  BOOL					Is_Camera_Bone_Pos_X (void) const			{ return m_CameraBonePosX; }

        //
        // Object rotation methods
        //
        void					ResetObject (void);
        void					RotateObject (OBJECT_ROTATION rotation);
        OBJECT_ROTATION		GetObjectRotation (void) const				{ return m_objectRotation; }

        //
        // Light rotation methods
        //
        void					Rotate_Light (OBJECT_ROTATION rotation)	{ m_LightRotation = rotation; }
        OBJECT_ROTATION		Get_Light_Rotation (void) const				{ return m_LightRotation; }

			//
			//	Fullscreen mode
			//
			BOOL					Is_Fullscreen (void) const						{ return !(BOOL)m_iWindowed; }
			void					Set_Fullscreen (bool fullscreen)				{ m_iWindowed = fullscreen ? 0 : 1; InitializeGraphicView (); }

			//
			//	Misc
			//
			RenderObjClass *	Get_Light_Mesh (void) const					{ return m_pLightMesh; }
			Vector3 &			Get_Object_Center (void)						{ return m_ObjectCenter; }

			//
			//	FOV methods
			//
			void					Set_FOV (double hfov, double vfov, bool force = false);
			void					Reset_FOV (void);

    protected:

        /////////////////////////////////////////////////
        //
        //  Protected methods
        //
		  void					Rotate_Object (void);
		  void					Rotate_Light (void);

		  //
		  //	Mouse interaction handlers (driven by eventFilter)
		  //
		  void					Handle_Mouse_Press (QMouseEvent *event);
		  void					Handle_Mouse_Release (QMouseEvent *event);
		  void					Handle_Mouse_Move (QMouseEvent *event);
		  void					Handle_Wheel (QWheelEvent *event);

    private:

        /////////////////////////////////////////////////
        //
        //  Private Member Data
        //
        BOOL					m_bInitialized;
        BOOL					m_bActive;
        UINT					m_TimerID;
        CameraClass	*		m_pCamera;
        RenderObjClass *	m_pLightMesh;
        bool					m_bLightMeshInScene;
        Vector3				m_ObjectCenter;
        SphereClass			m_ViewedSphere;		  

        BOOL					m_bMouseDown;
        BOOL					m_bRMouseDown;
        int						m_iWindowed;
        int						m_UpdateCounter;
        float					m_CameraDistance;
        DWORD					m_ParticleCountUpdate;
        BOOL					m_CameraBonePosX;
        
        // Animation data
        DWORD					m_dwLastFrameUpdate;
        float					m_animationSpeed;
        ANIMATION_STATE		m_animationState;
        OBJECT_ROTATION		m_objectRotation;
        OBJECT_ROTATION		m_LightRotation;
        CAMERA_ROTATION		m_allowedCameraRotation;

        SDL_Window *		m_pSDLWindow;

        // Render loop + input plumbing
        QTimer *				m_renderTimer;
        QWindow *			m_sdlQWindow;
        QPoint				m_lastPoint;
};