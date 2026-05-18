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

#include "w3dviewwin.h"
#include "graphicview.h"
#include "globals.h"
#include "viewerassetmgr.h"
#include "w3dviewdoc.h"
#include "utils.h"

#include "distlod.h"
#include "part_ldr.h"
#include "agg_def.h"
#include "part_emt.h"
#include "ringobj.h"
#include "sphereobj.h"
#include "hlod.h"
#include "dx8wrapper.h"

#include "ww3d.h"
#include "wwdebug.h"

// Extra includes
#include <QFileDialog>
#include <QItemSelectionModel>
#include <QSettings>
#include "ui_w3dviewwin.h"

W3DViewWindow::W3DViewWindow(QWidget *parent) : QMainWindow(parent)
{
    m_ui = new Ui::W3DViewWindow();
    m_ui->setupUi(this);
    m_ui->treeView->setModel(&m_dataTreeModel);

    connect(m_ui->treeView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &W3DViewWindow::OnTreeSelectionChanged);
    connect(m_ui->actionOpen, &QAction::triggered, this, &W3DViewWindow::OnFileOpen);
    connect(m_ui->actionExit, &QAction::triggered, this, &W3DViewWindow::OnExit);

    // Set DXVK_WSI_DRIVER env variable to SDL3
    setenv("DXVK_WSI_DRIVER", "SDL3", 1);

    InitializeRenderer();
}

void W3DViewWindow::InitializeRenderer()
{
    if (m_rendererInitialized)
    {
        return;
    }

    // Initialize the WW3D engine using the window handle from
    // the graphic viewer class.
    SDL_Window* win = m_ui->graphicView->GetSDLWindow();
    WWASSERT(win != nullptr);

    BOOL bReturn = (WW3D::Init (win) == WW3D_ERROR_OK);
    assert (bReturn);
    WW3D::Enable_Static_Sort_Lists(true);

    //
    //	Initialize the device
    //
    g_iWidth	= 640;
    g_iHeight	= 480;					 
    Select_Device (false);

    //
    // Register the prototype loaders we'll need
    //
    WW3DAssetManager::Get_Instance()->Register_Prototype_Loader (&_HLodLoader);
    WW3DAssetManager::Get_Instance()->Register_Prototype_Loader (&_DistLODLoader);
    WW3DAssetManager::Get_Instance()->Register_Prototype_Loader (&_ParticleEmitterLoader);
    WW3DAssetManager::Get_Instance()->Register_Prototype_Loader (&_AggregateLoader);
    WW3DAssetManager::Get_Instance()->Register_Prototype_Loader (&_RingLoader);
    WW3DAssetManager::Get_Instance()->Register_Prototype_Loader (&_SphereLoader);
    // WW3DAssetManager::Get_Instance()->Register_Prototype_Loader (&_SoundRenderObjLoader);

    //
    // Restore the N-Patch Subdivision Level and Gap-Filling settings from the last session.
    //
    QSettings settings(QSettings::Scope::UserScope);
    int subdivision_level	= settings.value("NPatchesSubdivision", 4).toInt();
    int gap_filling			= settings.value("NPatchesGapFilling", 0).toInt();
    WW3D::Set_NPatches_Gap_Filling_Mode(gap_filling ? WW3D::NPATCHES_GAP_FILLING_ENABLED : WW3D::NPATCHES_GAP_FILLING_DISABLED);
    WW3D::Set_NPatches_Level((unsigned int)subdivision_level);

    //
    // Restore munge sort on load settings
    //
    int munge_sort=settings.value("MungeSortOnLoad",0).toInt();
    WW3D::Enable_Munge_Sort_On_Load(munge_sort==1?true:false);
    
    int sort=settings.value("EnableSorting",1).toInt();
    WW3D::Enable_Sorting(sort==1?true:false);

    // restore gamma settings
    int setting=settings.value("EnableGamma",0).toInt();
    if (setting) {
        float gamma=settings.value("Gamma",10).toFloat();
        gamma=gamma/10.0f;
        if (gamma<1.0) gamma=1.0;
        if (gamma>3.0) gamma=3.0;
        DX8Wrapper::Set_Gamma(gamma,0.0f,1.0f);
    }

    m_rendererInitialized = true;
}

void W3DViewWindow::Select_Device(bool show_dialog)
{
    m_ui->graphicView->InitializeGraphicView();
}


W3DViewWindow::~W3DViewWindow()
{
    delete m_ui;
}

void W3DViewWindow::OnFileOpen()
{
    W3DViewDoc *doc = GetCurrentDocument ();
	if (doc == NULL) {
		return ;
	}

    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open File"), "",
                                                    tr("Westwood 3D Files (*.w3d, *.W3D);;All Files (*)"));

    doc->LoadAssetsFromFile (fileName.toUtf8().constData());
    m_dataTreeModel.LoadAssetsIntoTree();
}

void W3DViewWindow::OnExit()
{
    close();
}

void W3DViewWindow::OnTreeSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);

    if (selected.indexes().isEmpty())
    {
        return;
    }

    // Get the selected ChunkItem from the model
    QModelIndex index = selected.indexes().first();
    QStandardItem *item = m_dataTreeModel.itemFromIndex(index);
    m_dataTreeModel.Display_Asset(item);
    m_ui->graphicView->update();
}