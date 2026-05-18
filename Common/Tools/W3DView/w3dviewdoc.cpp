#include "w3dviewdoc.h"
#include "w3dview.h"
#include "ffactory.h"
#include "globals.h"
#include "viewerassetmgr.h"
#include "globals.h"
#include "rendobj.h"
#include "graphicview.h"
#include "distlod.h"
#include "light.h"
#include "camera.h"
#include "w3d_file.h"
#include "wwfile.h"
#include "bmp2d.h"
#include "part_emt.h"
#include "part_ldr.h"
#include "utils.h"
#include "w3derr.h"
#include "chunkio.h"
#include "meshmdl.h"
#include "agg_def.h"
#include "hlod.h"
#include "viewerscene.h"
#include "ini.h"
#include "ww3d.h"
#include "mesh.h"
#include "sphereobj.h"
#include "ringobj.h"
#include "textfile.h"
#include "hmorphanim.h"
#include "soundrobj.h"
#include "dazzle.h"
/// Extra includes
#include <QSettings>

///////////////////////////////////////////////////////////////
//
//  W3DViewDoc
//
W3DViewDoc::W3DViewDoc (void)
    : m_pCScene (NULL),
      m_pC2DScene (NULL),
		m_pCursorScene (NULL),
      m_pCBackObjectScene (NULL),
		m_pDazzleLayer (NULL),
      m_pCBackObjectCamera (NULL),  
      m_pCBackgroundObject (NULL),
      m_pC2DCamera (NULL),
      m_pCSceneLight (NULL),
      m_pCRenderObj (NULL),
      m_pCAnimation (NULL),
		m_pCAnimCombo (NULL),
		m_pCBackgroundBMP (NULL),		
      m_CurrentFrame (0),
      m_bAnimBlend (TRUE),
		m_bAnimateCamera (false),
		m_bAutoCameraReset (true),
		m_bOneTimeReset (true),
		m_pCursor (NULL),
      m_backgroundColor (0.5F, 0.5F, 0.5F),
		m_ManualFOV (false),
		m_ManualClipPlanes (false),
		m_IsInitialized (false),
		m_bFogEnabled(false),
		m_nChannelQnBytes(2),
		m_bCompress_channel_Q(false)
{
	// Read the camera animation settings from the registry
    QSettings settings(QSettings::Scope::UserScope);
	m_bAnimateCamera = settings.value("AnimateCamera", 0).toBool();
	m_bAutoCameraReset = settings.value("ResetCamera", 1).toBool();

	InitScene ();
	return ;
}

///////////////////////////////////////////////////////////////
//
//  ~W3DViewDoc
//
///////////////////////////////////////////////////////////////
W3DViewDoc::~W3DViewDoc (void)
{
    CleanupResources ();
    return ;
}


///////////////////////////////////////////////////////////////
//
//  CleanupResources
//
///////////////////////////////////////////////////////////////
void
W3DViewDoc::CleanupResources (void)
{
    if (m_pC2DScene)
    {
		  if (m_pCBackgroundBMP)
        {
            // Remove the background BMP from the scene
				m_pCBackgroundBMP->Remove ();
        }

        // Release the 2D scene we allocated to display background BMPs
        m_pC2DScene->Release_Ref ();
        m_pC2DScene = NULL;
    }

    if (m_pCBackObjectScene)
    {
        if (m_pCBackgroundObject)
        {
            // Remove the background BMP from the scene
				m_pCBackgroundObject->Remove ();
        }

        // Release the scene we allocated to display background objects
        m_pCBackObjectScene->Release_Ref ();
        m_pCBackObjectScene = NULL;
    }

	// if (m_pCursor != NULL) {
	// 	m_pCursor->Remove ();		
	// }
	MEMBER_RELEASE (m_pCursorScene);

    if (m_pCScene)
    {
        if (m_pCRenderObj)
        {
            // Remove the currently displayed object from the scene
				Remove_Object_From_Scene (m_pCRenderObj);
        }

        if (m_pCSceneLight)
        {
            // Remove the light from the scene
				Remove_Object_From_Scene (m_pCSceneLight);
        }

		  // Get rid of the lined up objects.
		  m_pCScene->Clear_Lineup();

        // Release the scene object we allocated earlier
        m_pCScene->Release_Ref ();
        m_pCScene = NULL;
    }

	 // Was there a dazzle layer?
	 if (m_pDazzleLayer) {
		 delete m_pDazzleLayer;
		 m_pDazzleLayer = NULL;
	 }

    // Was there a valid scene object?
    if (m_pCBackObjectScene)
    {
        // Free the scene object
        m_pCBackObjectScene->Release_Ref ();
        m_pCBackObjectScene = NULL;
    }

    // Was there a valid 2D camera?
    if (m_pC2DCamera)
    {
        // Free the camera object
        m_pC2DCamera->Release_Ref ();
        m_pC2DCamera = NULL;
    }

    // Was there a valid background camera?
    if (m_pCBackObjectCamera)
    {
        // Free the camera object
        m_pCBackObjectCamera->Release_Ref ();
        m_pCBackObjectCamera = NULL;
    }

    // Was there a valid background BMP?
    if (m_pCBackgroundBMP)
    {
        m_pCBackgroundBMP->Release_Ref ();
        m_pCBackgroundBMP = NULL;
    }

    // Was there a valid scene light?
    if (m_pCSceneLight)
    {
        m_pCSceneLight->Release_Ref ();
        m_pCSceneLight = NULL;
    }

    // Was there a valid display object?
    if (m_pCRenderObj)
    {
        // Free the currently displayed object
			SAFE_DELETE (m_pCAnimCombo);
			MEMBER_RELEASE (m_pCAnimation);
			MEMBER_RELEASE (m_pCRenderObj);
    }

    return ;
}

