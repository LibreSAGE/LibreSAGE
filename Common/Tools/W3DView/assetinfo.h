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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : W3DView                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/W3DView/AssetInfo.h                    $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/09/99 2:50p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif


#ifndef __ASSET_INFO_H
#define __ASSET_INFO_H

#include "rendobj.h"
#include "utils.h"
#include "assettypes.h"

#include <QString>

/////////////////////////////////////////////////////////////////////////////
//
//  AssetInfoClass
//
//		Class used by the data tree to identify each individual
//		entry in the tree.
//
class AssetInfoClass
{
	public:

		//////////////////////////////////////////////////////////////
		//
		//  Public constructors/destructors
		//
		AssetInfoClass (void)
			: m_AssetType (TypeUnknown),
			  m_ptrUserData (0L),
			  m_pRenderObj (NULL)			{ Initialize (); }

		AssetInfoClass (const char* passet_name, ASSET_TYPE type, RenderObjClass *prender_obj = NULL, uintptr_t user_data = 0L)
			: m_Name (passet_name),
			  m_AssetType (type),
			  m_ptrUserData (user_data),
			  m_pRenderObj (NULL)			{ MEMBER_ADD (m_pRenderObj, prender_obj); Initialize (); }
		
		virtual ~AssetInfoClass (void)	{ MEMBER_RELEASE (m_pRenderObj); }

		//////////////////////////////////////////////////////////////
		//
		//  Public methods
		//

		//
		//  Inline accessors
		//
		const QString &	Get_Name (void) const						{ return m_Name; }
		const QString &	Get_Hierarchy_Name (void) const			{ return m_HierarchyName; }
		const QString &	Get_Original_Name (void) const			{ return m_OriginalName; }
		ASSET_TYPE			Get_Type (void) const						{ return m_AssetType; }
		uintptr_t			Get_User_Number (void) const				{ return m_ptrUserData; }
		const QString &	Get_User_String (void) const				{ return m_UserString; }
		RenderObjClass *	Get_Render_Obj (void) const				{ SAFE_ADD_REF (m_pRenderObj); return m_pRenderObj; }
		RenderObjClass *	Peek_Render_Obj (void) const				{ return m_pRenderObj; }
		void					Set_Name (const char* pname)					{ m_Name = pname; }
		void					Set_Hierarchy_Name (const char* pname)		{ m_HierarchyName = pname; }
		void					Set_Type (ASSET_TYPE type)					{ m_AssetType = type; }
		void					Set_User_Number (uintptr_t user_data)		{ m_ptrUserData = user_data; }
		void					Set_User_String (const char* string)			{ m_UserString = string; }
		void					Set_Render_Obj (RenderObjClass *pobj)	{ MEMBER_ADD (m_pRenderObj, pobj); }

		//
		//	Information methods
		//
		bool					Can_Asset_Have_Animations (void) const	{ return bool(m_HierarchyName.length () > 0); }

	protected:
		
		//////////////////////////////////////////////////////////////
		//
		//  Protected methods
		//
		void					Initialize (void);
		
		
	private:

		//////////////////////////////////////////////////////////////
		//
		//  Private member data
		//
		QString				m_Name;
		QString				m_HierarchyName;
		QString				m_UserString;
		QString				m_OriginalName;
		ASSET_TYPE			m_AssetType;
		uintptr_t			m_ptrUserData;
		RenderObjClass *	m_pRenderObj;
};



#endif //__ASSET_INFO_H