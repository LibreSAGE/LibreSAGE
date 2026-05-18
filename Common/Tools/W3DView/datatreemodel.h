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
#pragma once

#include "assettypes.h"
#include "vector.h"

#include <QStandardItemModel>
#include <QIcon>

// Forward declarations
class RenderObjClass;
class AssetInfoClass;

/////////////////////////////////////////////////////////////////////////////
//
// DataTreeModel view
//
class DataTreeModel final : public QStandardItemModel
{
public:
	DataTreeModel();           // protected constructor used by dynamic creation
	virtual ~DataTreeModel();

	public:

	/////////////////////////////////////////////////////////////////////
	//	Public methods
	/////////////////////////////////////////////////////////////////////
	
	//
	//	Asset insertion methods
	//
	bool					Add_Asset_To_Tree (const char* name, ASSET_TYPE type);
	void					LoadAssetsIntoTree (void);
	void					Refresh_Asset (const char* new_name, const char* old_name, ASSET_TYPE type);

	//
	//	Animation insertion methods
	//
	void					LoadAnimationsIntoTree (void);
	void					LoadAnimationsIntoTree (QStandardItem* hItem);

	bool					Are_Anims_Restricted (void) const			{ return m_RestrictAnims; }
	void					Restrict_Anims (bool onoff);

	//
	//	Texture insertion methods
	//
	void					Load_Materials_Into_Tree (void);

	//
	//	Display methods
	//
	void					Display_Asset (QStandardItem* htree_item = NULL);
	void					Select_Next (void);
	void					Select_Prev (void);
	void					Reload_Lightmap_Models (void);

	//
	// Information methods
	//
	RenderObjClass *	Get_Current_Render_Obj (void) const;
	AssetInfoClass *	Get_Current_Asset_Info (void) const;		
	const char*			GetCurrentSelectionName (void);
	ASSET_TYPE			GetCurrentSelectionType (void);
	QStandardItem*		FindChildItem (QStandardItem* hParentItem, const char* pszChildItemName);
	QStandardItem*		FindChildItem (QStandardItem* hParentItem, RenderObjClass *prender_obj);
	QStandardItem*		FindFirstChildItemBasedOnHierarchyName (QStandardItem* hParentItem, const char* pszHierarchyName);
	QStandardItem*		FindSiblingItemBasedOnHierarchyName (QStandardItem* hCurrentItem, const char* pszHierarchyName);
	void				Build_Render_Object_List (DynamicVectorClass <QString> &asset_list, QStandardItem* hparent = nullptr);
	
	//
	//	Initialization methods
	//
	void					CreateRootNodes (void);

	protected:
		
		///////////////////////////////////////////////////////////////////////
		//	Protected methods
		///////////////////////////////////////////////////////////////////////
		ASSET_TYPE			Determine_Tree_Location (RenderObjClass &render_obj, QStandardItem* &hroot, QIcon &icon);
		void					Determine_Tree_Location (ASSET_TYPE type, QStandardItem* &hroot, QIcon &icon);
		RenderObjClass *	Create_Render_Obj_To_Display (QStandardItem* htree_item);
		// void					Add_Emitters_To_Menu (HMENU hmenu, RenderObjClass &render_obj);		
		void					Free_Child_Models (QStandardItem* parent_item);

	private:
		
		///////////////////////////////////////////////////////
		//
		//	Private member data
		//
		QStandardItem*	m_hMaterialsRoot;
		QStandardItem*	m_hMeshRoot;
		QStandardItem*	m_hAggregateRoot;
		QStandardItem*	m_hLODRoot;
		QStandardItem*	m_hMeshCollectionRoot;
		QStandardItem*	m_hEmitterRoot;
		QStandardItem*	m_hPrimitivesRoot;
		QStandardItem*	m_hHierarchyRoot;
		QStandardItem*	m_hSoundRoot;
		QIcon			m_iAnimationIcon;
		QIcon			m_iTCAnimationIcon;
		QIcon			m_iADAnimationIcon;
		QIcon			m_iMeshIcon;
		QIcon			m_iMaterialIcon;
		QIcon			m_iLODIcon;
		QIcon			m_iEmitterIcon;
		QIcon			m_iPrimitivesIcon;
		QIcon			m_iAggregateIcon;
		QIcon			m_iHierarchyIcon;
		QIcon			m_iSoundIcon;
		bool		m_RestrictAnims;
};