///////////////////////////////////////////////////////////////
//
//  InitScene
//
///////////////////////////////////////////////////////////////
void
W3DViewDoc::InitScene (void)
{
	if (m_pCScene == NULL) {

		//
		//	Make sure the emitters don't remove themselves from the scene
		// when they are finished emitting...
		//
		ParticleEmitterClass::Set_Default_Remove_On_Complete (false);

		m_pCScene = new ViewerSceneClass;
		WWASSERT (m_pCScene);
		if (m_pCScene != NULL) {

			// Set some default ambient lighting
			m_pCScene->Set_Ambient_Light (Vector3 (0.5F, 0.5F, 0.5F));

			// Set up the correct fog color.
			m_pCScene->Set_Fog_Color(GetBackgroundColor());

			// Create a new scene light
			m_pCSceneLight = new LightClass;
			WWASSERT (m_pCSceneLight);

			if (m_pCSceneLight != NULL) {

				// Create some default light settings
				m_pCSceneLight->Set_Position (Vector3 (0, 5000, 3000));
				m_pCSceneLight->Set_Intensity (1.0F);
				m_pCSceneLight->Set_Force_Visible(true);
				m_pCSceneLight->Set_Flag (LightClass::NEAR_ATTENUATION, false);
				m_pCSceneLight->Set_Far_Attenuation_Range (1000000, 1000000);
				m_pCSceneLight->Set_Ambient(Vector3(0,0,0));
				m_pCSceneLight->Set_Diffuse (Vector3(1, 1, 1));
				m_pCSceneLight->Set_Specular (Vector3(1, 1, 1));

				// Add this light to the scene
				m_pCScene->Add_Render_Object (m_pCSceneLight);
			}
		}

		// Instantiate a new 2D scene
		m_pC2DScene = new SimpleSceneClass;
		WWASSERT (m_pC2DScene);

		// Instantiate a new 2D cursor scene
		m_pCursorScene = new SimpleSceneClass;
		WWASSERT (m_pCursorScene);

		// Create_Cursor ();
		// m_pCursorScene->Add_Render_Object (m_pCursor);


		m_pCBackObjectScene = new SimpleSceneClass;

		// Were we successful in instantiating the scene object?
		WWASSERT (m_pCBackObjectScene);
		if (m_pCBackObjectScene) {

			// Set the default ambient light for the background object
			m_pCBackObjectScene->Set_Ambient_Light (Vector3 (0.5F, 0.5F, 0.5F));
		}

			
		// Create a new instance of the camera class to use
		// when rendering the background object
		m_pCBackObjectCamera = new CameraClass ();

		// Were we successful in creating the new instance?
		WWASSERT (m_pCBackObjectCamera);
		if (m_pCBackObjectCamera) {

			// Set the default values for the new camera
			m_pCBackObjectCamera->Set_View_Plane (Vector2 (-1.00F, -1.00F), Vector2 (1.00F, 1.00F));
			m_pCBackObjectCamera->Set_Position (Vector3 (0.00F, 0.00F, 0.00F));
			m_pCBackObjectCamera->Set_Clip_Planes (0.1F, 10.0F);
		}        

		// Create a new instance of the camera class to use
		// when rendering the background BMP
		m_pC2DCamera = new CameraClass ();

		// Were we successful in creating the new instance?
		WWASSERT (m_pC2DCamera);		
		if (m_pC2DCamera) {

			// Set the default values for the new camera
			m_pC2DCamera->Set_View_Plane (Vector2 (-1.00F, -1.00F), Vector2 (1.00F, 1.00F));
			m_pC2DCamera->Set_Position (Vector3 (0.00F, 0.00F, 1.00F));
			m_pC2DCamera->Set_Clip_Planes (0.1F, 10.0F);
		}

		//
		// Read the texture paths from the registry
		//
        QSettings settings(QSettings::Scope::UserScope);
		QString path1 = settings.value("TexturePath1", "").toString();
		QString path2 = settings.value("TexturePath2", "").toString();
		Set_Texture_Path1 (path1.toUtf8().constData());
		Set_Texture_Path2 (path2.toUtf8().constData());

		// Construct a dazzle layer object 
		m_pDazzleLayer  = new DazzleLayerClass();
		DazzleRenderObjClass::Set_Current_Dazzle_Layer(m_pDazzleLayer);

		// Enable fog if appropriate.
		if (IsFogEnabled()) {
			m_pCScene->Set_Fog_Enable(true);
		}
	}

	Load_Camera_Settings ();
	m_IsInitialized = true;
	return ;
}


