/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 Electronic Arts Inc.
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

/////////////////////////////////////////////////////////////////////////////
//
//  Utils.CPP
//
//  Module containing usefull misc. utility functions
//


#include "utils.h"
#include "w3dviewdoc.h"
#include "w3dview.h"
#include "assetmgr.h"
#include "agg_def.h"
#include "hlod.h"

#include <QApplication>
#include <QFile>
#include <SDL3/SDL.h>

////////////////////////////////////////////////////////////////////////////
//
//  GetCurrentDocument
//
////////////////////////////////////////////////////////////////////////////
W3DViewDoc *
GetCurrentDocument (void)
{
    // Assume failure
    W3DViewDoc *pDoc = NULL;

    // Get a pointer to the application instance
	W3DViewApp *pApp = qobject_cast<W3DViewApp *>(QApplication::instance());
	WWASSERT(pApp);


	pDoc = pApp->GetCurrentDocument();
	WWASSERT(pDoc);

    // Return the doc pointer
    return pDoc;
}

////////////////////////////////////////////////////////////////////////////
//
//  Asset_Name_From_Filename
//
QString
Asset_Name_From_Filename (const char* filename)
{
	// Get the filename from this path
	QString asset_name = ::Get_Filename_From_Path (filename);
	
	// Find the index of the extension (if exists)
	int extension = asset_name.lastIndexOf ('.');	
	
	// Strip off the extension
	if (extension != -1) {
		asset_name = asset_name.left (extension);
	}

	// Return the name of the asset
	return asset_name;
}


////////////////////////////////////////////////////////////////////////////
//
//  Filename_From_Asset_Name
//
QString
Filename_From_Asset_Name (const char* asset_name)
{
	// The filename is simply the asset name plus the .w3d extension
	QString filename = QString(asset_name) + ".w3d";

	// Return the filename
	return filename;
}


////////////////////////////////////////////////////////////////////////////
//
//  Get_Filename_From_Path
//
QString
Get_Filename_From_Path (const char* path)
{
	// Find the last occurance of the directory deliminator
	const char* filename = ::strrchr (path, '\\');
	if (filename != NULL) {
		// Increment past the directory deliminator
		filename ++;
	} else {
		filename = path;
	}

	// Return the filename part of the path
	return QString(filename);
}


////////////////////////////////////////////////////////////////////////////
//
//  Strip_Filename_From_Path
//
QString
Strip_Filename_From_Path (const char* path)
{
	// Copy the path to a buffer we can modify
	char temp_path[MAX_PATH];
	::strcpy (temp_path, path);

	// Find the last occurance of the directory deliminator
	char* filename = ::strrchr (temp_path, '\\');
	if (filename != NULL) {
		// Strip off the filename
		filename[0] = 0;
	}

	// Return the path only
	return QString (temp_path);
}


////////////////////////////////////////////////////////////////////////////
//
//  Build_Emitter_List
//
void
Build_Emitter_List
(
	RenderObjClass &render_obj,
	DynamicVectorClass<QString> &list
)
{
	// Loop through all this render obj's sub-obj's
	for (int index = 0; index < render_obj.Get_Num_Sub_Objects (); index ++) {
		RenderObjClass *psub_obj = render_obj.Get_Sub_Object (index);
		if (psub_obj != NULL) {

			// Is this sub-obj an emitter?
			if (psub_obj->Class_ID () == RenderObjClass::CLASSID_PARTICLEEMITTER) {
				
				// Is this emitter already in the list?
				bool found = false;
				for (int list_index = 0; (list_index < list.Count ()) && !found; list_index++) {
					if (list[list_index].compare(psub_obj->Get_Name()) == 0) {
						found = true;
					}
				}

				// Add this emitter to the list if necessary
				if (!found) {
					list.Add (psub_obj->Get_Name ());
				}
			}

			// Recursivly add emitters to the list
			Build_Emitter_List (*psub_obj, list);
			MEMBER_RELEASE (psub_obj);
		}		
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Is_Aggregate
//
bool
Is_Aggregate (const char *asset_name)
{
	// Assume that the asset isn't an aggregate
	bool retval = false;

	// Check to see if this object is an aggregate
	RenderObjClass *prender_obj = WW3DAssetManager::Get_Instance()->Create_Render_Obj (asset_name);
	if ((prender_obj != NULL) &&
		 (prender_obj->Get_Base_Model_Name () != NULL))
	{
		retval = true;
	}

	// Free our hold on the temporary render object
	MEMBER_RELEASE (prender_obj);
	
	// Return the true/false result code
	return retval;
}


////////////////////////////////////////////////////////////////////////////
//
//  Rename_Aggregate_Prototype
//
void
Rename_Aggregate_Prototype
(
	const char *old_name,
	const char *new_name
)
{
	// Params valid?
	if ((old_name != NULL) &&
		 (new_name != NULL) &&
		 (::lstrcmpi (old_name, new_name) != 0)) {

		// Get the prototype from the asset manager
		AggregatePrototypeClass *proto = NULL;
		proto = (AggregatePrototypeClass *)WW3DAssetManager::Get_Instance ()->Find_Prototype (old_name);
		if (proto != NULL) {

			// Copy the definition from the prototype and remove the prototype
			AggregateDefClass *pdefinition = proto->Get_Definition ();
			AggregateDefClass *pnew_definition = pdefinition->Clone ();
			WW3DAssetManager::Get_Instance ()->Remove_Prototype (old_name);

			// Rename the definition, create a new prototype, and add it to the asset manager
			pnew_definition->Set_Name (new_name);
			proto = new AggregatePrototypeClass (pnew_definition);
			WW3DAssetManager::Get_Instance ()->Add_Prototype (proto);
		}		
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Is_Real_LOD
//
bool
Is_Real_LOD (const char *asset_name)
{
	// Assume that the asset isn't a true LOD (HLOD w/ more than one 
	bool retval = false;

	// Check to see if this object is an aggregate
	RenderObjClass *prender_obj = WW3DAssetManager::Get_Instance()->Create_Render_Obj (asset_name);
	if ((prender_obj != NULL) &&
		 (prender_obj->Class_ID () == RenderObjClass::CLASSID_HLOD) &&
		 (((HLodClass *)prender_obj)->Get_LOD_Count () > 1)) {
		retval = true;
	}

	// Free our hold on the temporary render object
	MEMBER_RELEASE (prender_obj);
	
	// Return the true/false result code
	return retval;
}

////////////////////////////////////////////////////////////////////////////
//
//  Resolve_Path
//
////////////////////////////////////////////////////////////////////////////
void
Resolve_Path (QString &filename)
{
	if (filename.indexOf ('\\') == -1) {
		char path[MAX_PATH];
		strncpy (path, SDL_GetCurrentDirectory(), MAX_PATH);
		::Delimit_Path (path);
		filename = QString(path) + filename;
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Find_Missing_Textures
//
////////////////////////////////////////////////////////////////////////////
void
Find_Missing_Textures
(
	DynamicVectorClass<QString> &	list,
	LPCTSTR								name,
	int									frame_count
)
{
	//
	//	If this file doesn't exist, then add it to our list
	//
	if (!QFile::exists(name)) {
		QString full_path = name;
		Resolve_Path (full_path);
		list.Add (full_path);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Get_Graphic_View
//
////////////////////////////////////////////////////////////////////////////
GraphicView *
Get_Graphic_View (void)
{
	GraphicView *view = NULL;

	//
	//	Get the view from the current document
	//
	W3DViewDoc *doc = GetCurrentDocument ();
	if (doc != NULL) {
		view = doc->GetGraphicView ();
	}

	return view;
}
