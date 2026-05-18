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
 *                     $Archive:: /Commando/Code/Tools/W3DView/DataTreeView.cpp               $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 6/18/01 9:11a                                               $*
 *                                                                                             *
 *                    $Revision:: 35                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "datatreemodel.h"
#include "w3dview.h"
#include "rendobj.h"
#include "viewerassetmgr.h"
#include "globals.h"
#include "w3dviewdoc.h"
#include "distlod.h"
#include "animobj.h"
#include "hcanim.h"
#include "assetinfo.h"
#include "utils.h"
#include "vector.h"
#include "part_emt.h"
#include "agg_def.h"
#include "bmp2d.h"
#include "hlod.h"
#include "viewerscene.h"
#include "texture.h"

////////////////////////////////////////////////////////////////////////////
//	Local Prototypes
////////////////////////////////////////////////////////////////////////////
void Set_Highest_LOD (RenderObjClass *render_obj);



////////////////////////////////////////////////////////////////////////////
//
//  DataTreeModel
//
////////////////////////////////////////////////////////////////////////////
DataTreeModel::DataTreeModel (void)
        : m_hMaterialsRoot (NULL),          
			 m_hMeshRoot  (NULL),
			 m_hMeshCollectionRoot (NULL),
			 m_hAggregateRoot (NULL),
			 m_hPrimitivesRoot (NULL),
			 m_hEmitterRoot (NULL),
          m_hLODRoot (NULL),
			 m_hSoundRoot (NULL),
			 m_RestrictAnims (true)