///////////////////////////////////////////////////////////////
//
//  LoadAssetsFromFile
//
///////////////////////////////////////////////////////////////
void
W3DViewDoc::LoadAssetsFromFile (const char* lpszPathName) 
{
	if (m_pCScene == NULL) {
		InitScene ();
	}

	//
	//	Remember the last path we opened
	//
	m_LastPath = ::Strip_Filename_From_Path (lpszPathName);

	//
	//	Add this path to the load list
	//
	m_LoadList.Add (lpszPathName);
	
	//
	//	Don't allow repaints while the load is going on
	//
	GraphicView *current_view = ::Get_Graphic_View ();
	if (current_view != NULL) {
		current_view->Allow_Update (false);
	}

	//
	// HACK HACK -- Force the current directory to be the directory
	//              the file is located in.
	//
	if (::strrchr (lpszPathName, '/')) {
		QString stringTemp = lpszPathName;
		stringTemp = stringTemp.left ((long)::strrchr (lpszPathName, '/') - (long)lpszPathName);
        #ifdef _WIN32
		::SetCurrentDirectory (stringTemp.toUtf8().constData());
        #else
        chdir(stringTemp.toUtf8().constData());
        #endif
		_TheSimpleFileFactory->Append_Sub_Directory(stringTemp.toUtf8().constData());
	}

	const char* extension = ::strrchr (lpszPathName, '.');
	if (::lstrcmpi (extension, ".tga") == 0 || ::lstrcmpi (extension, ".dds") == 0) {
		
		// Load the texture file into the asset manager
        QString filename = ::Get_Filename_From_Path (lpszPathName);
		TextureClass *ptexture = WW3DAssetManager::Get_Instance()->Get_Texture (filename.toUtf8().constData());
		if (ptexture != NULL) {
			ptexture->Release_Ref();
		}

	} else {
        QString filename = ::Get_Filename_From_Path (lpszPathName);
		WW3DAssetManager::Get_Instance()->Load_3D_Assets (filename.toUtf8().constData());
	}

	//
	//	Turn repainting back on...
	//
	if (current_view != NULL) {
		current_view->Allow_Update (true);
	}

	return ;
}

///////////////////////////////////////////////////////////////
//
//  Display_Emitter
//
///////////////////////////////////////////////////////////////
void
W3DViewDoc::Display_Emitter
(
	ParticleEmitterClass *pemitter,
	bool use_global_reset_flag,
	bool allow_reset
)
{
	WWASSERT (m_pCScene);

	// Data OK?
	if (m_pCScene != NULL) {

		// Lose the animation
		SAFE_DELETE (m_pCAnimCombo);
		MEMBER_RELEASE (m_pCAnimation);

			if (m_pCRenderObj != NULL) {
				
				// Remove this object from the scene
				Remove_Object_From_Scene (m_pCRenderObj);
				m_pCRenderObj->Release_Ref ();
				m_pCRenderObj = NULL;
			}
			m_pCScene->Clear_Lineup();

		// Do we have a new emitter to display?
		if (pemitter != NULL) {

			// Add the emitter to the scene
			pemitter->Set_Transform (Matrix3D (1));			
			MEMBER_ADD (m_pCRenderObj, pemitter);
			m_pCScene->Add_Render_Object (m_pCRenderObj);
			pemitter->Start ();

			GraphicView *pGraphicView = GetGraphicView ();
			if (pGraphicView) {

				// Try to find a good view for the emitter
				if ((use_global_reset_flag && m_bAutoCameraReset) ||
					 ((use_global_reset_flag == false) && allow_reset) ||
					 m_bOneTimeReset) {
					pGraphicView->Reset_Camera_To_Display_Emitter (*pemitter);
					m_bOneTimeReset = false;
				}
			}
		}
	}
	
	return ;
}


