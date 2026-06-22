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
//  Utils.H
//
//  Module containing usefull misc. utility functions
//

#ifndef __UTILS_H
#define __UTILS_H

#include "vector.h"

#include <QString>

// Forward declarations
class RenderObjClass;


/////////////////////////////////////////////////////////////////////////////
//
// Macros
//
#define SAFE_DELETE(pobject)					\
			if (pobject) {							\
				delete pobject;					\
				pobject = NULL;					\
			}											\

#define SAFE_DELETE_ARRAY(pobject)			\
			if (pobject) {							\
				delete [] pobject;				\
				pobject = NULL;					\
			}											\

#define SAFE_ADD_REF(pobject)					\
			if (pobject) {							\
				pobject->Add_Ref ();				\
			}											\

#define SAFE_RELEASE_REF(pobject)			\
			if (pobject) {							\
				pobject->Release_Ref ();		\
			}											\
			
#define MEMBER_RELEASE(pmember)				\
			SAFE_RELEASE_REF(pmember);			\
			pmember = NULL;						\


#define MEMBER_ADD(pmember, pnew)			\
			MEMBER_RELEASE (pmember);			\
			pmember = pnew;						\
			SAFE_ADD_REF (pmember);				\


#define COM_RELEASE(pobject)					\
			if (pobject) {							\
				pobject->Release ();				\
			}											\
			pobject = NULL;						\

#define SAFE_CLOSE(handle)								\
			if (handle != INVALID_HANDLE_VALUE) {	\
				::CloseHandle (handle);					\
				handle = INVALID_HANDLE_VALUE;		\
			}													\

#define SANITY_CHECK(expr)		\
			ASSERT (expr);			\
			if (!expr)

/////////////////////////////////////////////////////////////////////////////
//
// Inlines
//
/////////////////////////////////////////////////////////////////////////////

inline void Delimit_Path (char* path)
{
	if (::lstrlen (path) > 0 && path[::lstrlen (path) - 1] != '\\') {
		::lstrcat (path, "\\");
	}
	return ;
}



// Forward declarations
class TextureClass;
class GraphicView;
class W3DViewWindow;

/////////////////////////////////////////////////////////////////////////////
//
// Prototypes
//
class W3DViewDoc *	GetCurrentDocument (void);
GraphicView *		Get_Graphic_View (void);
W3DViewWindow *		Get_Main_Window (void);

//
//	String manipulation routines
//
QString					Get_Filename_From_Path (const char* path);
QString					Strip_Filename_From_Path (const char* path);
QString					Asset_Name_From_Filename (const char* filename);
QString					Filename_From_Asset_Name (const char* asset_name);


//
//	Texture routines
//
void						Find_Missing_Textures (DynamicVectorClass<QString> &list, const char* filename, int frame_count = 1);


// Emitter routines
void						Build_Emitter_List (RenderObjClass &render_obj, DynamicVectorClass<QString> &list);

// Identification routines
bool						Is_Aggregate (const char *asset_name);
bool						Is_Real_LOD (const char *asset_name);

// Prototype routines
void						Rename_Aggregate_Prototype (const char *old_name, const char *new_name);

#endif //__UTILS_H