{
	m_iAnimationIcon		= QIcon(":/icons/anim.ico");
	m_iTCAnimationIcon	= QIcon(":/icons/anim_compressed.ico");
	m_iADAnimationIcon  = QIcon(":/icons/anim_adaptive.ico");
    m_iMeshIcon			= QIcon(":/icons/mesh.ico");
    m_iMaterialIcon		= QIcon(":/icons/material.ico");
    m_iLODIcon				= QIcon(":/icons/lod.ico");
	m_iAggregateIcon		= QIcon(":/icons/aggregate.ico");
	m_iEmitterIcon		= QIcon(":/icons/emitter.ico");
	m_iHierarchyIcon		= QIcon(":/icons/hierarchy.ico");
	m_iPrimitivesIcon	= QIcon(":/icons/primitives.ico");
	m_iSoundIcon			= QIcon(":/icons/sound.ico");

    // Create the root nodes that will hold the contain
    // asset types.
    CreateRootNodes ();
    return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  ~DataTreeModel
//
DataTreeModel::~DataTreeModel (void)
{
    return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  CreateRootNodes
//
void
DataTreeModel::CreateRootNodes (void) 
{
	// Insert all the root nodes
	QStandardItem* rootNode = invisibleRootItem();
	m_hMaterialsRoot		= new QStandardItem(m_iMaterialIcon, "Materials");
	rootNode->appendRow(m_hMaterialsRoot);
	m_hMeshRoot				= new QStandardItem(m_iMeshIcon, "Mesh");
	rootNode->appendRow(m_hMeshRoot);
	m_hHierarchyRoot		= new QStandardItem(m_iHierarchyIcon, "Hierarchy");
	rootNode->appendRow(m_hHierarchyRoot);
	m_hLODRoot				= new QStandardItem(m_iLODIcon, "H-LOD");
	rootNode->appendRow(m_hLODRoot);
	m_hMeshCollectionRoot = new QStandardItem(m_iMeshIcon, "Mesh Collection");
	rootNode->appendRow(m_hMeshCollectionRoot);
	m_hAggregateRoot		= new QStandardItem(m_iAggregateIcon, "Aggregate");
	rootNode->appendRow(m_hAggregateRoot);
	m_hEmitterRoot			= new QStandardItem(m_iEmitterIcon, "Emitter");
	rootNode->appendRow(m_hEmitterRoot);
	m_hPrimitivesRoot		= new QStandardItem(m_iPrimitivesIcon, "Primitives");
	rootNode->appendRow(m_hPrimitivesRoot);
	m_hSoundRoot			= new QStandardItem(m_iSoundIcon, "Sounds");
	rootNode->appendRow(m_hSoundRoot);
	return ;
}

////////////////////////////////////////////////////////////////////////////
//
//  Load_Materials_Into_Tree
//
void
DataTreeModel::Load_Materials_Into_Tree (void) 
{
	// Get an iterator from the asset manager that we can
	// use to enumerate the currently loaded textures
	HashTemplateIterator<StringClass,TextureClass*> ite(WW3DAssetManager::Get_Instance()->Texture_Hash());
		
	// Loop through all the textures in the manager
	for (ite.First ();
		  !ite.Is_Done ();
		  ite.Next ()) {

		// Get the current texture name
		TextureClass* ptexture=ite.Peek_Value();
		const char* texture_name = ptexture->Get_Texture_Name();

		if ((ptexture != NULL) &&
			 FindChildItem (m_hMaterialsRoot, texture_name) == NULL) {

			// Add this entry to the tree
			QStandardItem* tree_item = new QStandardItem(m_iMaterialIcon, texture_name);
			 m_hMaterialsRoot->appendRow(tree_item);
			WWASSERT (tree_item != NULL);

			// Allocate a new asset information class to associate with this entry
			ptexture->Add_Ref ();
			AssetInfoClass *asset_info = new AssetInfoClass (texture_name, TypeMaterial, NULL, (uintptr_t)ptexture);
			tree_item->setData((qulonglong)asset_info);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  LoadAssetsIntoTree
//
void
DataTreeModel::LoadAssetsIntoTree (void) 
{
	// Turn off repainting
	// GetTreeCtrl ().SetRedraw (FALSE);

	DynamicVectorClass<QString> dist_lod_list;

	// Get an iterator from the asset manager that we can
	// use to enumerate the currently loaded assets
	RenderObjIterator *pObjEnum = WW3DAssetManager::Get_Instance()->Create_Render_Obj_Iterator ();
	WWASSERT (pObjEnum);
	if (pObjEnum) {

		// Loop through all the assets in the manager
		for (pObjEnum->First ();
			  pObjEnum->Is_Done () == FALSE;
			  pObjEnum->Next ()) {

			// Does this render obj really exist?
			const char* pszItemName = pObjEnum->Current_Item_Name ();
			if (WW3DAssetManager::Get_Instance()->Render_Obj_Exists (pszItemName)) {
				
				BOOL bInsert = FALSE;
				QStandardItem* hParentNode = NULL;
				ASSET_TYPE assetType = TypeUnknown;
				QIcon icon;

				// What type of asset is this?
				switch (pObjEnum->Current_Item_Class_ID ()) {

					case RenderObjClass::CLASSID_COLLECTION:						
						// This is a 'mesh collection', we want to add this under the 'collection' node.
						bInsert = TRUE;
						hParentNode = m_hMeshCollectionRoot;
						assetType = TypeMesh;
						icon = m_iMeshIcon;
						break;

					//
					// This is a mesh render object, we want to add this under the mesh node.
					//
					case RenderObjClass::CLASSID_MESH:						
						bInsert			= TRUE;
						hParentNode		= m_hMeshRoot;
						assetType		= TypeMesh;
						icon		= m_iMeshIcon;
						break;

					//
					// This is a sound render obj, we want to add this under the sound node.
					//					
					case RenderObjClass::CLASSID_SOUND:
						bInsert			= TRUE;
						hParentNode		= m_hSoundRoot;
						assetType		= TypeSound;
						icon		= m_iSoundIcon;						
						break;

					case RenderObjClass::CLASSID_HMODEL:
						// Shouldn't happen
						WWASSERT (0);
						break;

					case RenderObjClass::CLASSID_PARTICLEEMITTER:							
						// This is an 'emitter', we want to add this under the 'emitter' node.
						bInsert = TRUE;
						hParentNode = m_hEmitterRoot;
						assetType = TypeEmitter;
						icon = m_iEmitterIcon;
						break;

					case RenderObjClass::CLASSID_SPHERE:
					case RenderObjClass::CLASSID_RING:
						bInsert		= TRUE;
						hParentNode = m_hPrimitivesRoot;
						assetType	= TypePrimitives;
						icon	= m_iPrimitivesIcon;
						break;

					case RenderObjClass::CLASSID_DISTLOD:
						dist_lod_list.Add (pszItemName);
					case RenderObjClass::CLASSID_HLOD:
						// Assume this is a simple hierarchy LOD, we want to add this under the hierarchy node.
						bInsert = TRUE;
						hParentNode = m_hHierarchyRoot;
						assetType = TypeHierarchy;
						icon = m_iHierarchyIcon;

						// Test this HLOD to see if its a true LOD or a simple hierarchy
						if (::Is_Real_LOD (pszItemName)) {
							hParentNode = m_hLODRoot;
							assetType = TypeLOD;
							icon = m_iLODIcon;
						}
						break;
				}

				if (bInsert) {

					// Check to see if this object is an aggregate
					if (::Is_Aggregate (pszItemName)) {
						hParentNode = m_hAggregateRoot;
						assetType = TypeAggregate;
						icon = m_iAggregateIcon;
					}

					// If this object isn't already in the tree then add it
					if (FindChildItem (hParentNode, pszItemName) == NULL) {

						// Add this entry to the tree
						QStandardItem* hItem = new QStandardItem(icon, QString(pszItemName));
						hParentNode->appendRow(hItem);
						WWASSERT (hItem != NULL);

						// Allocate a new asset information class to associate with this entry
						AssetInfoClass *asset_info = new AssetInfoClass (pszItemName, assetType);
						hItem->setData((quint64)asset_info);
					}
				}
			}
		}

		// Free the enumerator object we created earlier
		SAFE_DELETE (pObjEnum);
	}

	// Loop through all the old-style dist lod's and convert their prototypes
	// to the new HLOD format
	for (int index = 0; index < dist_lod_list.Count (); index ++) {
		HLodClass *plod = (HLodClass *)WW3DAssetManager::Get_Instance ()->Create_Render_Obj (dist_lod_list[index].toUtf8().constData ());
		if (plod != NULL) {
			HLodDefClass *definition = new HLodDefClass (*plod);
			HLodPrototypeClass *prototype = new HLodPrototypeClass (definition);
			WW3DAssetManager::Get_Instance ()->Remove_Prototype (dist_lod_list[index].toUtf8().constData ());
			WW3DAssetManager::Get_Instance ()->Add_Prototype (prototype);
		}
	}

	// Now that we've added all the hierarchies to the tree, add their animations
	// as well.
	LoadAnimationsIntoTree ();
	Load_Materials_Into_Tree ();

	// Turn;repainting back on
	// GetTreeCtrl ().SetRedraw (TRUE);

	// Force the window to be repainted
	// Invalidate (FALSE);
	// UpdateWindow ();
	return ;
}

////////////////////////////////////////////////////////////////////////////
//
//  LoadAnimationsIntoTree
//
void
DataTreeModel::LoadAnimationsIntoTree (void)
{
    // Get an iterator from the asset manager that we can
    // use to enumerate the currently loaded assets
    AssetIterator *pAnimEnum = WW3DAssetManager::Get_Instance()->Create_HAnim_Iterator ();
    WWASSERT (pAnimEnum);
    if (pAnimEnum)
    {
        // Loop through all the animations in the manager
        for (pAnimEnum->First ();
             (pAnimEnum->Is_Done () == FALSE);
             pAnimEnum->Next ())
        {
            const char* pszAnimName = pAnimEnum->Current_Item_Name ();

            // Get an instance of the animation object
            HAnimClass *pHierarchyAnim = WW3DAssetManager::Get_Instance()->Get_HAnim (pszAnimName);
            
            WWASSERT (pHierarchyAnim);
            if (pHierarchyAnim)
            {
                // Get the name of the hierarchy that this animation belongs to
                const char* pszHierarchyName = pHierarchyAnim->Get_HName ();

                // Loop through all the hierarchies and add this animation to any pertinent ones
                for (QStandardItem* hNode = FindFirstChildItemBasedOnHierarchyName (m_hHierarchyRoot, pszHierarchyName);
                     (hNode != NULL);
                     hNode = FindSiblingItemBasedOnHierarchyName (hNode, pszHierarchyName))
                {
                    // Is this animation already loaded into the tree?
                    QStandardItem* hAnimationNode = FindChildItem (hNode, pszAnimName);
                    if (hAnimationNode == NULL)
                    {
                        // Add this animation as a child of the hierarchy
                        hAnimationNode = new QStandardItem(m_iAnimationIcon, pszAnimName);
                        hNode->appendRow(hAnimationNode);
                        WWASSERT (hAnimationNode != NULL);
                                    
                        // Associate the items name with its entry
                        hAnimationNode->setData((quint64)new AssetInfoClass (pszAnimName, TypeAnimation));
                    }
                }

                // Loop through all the aggregates and add this animation to any pertinent ones
                for (QStandardItem* hNode = FindFirstChildItemBasedOnHierarchyName (m_hAggregateRoot, pszHierarchyName);
                     (hNode != NULL);
                     hNode = FindSiblingItemBasedOnHierarchyName (hNode, pszHierarchyName))
                {
                    // Is this animation already loaded into the tree?
                    QStandardItem* hAnimationNode = FindChildItem (hNode, pszAnimName);
                    if (hAnimationNode == NULL)
                    {
                        // Add this animation as a child of the hierarchy
                        hAnimationNode = new QStandardItem(m_iAnimationIcon, pszAnimName);
                        hNode->appendRow(hAnimationNode);
                        WWASSERT (hAnimationNode != NULL);
                                    
                        // Associate the items name with its entry
                        hAnimationNode->setData((quint64)new AssetInfoClass (pszAnimName, TypeAnimation));
                    }
                }

                // Loop through all the hierarchies and add this animation to any pertinent ones
                for (QStandardItem* hNode = FindFirstChildItemBasedOnHierarchyName (m_hLODRoot, pszHierarchyName);
                     (hNode != NULL);
                     hNode = FindSiblingItemBasedOnHierarchyName (hNode, pszHierarchyName))
                {
                    // Is this animation already loaded into the tree?
                    QStandardItem* hAnimationNode = FindChildItem (hNode, pszAnimName);
                    if (hAnimationNode == NULL)
                    {
                        // Add this animation as a child of the hierarchy
                        hAnimationNode = new QStandardItem(m_iAnimationIcon, pszAnimName);
                        hNode->appendRow(hAnimationNode);
                        WWASSERT (hAnimationNode != NULL);
                                    
                        // Associate the items name with its entry
                        hAnimationNode->setData((quint64)new AssetInfoClass (pszAnimName, TypeAnimation));
                    }
                }

                // Release our hold on this animation...
				MEMBER_RELEASE (pHierarchyAnim);
            }							
        }
        
        // Free the object
        delete pAnimEnum;
        pAnimEnum = NULL;
    }
    
    return ;
}

////////////////////////////////////////////////////////////////////////////
//
//  LoadAnimationsIntoTree
//
void
DataTreeModel::LoadAnimationsIntoTree (QStandardItem* hItem)
{
    // Get the data associated with this item
    AssetInfoClass *asset_info = (AssetInfoClass *)hItem->data().toULongLong();
    WWASSERT (asset_info != NULL);

    // Get an iterator from the asset manager that we can
    // use to enumerate the currently loaded assets
    AssetIterator *pAnimEnum = WW3DAssetManager::Get_Instance()->Create_HAnim_Iterator ();
    WWASSERT (pAnimEnum);
    if (pAnimEnum)
    {
        // Loop through all the animations in the manager
        for (pAnimEnum->First ();
             (pAnimEnum->Is_Done () == FALSE);
             pAnimEnum->Next ())
        {
            const char* pszAnimName = pAnimEnum->Current_Item_Name ();

            // Get an instance of the animation object
            HAnimClass *pHierarchyAnim = WW3DAssetManager::Get_Instance()->Get_HAnim (pszAnimName);
            
            WWASSERT (pHierarchyAnim);
            if (pHierarchyAnim)
            {
                // Get the name of the hierarchy that this animation belongs to
                const char* pszHierarchyName = pHierarchyAnim->Get_HName ();

                // Does the item match the hierarchy name?
                if (::strcmp (asset_info->Get_Hierarchy_Name ().toUtf8().constData (), pszHierarchyName) == 0)
                {
                    // Is this animation already loaded into the tree?
                    QStandardItem* hAnimationNode = FindChildItem (hItem, pszAnimName);
                    if (hAnimationNode == NULL)
                    {
                        // Add this animation as a child of the hierarchy
                        hAnimationNode = new QStandardItem(m_iAnimationIcon, pszAnimName);
                        WWASSERT (hAnimationNode != NULL);
						hItem->appendRow(hAnimationNode);

                        // Associate the items name with its entry
						hAnimationNode->setData((quint64)new AssetInfoClass (pszAnimName, TypeAnimation));
                    }
                }

                // Release our hold on the animation object
                MEMBER_RELEASE (pHierarchyAnim);
            }				
        }
        
        // Free the object
        delete pAnimEnum;
        pAnimEnum = NULL;
    }
    
    return ;
}
   

////////////////////////////////////////////////////////////////////////////
//
//  Determine_Tree_Location
//
ASSET_TYPE
DataTreeModel::Determine_Tree_Location
(
	RenderObjClass &render_obj,
	QStandardItem* &hroot,
	QIcon &icon
)
{
	ASSET_TYPE type = TypeUnknown;

	// What class does this render object belong to?
	switch (render_obj.Class_ID ()) {
		
		case RenderObjClass::CLASSID_COLLECTION:
			hroot				= m_hMeshCollectionRoot;
			type				= TypeMesh;
			icon		= m_iMeshIcon;
			break;

		case RenderObjClass::CLASSID_MESH:
			hroot = m_hMeshRoot;
			type = TypeMesh;
			icon		= m_iMeshIcon;
			break;						  
		
		case RenderObjClass::CLASSID_SOUND:
			hroot				= m_hSoundRoot;
			type				= TypeSound;
			icon		= m_iSoundIcon;
			break;						  		

		case RenderObjClass::CLASSID_HMODEL:			
			// Shouldn't happen
			WWASSERT (0);
			break;

		case RenderObjClass::CLASSID_PARTICLEEMITTER:
			hroot				= m_hEmitterRoot;
			type				= TypeEmitter;
			icon		= m_iEmitterIcon;
			break;

		case RenderObjClass::CLASSID_SPHERE:
		case RenderObjClass::CLASSID_RING:
			hroot				= m_hPrimitivesRoot;
			type				= TypePrimitives;
			icon		= m_iPrimitivesIcon;
			break;
	
		case RenderObjClass::CLASSID_DISTLOD:
		case RenderObjClass::CLASSID_HLOD:
         hroot				= m_hHierarchyRoot;
         type				= TypeHierarchy;
         icon		= m_iHierarchyIcon;

			//
			// Determine if this is a true LOD or a simple hierarchy
			//
			if (((HLodClass &)render_obj).Get_LOD_Count () > 1) {
				hroot			= m_hLODRoot;
				type			= TypeLOD;
				icon		= m_iLODIcon;
			}
			break;
	}

	//
	// Is this an aggregate?
	//
	if (render_obj.Get_Base_Model_Name () != NULL) {
		hroot			= m_hAggregateRoot;
		type			= TypeAggregate;
		icon		= m_iAggregateIcon;
	}

	// Return the type of node
	return type;
}



////////////////////////////////////////////////////////////////////////////
//
//  Determine_Tree_Location
//
void
DataTreeModel::Determine_Tree_Location
(
	ASSET_TYPE type,
	QStandardItem* &hroot,
	QIcon &icon
)
{
	// What type of asset is this?
	switch (type) {

		case TypeMesh:
			hroot			= m_hMeshRoot;
			icon		= m_iMeshIcon;
			break;

		case TypeSound:
			hroot			= m_hSoundRoot;
			icon		= m_iSoundIcon;
			break;

		case TypeAggregate:
			hroot			= m_hAggregateRoot;
			icon		= m_iAggregateIcon;
			break;

		case TypeHierarchy:
			// Shouldn't happen
			WWASSERT (0);
			break;

		case TypeEmitter:
			hroot			= m_hEmitterRoot;
			icon		= m_iEmitterIcon;
			break;

		case TypePrimitives:
			hroot			= m_hPrimitivesRoot;
			icon		= m_iPrimitivesIcon;
			break;

		case TypeLOD:
			hroot			= m_hLODRoot;
			icon		= m_iLODIcon;
			break;
	}

	return ;
}



////////////////////////////////////////////////////////////////////////////
//
//  Add_Asset_To_Tree
//
bool
DataTreeModel::Add_Asset_To_Tree
(
	const char* name,
	ASSET_TYPE type
)
{
	// Assume failure
	bool retval = false;

	// Param OK?
	WWASSERT (name != NULL);
	if (name != NULL) {

		// Turn off repainting
		// GetTreeCtrl ().SetRedraw (FALSE);
		
		// Determime where this asset should go
		QStandardItem* hparent = NULL;
		QIcon icon;
		Determine_Tree_Location (type, hparent, icon);

		// Is this asset already in the tree?
		QStandardItem* htree_item = FindChildItem (hparent, name);
		if (htree_item == NULL) {

			// Add this object to the tree
			htree_item = new QStandardItem(icon, name);
			hparent->appendRow(htree_item);
			
			// Associate the render object with its entry in the tree
			AssetInfoClass *asset_info = new AssetInfoClass (name, type);
			htree_item->setData((quint64)asset_info);

			// Load the object's animations into the tree (if necessary)
			if (asset_info->Can_Asset_Have_Animations ()) {
				LoadAnimationsIntoTree (htree_item);
			}

			// Success!
			retval = (htree_item != NULL);
		}

		// Select the instance (if requested)
		// if (bselect) {
		// 	GetTreeCtrl ().SelectItem (htree_item);
		// 	GetTreeCtrl ().EnsureVisible (htree_item);
		// }

		// Turn painting back on
		// GetTreeCtrl ().SetRedraw (TRUE);

		// // Force the window to be repainted
		// Invalidate (FALSE);
		// UpdateWindow ();
	}

	// Return the true/false result code
	return retval;
}


////////////////////////////////////////////////////////////////////////////
//
//  FindChildItem
//
QStandardItem*
DataTreeModel::FindChildItem
(
	QStandardItem* hParentItem,
	RenderObjClass *prender_obj
)
{
	// Assume we won't find the item
	QStandardItem* hchild_item = NULL;

	// Loop through all the children of this node
	for(int i = 0; i < hParentItem->rowCount(); i++) {
		QStandardItem* htree_item = hParentItem->child(i);

		// Get the data associated with this item
		AssetInfoClass *asset_info = (AssetInfoClass *)htree_item->data().toULongLong();

		// Is this the item we were looking for?
		if (asset_info &&
			 asset_info->Peek_Render_Obj () == prender_obj) {

			// This was the item we were looking for, return
			// its handle to the caller
			hchild_item = htree_item;
		}
	}

	// Return the child item handle
	return hchild_item;
}


////////////////////////////////////////////////////////////////////////////
//
//  FindChildItem
//
QStandardItem*
DataTreeModel::FindChildItem
(
    QStandardItem* hParentItem,
    const char* pszChildItemName
)
{
    // Assume we won't find the item
    QStandardItem* hChildItem = NULL;

	// Loop through all the children of this node
	for(int i = 0; i < hParentItem->rowCount(); i++)
	{
		QStandardItem* htree_item = hParentItem->child(i);
		if (htree_item != NULL) {

			// Is this the item we were looking for?
			if (::strcmp (htree_item->text().toUtf8().constData(), pszChildItemName) == 0) {

				// This was the item we were looking for, return
				// its handle to the caller
				hChildItem = htree_item;
				break;
			}
		}
	}

    // Return the child item handle
    return hChildItem;
}

////////////////////////////////////////////////////////////////////////////
//
//  FindSiblingItemBasedOnHierarchyName
//
QStandardItem*
DataTreeModel::FindSiblingItemBasedOnHierarchyName
(
    QStandardItem* hCurrentItem,
    const char* pszHierarchyName
)
{
    // Assume we won't find the item
    QStandardItem* hSiblingItem = NULL;

    // Loop through all the siblings of this node
    QStandardItem* hParentItem = hCurrentItem->parent();
	WWASSERT (hParentItem != NULL);

	// Iterate through all children of the parent item, looking for one with a matching hierarchy name
	for(int i = 0; i < hParentItem->rowCount(); i++)
	{
		QStandardItem* hTreeItem = hParentItem->child(i);
		if (hTreeItem == hCurrentItem) {
			continue;
		}

		// Get the data associated with this item
		AssetInfoClass *asset_info = (AssetInfoClass *)hTreeItem->data().toULongLong();

		// Is this the item we were looking for?
        if (m_RestrictAnims == false ||
				(asset_info && ::strcmp (asset_info->Get_Hierarchy_Name ().toUtf8().constData(), pszHierarchyName) == 0))
        {
            // This was the item we were looking for, return
            // its handle to the caller
            hSiblingItem = hTreeItem;
            break;
        }   
	}

    // Return the sibling item handle
    return hSiblingItem;
}

////////////////////////////////////////////////////////////////////////////
//
//  FindFirstChildItemBasedOnHierarchyName
//
QStandardItem*
DataTreeModel::FindFirstChildItemBasedOnHierarchyName(QStandardItem* hParentItem, const char* pszHierarchyName) {
    // Assume we won't find the item
    QStandardItem* hChildItem = NULL;

    // Loop through all the children of this node
    for(int i = 0; i < hParentItem->rowCount(); i++) {
        QStandardItem* htree_item = hParentItem->child(i);
        WWASSERT(htree_item != NULL);

		// Get the data associated with this item
		AssetInfoClass *asset_info = (AssetInfoClass *)htree_item->data().toULongLong();

        //
		  // Is this the item we were looking for?
		  //
        if (m_RestrictAnims == false ||
				(asset_info && ::strcmp (asset_info->Get_Hierarchy_Name ().toUtf8().constData(), pszHierarchyName) == 0))
        {
            // This was the child item we were looking for, return
            // its handle to the caller
            hChildItem = htree_item;
        }        
    }

    // Return the child item handle
    return hChildItem;
}


////////////////////////////////////////////////////////////////////////////
//
//  Display_Asset
//
void
DataTreeModel::Display_Asset (QStandardItem* htree_item)
{
	WWASSERT (htree_item != NULL);

	//
	// Get the object associated with this entry
	//
	AssetInfoClass *asset_info = NULL;	
	if (htree_item != NULL) {
		asset_info = (AssetInfoClass *)htree_item->data().toULongLong();
	}

	if (asset_info != NULL) {

		// Get the current document, so we can get a pointer to the scene
		W3DViewDoc *pdoc = GetCurrentDocument ();
		WWASSERT (pdoc != NULL);
		if (pdoc != NULL) {

			// What type of asset is it?
			switch (asset_info->Get_Type ())
			{
				case TypeCompressedAnimation:
				case TypeAnimation:
				{
					QStandardItem* hParentItem = htree_item->parent();
					if (hParentItem != NULL) {

						// Ask the document to start playing the animation for this object
						RenderObjClass *prender_obj = Create_Render_Obj_To_Display (hParentItem);                         
						// pdoc->PlayAnimation (prender_obj,asset_info->Get_Name ().toUtf8().constData());
						MEMBER_RELEASE (prender_obj);
					}
				}
				break;

				case TypeEmitter:
				{
					// Ask the document to display this object
					ParticleEmitterClass *emitter = (ParticleEmitterClass *)Create_Render_Obj_To_Display (htree_item);
					// pdoc->Display_Emitter (emitter);
					MEMBER_RELEASE (emitter);
				}
				break;

				case TypeHierarchy:
				{
					// If the advanced animation option is turned on, display a dialog
					// where the user can choose which animations to play together.

				}

				default:
				{
					// Ask the document to display this object
					RenderObjClass *prender_obj = Create_Render_Obj_To_Display (htree_item);
					pdoc->DisplayObject (prender_obj);
					MEMBER_RELEASE (prender_obj);
				}
				break;
			}
		}
	} else {

		// Reset the display
		W3DViewDoc* pdoc = GetCurrentDocument ();
		WWASSERT (pdoc != NULL);
		if (pdoc != NULL) {
			pdoc->DisplayObject ((RenderObjClass *)NULL);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Build_Render_Object_List
//
void
DataTreeModel::Build_Render_Object_List
(
	DynamicVectorClass <QString> &asset_list,
	QStandardItem* hparent
)
{
	// Loop through all the children of this node
	for(int i = 0; i < hparent->rowCount(); i++) {		
		QStandardItem* htree_item = hparent->child(i);

		// Determine if this is an asset type we want to add to the list
		AssetInfoClass *asset_info = (AssetInfoClass *)htree_item->data().toULongLong();
		if ((asset_info != NULL) &&
			 (asset_info->Get_Type () != TypeAnimation) &&
			 (asset_info->Get_Type () != TypeCompressedAnimation) &&
			 (asset_info->Get_Type () != TypeMaterial))
		{
			asset_list.Add (htree_item->text());
		}

		// If this item has children, then add the children recursively
		if (htree_item->hasChildren ()) {
			Build_Render_Object_List (asset_list, htree_item);
		}
	}
	
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Create_Render_Obj_To_Display
//
////////////////////////////////////////////////////////////////////////////
RenderObjClass *
DataTreeModel::Create_Render_Obj_To_Display (QStandardItem* htree_item)
{
	// Lookup the information object associated with this asset
	RenderObjClass *render_obj = NULL;
	AssetInfoClass *asset_info = (AssetInfoClass *)htree_item->data().toULongLong();
	if (asset_info != NULL) {
		
		// Use the asset's instance if there is one, otherwise attempt to create one
		render_obj = asset_info->Get_Render_Obj ();
		if (render_obj == NULL) {

			// If this is a texture, then create a special BMP obj from it
			if (asset_info->Get_Type () == TypeMaterial) {
				TextureClass *ptexture = (TextureClass *)asset_info->Get_User_Number ();
				if (ptexture != NULL) {
					render_obj = new Bitmap2DObjClass (ptexture, 0.5F, 0.5F, true, false, false, true);
				}
			}

			//
			// Finally, if we aren't successful, create a new instance based on its name
			//
			if (render_obj == NULL) {
				render_obj = WW3DAssetManager::Get_Instance()->Create_Render_Obj(asset_info->Get_Name().toUtf8().constData());
			}
		}
	}

	//
	//	Force the highest level LOD
	//
	if (	render_obj != NULL &&
			::GetCurrentDocument ()->GetScene ()->Are_LODs_Switching () == false)
	{
		Set_Highest_LOD (render_obj);
	}
	
	return render_obj;
}


////////////////////////////////////////////////////////////////////////////
//
//  Refresh_Asset
//
void
DataTreeModel::Refresh_Asset
(
	const char* new_name,
	const char* old_name,
	ASSET_TYPE type
)
{
	// Params OK?
	if ((new_name != NULL) && (old_name != NULL)) {

		// Turn off repainting
		// GetTreeCtrl ().SetRedraw (FALSE);
		
		// Determime where this asset should go
		QStandardItem* hparent = NULL;
		QIcon icon;
		Determine_Tree_Location (type, hparent, icon);

		// Can we find the item we are supposed to refresh?
		QStandardItem* htree_item = FindChildItem (hparent, old_name);
		if (htree_item != NULL) {

			// Refresh the item's text in the tree control
			htree_item->setText (new_name);

			// Refresh the associated asset info structure
			AssetInfoClass *asset_info = (AssetInfoClass *)htree_item->data().toULongLong();
			if (asset_info != NULL) {
				asset_info->Set_Name (new_name);
			}		
		} else {

			// This asset wasn't already in the tree, so add it...
			Add_Asset_To_Tree (new_name, type);
		}

		// Turn painting back on
		// GetTreeCtrl ().SetRedraw (TRUE);

		// Force the window to be repainted
		// Invalidate (FALSE);
		// UpdateWindow ();	
	}

	return ;
}

////////////////////////////////////////////////////////////////////////////
//
//  Reload_Lightmap_Models
//
////////////////////////////////////////////////////////////////////////////
void
DataTreeModel::Reload_Lightmap_Models (void)
{
	Free_Child_Models (m_hMeshCollectionRoot);
	Free_Child_Models (m_hHierarchyRoot);
	Free_Child_Models (m_hMeshRoot);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Free_Child_Models
//
////////////////////////////////////////////////////////////////////////////
void
DataTreeModel::Free_Child_Models (QStandardItem* parent_item)
{
	//
	// Loop through all the children of this node
	//
	while (parent_item->rowCount())
	{
		QStandardItem *child_item = parent_item->takeChild(0);
		//
		// Get the data associated with this item
		//
		AssetInfoClass *asset_info = (AssetInfoClass *)child_item->data().toULongLong();
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Set_Highest_LOD
//
////////////////////////////////////////////////////////////////////////////
void
Set_Highest_LOD (RenderObjClass *render_obj) 
{
	if (render_obj != NULL) {
		for (int index = 0; index < render_obj->Get_Num_Sub_Objects (); index ++) {
			RenderObjClass *sub_obj = render_obj->Get_Sub_Object (index);
			if (sub_obj != NULL) {
				Set_Highest_LOD (sub_obj);
			}
			MEMBER_RELEASE (sub_obj);
		}

		//
		// Switcht this LOD to its highest level
		//
		if (render_obj->Class_ID () == RenderObjClass::CLASSID_HLOD) {
			((HLodClass *)render_obj)->Set_LOD_Level (((HLodClass *)render_obj)->Get_Lod_Count () - 1);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Restrict_Anims
//
////////////////////////////////////////////////////////////////////////////
void
DataTreeModel::Restrict_Anims (bool onoff)
{
	if (m_RestrictAnims != onoff) {
		m_RestrictAnims = onoff;
		
		//
		//	Reload the tree
		//
		Free_Child_Models(invisibleRootItem());
		CreateRootNodes ();
		LoadAssetsIntoTree ();
	}

	return ;
}