///////////////////////////////////////////////////////////////
//
//  DisplayObject
//
///////////////////////////////////////////////////////////////
void
W3DViewDoc::DisplayObject
(
	RenderObjClass *pCModel,
	bool use_global_reset_flag,
	bool allow_reset,
	bool add_ghost
)
{
    WWASSERT (m_pCScene);

    // Data OK?
    if (m_pCScene)
    {
        // Lose the animation
		  SAFE_DELETE (m_pCAnimCombo);
		  MEMBER_RELEASE (m_pCAnimation);

        // Do we have an old object to remove from the scene?
		  if (add_ghost == false) {
			  if (m_pCRenderObj)
			  {
					// Remove this object from the scene
					Remove_Object_From_Scene (m_pCRenderObj);
					m_pCRenderObj->Release_Ref ();
					m_pCRenderObj = NULL;
			  }
		  }
		  m_pCScene->Clear_Lineup();

        // Do we have a new object to display?
        if (pCModel && (add_ghost == false))
        {
            // Reset the animation for this object
            pCModel->Set_Animation ();

            m_pCRenderObj = pCModel;
            m_pCRenderObj->Add_Ref ();
            m_pCRenderObj->Set_Transform (Matrix3D (1));

            // Add this object to the scene
				if (m_pCRenderObj->Class_ID () == RenderObjClass::CLASSID_BITMAP2D) {
					m_pC2DScene->Add_Render_Object (m_pCRenderObj);
				} else {
					m_pCScene->Add_Render_Object (m_pCRenderObj);
				}

				// Reset the current lod to be the lowest possible LOD...
				if ((m_pCScene->Are_LODs_Switching ()) &&
					 (m_pCRenderObj->Class_ID () == RenderObjClass::CLASSID_HLOD)) {
					((HLodClass *)m_pCRenderObj)->Set_LOD_Level (0);
				}

            GraphicView *pGraphicView = GetGraphicView ();
            if (pGraphicView)
            {
                // Reset the camera to so the user can see
                // the whole object
                if ((use_global_reset_flag && m_bAutoCameraReset) ||
					     ((use_global_reset_flag == false) && allow_reset) ||
						  m_bOneTimeReset) {
						pGraphicView->Reset_Camera_To_Display_Object (*m_pCRenderObj);
						m_bOneTimeReset = false;
					 }
            }
        }
		  else if (pCModel) {
            // Reset the animation for this object
            pCModel->Set_Animation ();

				RenderObjClass *m_pCRenderObj;

            m_pCRenderObj = pCModel;
            m_pCRenderObj->Add_Ref ();
            m_pCRenderObj->Set_Transform (Matrix3D (1));

            // Add this object to the scene
				if (m_pCRenderObj->Class_ID () == RenderObjClass::CLASSID_BITMAP2D) {
					m_pC2DScene->Add_Render_Object (m_pCRenderObj);
				} else {
					m_pCScene->Clear_Lineup();
					m_pCScene->Add_Render_Object (m_pCRenderObj);
				}

				// Reset the current lod to be the lowest possible LOD...
				if ((m_pCScene->Are_LODs_Switching ()) &&
					 (m_pCRenderObj->Class_ID () == RenderObjClass::CLASSID_HLOD)) {
					((HLodClass *)m_pCRenderObj)->Set_LOD_Level (0);
				}

            GraphicView *pGraphicView = GetGraphicView ();
            if (pGraphicView)
            {
                // Reset the camera to so the user can see
                // the whole object
                if ((use_global_reset_flag && m_bAutoCameraReset) ||
					     ((use_global_reset_flag == false) && allow_reset) ||
						  m_bOneTimeReset) {
						pGraphicView->Reset_Camera_To_Display_Object (*m_pCRenderObj);
						m_bOneTimeReset = false;
					 }
            }
		  }
    }

    return ;
}

///////////////////////////////////////////////////////////////
//
//  GetGraphicView
//
///////////////////////////////////////////////////////////////
GraphicView *
W3DViewDoc::GetGraphicView (void)
{
    GraphicView *pGraphicView = NULL;

    // Get a pointer to the application instance
	W3DViewApp *pApp = qobject_cast<W3DViewApp *>(QApplication::instance());
	WWASSERT(pApp);

    pApp->GetGraphicView();

    // Return a pointer to the graphic view
    return pGraphicView;
}

///////////////////////////////////////////////////////////////
//
//  Remove_Object_From_Scene
//
///////////////////////////////////////////////////////////////
void
W3DViewDoc::Remove_Object_From_Scene (RenderObjClass *prender_obj)
{
	// If the render object is NULL, then remove the current render object
	if (prender_obj == NULL) {
		prender_obj = m_pCRenderObj;
	}

	// Recursively remove objects from the scene (to make sure we get all particle buffers)
	//for (int index = 0; index < prender_obj->Get_Num_Sub_Objects (); index ++) {
	while (prender_obj->Get_Num_Sub_Objects () > 0) {
		RenderObjClass *psub_obj = prender_obj->Get_Sub_Object (0);
		if (psub_obj != NULL) {
			Remove_Object_From_Scene (psub_obj);
		}
		MEMBER_RELEASE (psub_obj);
	}

	// If this is an emitter, then remove its buffer
	if ((prender_obj != NULL) &&
		 prender_obj->Class_ID () == RenderObjClass::CLASSID_PARTICLEEMITTER) {
		
		// Attempt to remove this emitter's buffer
		((ParticleEmitterClass *)prender_obj)->Stop ();
		((ParticleEmitterClass *)prender_obj)->Remove_Buffer_From_Scene ();
		((ParticleEmitterClass *)prender_obj)->Buffer_Scene_Not_Needed ();
	}

	// Remove the render object from the scene (if we have a valid scene)
	if (m_pCScene != NULL) {
		prender_obj->Remove ();
	}

	return ;
}

///////////////////////////////////////////////////////////////
//
//  Set_Texture_Path1
//
///////////////////////////////////////////////////////////////
void
W3DViewDoc::Set_Texture_Path1 (const char* path)
{		
	if (m_TexturePath1.compare(path, Qt::CaseInsensitive) != 0) {

		//
		//	Pass the new search path onto the File Factory
		//
		if (::lstrlen (path) > 0) {
			_TheSimpleFileFactory->Append_Sub_Directory(path);
		}

		m_TexturePath1 = path;
        QSettings settings(QSettings::Scope::UserScope);
        settings.setValue("TexturePath1", m_TexturePath1);
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//  Set_Texture_Path2
//
///////////////////////////////////////////////////////////////
void
W3DViewDoc::Set_Texture_Path2 (const char* path)
{	
	if (m_TexturePath2.compare(path, Qt::CaseInsensitive) != 0) {
		
		//
		//	Pass the new search path onto Surrender
		//
		if (::lstrlen (path) > 0) {
			_TheSimpleFileFactory->Append_Sub_Directory(path);
		}
		m_TexturePath2 = path;
        QSettings settings(QSettings::Scope::UserScope);
        settings.setValue("TexturePath2", m_TexturePath2);
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//  Load_Camera_Settings
//
///////////////////////////////////////////////////////////////
void
W3DViewDoc::Load_Camera_Settings (void)
{
    QSettings settings(QSettings::Scope::UserScope);
	m_ManualFOV				= settings.value("UseManualFOV", 0).toInt() == TRUE;
	m_ManualClipPlanes	= settings.value("UseManualClipPlanes", 0).toInt() == TRUE;

	GraphicView *graphic_view	= GetGraphicView ();
	if (graphic_view != NULL) {
		CameraClass *camera = graphic_view->GetCamera ();
		if (camera != NULL) {

			//
			// Should we load the FOV settings from the registry?
			//
			if (m_ManualFOV) {

                QString hfov_string = settings.value("hfov", "0").toString();
                QString vfov_string = settings.value("vfov", "0").toString();

				double hfov = hfov_string.toDouble();
				double vfov = vfov_string.toDouble();
				
				camera->Set_View_Plane (hfov, vfov);
			}

			//
			//	Should we load the clip planes from the registry?
			//
			if (m_ManualClipPlanes) {

                QString znear_string = settings.value("znear", "0.1F").toString();
                QString zfar_string = settings.value("zfar", "100.0F").toString();

                float znear = znear_string.toFloat();
                float zfar = zfar_string.toFloat();

				camera->Set_Clip_Planes (znear, zfar);
				
				if (m_pCScene != NULL) {
					m_pCScene->Set_Fog_Range (znear, zfar);
					m_pCScene->Recalculate_Fog_Planes();
				}
			}
		}
	}

	return ;
}