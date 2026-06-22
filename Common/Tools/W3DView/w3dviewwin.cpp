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
#include "w3dview.h"
#include "w3dviewdoc.h"
#include "animationtoolbar.h"
#include "utils.h"

#include "distlod.h"
#include "part_ldr.h"
#include "agg_def.h"
#include "part_emt.h"
#include "ringobj.h"
#include "sphereobj.h"
#include "hlod.h"
#include "dx8wrapper.h"
#include "rendobj.h"
#include "hanim.h"

#include "ww3d.h"
#include "wwdebug.h"

// Extra includes
#include <QFileDialog>
#include <QFileInfo>
#include <QAction>
#include <QItemSelectionModel>
#include <QSettings>
#include <QTimer>
#include <QLabel>
#include <QStatusBar>
#include <QMessageBox>
#include <QIcon>
#include "ui_w3dviewwin.h"

// Status-bar refresh cadence (ms); also used to derive render FPS.
static const int kStatusIntervalMs = 500;

//
//  Count particle emitters within a render object (best-effort stand-in for
//  the original active-particle counter, which lived in W3DViewDoc).
//
static int Count_Emitters(RenderObjClass *obj)
{
    if (obj == NULL) {
        return 0;
    }

    int count = (obj->Class_ID() == RenderObjClass::CLASSID_PARTICLEEMITTER) ? 1 : 0;
    for (int index = 0; index < obj->Get_Num_Sub_Objects(); ++index) {
        RenderObjClass *sub = obj->Get_Sub_Object(index);
        if (sub != NULL) {
            count += Count_Emitters(sub);
            sub->Release_Ref();
        }
    }
    return count;
}

W3DViewWindow::W3DViewWindow(QWidget *parent) : QMainWindow(parent)
{
    // Set the global w3dviewwindow pointer to this instance
	W3DViewApp *pApp = qobject_cast<W3DViewApp *>(QApplication::instance());
	WWASSERT(pApp);
	pApp->m_mainWindow = this;

    m_ui = new Ui::W3DViewWindow();
    m_ui->setupUi(this);
    m_ui->treeView->setModel(&m_dataTreeModel);

    m_ui->splitter->setStretchFactor(0, 1);   // tree
    m_ui->splitter->setStretchFactor(1, 3);   // graphic view
    m_ui->splitter->setSizes({ 256, 768 });

    connect(m_ui->treeView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &W3DViewWindow::OnTreeSelectionChanged);
    connect(m_ui->actionOpen, &QAction::triggered, this, &W3DViewWindow::OnFileOpen);
    connect(m_ui->actionExit, &QAction::triggered, this, &W3DViewWindow::OnExit);
    connect(m_ui->actionAbout, &QAction::triggered, this, &W3DViewWindow::OnAbout);

    // Recently-used file entries in the File menu, just ahead of Exit.
    m_recentFileSeparator = m_ui->menuFile->insertSeparator(m_ui->actionExit);
    for (int i = 0; i < MaxRecentFiles; ++i)
    {
        m_recentFileActions[i] = new QAction(this);
        m_recentFileActions[i]->setVisible(false);
        connect(m_recentFileActions[i], &QAction::triggered, this, &W3DViewWindow::OnOpenRecentFile);
        m_ui->menuFile->insertAction(m_recentFileSeparator, m_recentFileActions[i]);
    }
    UpdateRecentFileListActions();

    // Animation transport toolbar (the Qt equivalent of Renegade's
    // m_animationToolbar), docked at the bottom and toggled from
    // View > Toolbars > Animation.
    m_animationToolBar = new AnimationToolBar(this);
    addToolBar(Qt::BottomToolBarArea, m_animationToolBar);
    m_ui->actionToolbarAnimation->setCheckable(true);
    m_ui->actionToolbarAnimation->setChecked(true);
    connect(m_ui->actionToolbarAnimation, &QAction::toggled,
            m_animationToolBar, &QWidget::setVisible);
    connect(m_animationToolBar, &AnimationToolBar::Closed, this, [this]() {
        m_ui->actionToolbarAnimation->setChecked(false);
    });

    CreateStatusBar();

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

////////////////////////////////////////////////////////////////////////////
//
//  Update_Frame_Time
//
////////////////////////////////////////////////////////////////////////////
void
W3DViewWindow::Update_Frame_Time (DWORD clocks)
{
	static DWORD frames = 0;
	static DWORD total_clocks = 0;
	static DWORD last_update = 0;

	total_clocks += clocks;
	frames ++;

	//if (frames >= 20) {
	if ((::GetTickCount () - last_update) >= 1000) {
		
		//
		//	Average the frame time
		//
		float frame_time = ((float) total_clocks) / ((float) frames);
		QString text;
		text = QString("Clocks: %1").arg(frame_time);

		//
		//	Update the UI
		//
		m_statusFps->setText(text);

		frames = 0;
		total_clocks = 0;
		last_update = ::GetTickCount ();
	}
			
	// Update the resolution display
	GraphicView *pCGraphicView = m_ui->graphicView;
	if (pCGraphicView != NULL) {

		QRect bounds = pCGraphicView->geometry();

		QString text;
		text = QString(" %1 x %2 ").arg(bounds.width()).arg(bounds.height());

		m_statusResolution->setText(text);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  UpdatePolygonCount
//
////////////////////////////////////////////////////////////////////////////
void
W3DViewWindow::UpdatePolygonCount (int iPolygons)
{
    QString stringPolyCount;
    stringPolyCount = QString("Polys %1").arg(iPolygons);

    m_statusPolys->setText(stringPolyCount);
    return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Update_Particle_Count
//
////////////////////////////////////////////////////////////////////////////
void
W3DViewWindow::Update_Particle_Count (int particles)
{
    QString count_string;
    count_string = QString("Particles %1").arg(particles);

    m_statusParticles->setText(count_string);
    return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  UpdateFrameCount
//
////////////////////////////////////////////////////////////////////////////
void
W3DViewWindow::UpdateFrameCount
(
    int		iCurrentFrame,
    int		iTotalFrames,
	 float	frame_rate
)
{
    QString frames;
    frames = QString("Frame %1/%2 at %3 fps").arg(iCurrentFrame).arg(iTotalFrames).arg(frame_rate, 0, 'f', 2);

    m_statusFrame->setText(QString::fromUtf8(frames.toUtf8()));
    return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  UpdateCameraDistance
//
////////////////////////////////////////////////////////////////////////////
void
W3DViewWindow::UpdateCameraDistance (float cameraDistance)
{
    QString distance_string;
    distance_string = QString("Camera %1").arg(cameraDistance, 0, 'f', 3);

    m_statusCamera->setText(QString::fromUtf8(distance_string.toUtf8()));
    return ;
}



void W3DViewWindow::OnFileOpen()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open File"), "",
                                                    tr("Westwood 3D Files (*.w3d, *.W3D);;All Files (*)"));
    LoadFile(fileName);
}

void W3DViewWindow::OnOpenRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action != nullptr)
    {
        LoadFile(action->data().toString());
    }
}

void W3DViewWindow::LoadFile(const QString &fileName)
{
    if (fileName.isEmpty())
    {
        return;
    }

    W3DViewDoc *doc = GetCurrentDocument();
    if (doc == NULL)
    {
        return;
    }

    doc->LoadAssetsFromFile(fileName.toUtf8().constData());
    m_dataTreeModel.LoadAssetsIntoTree();
    AddToRecentFileList(fileName);
}

void W3DViewWindow::AddToRecentFileList(const QString &fileName)
{
    QSettings settings;
    QStringList files = settings.value("recentFiles").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles)
    {
        files.removeLast();
    }
    settings.setValue("recentFiles", files);
    UpdateRecentFileListActions();
}

void W3DViewWindow::UpdateRecentFileListActions()
{
    QSettings settings;
    QStringList files = settings.value("recentFiles").toStringList();
    const int count = qMin(files.size(), static_cast<int>(MaxRecentFiles));

    for (int i = 0; i < count; ++i)
    {
        const QString text = QString("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());
        m_recentFileActions[i]->setText(text);
        m_recentFileActions[i]->setData(files[i]);
        m_recentFileActions[i]->setToolTip(files[i]);
        m_recentFileActions[i]->setVisible(true);
    }
    for (int i = count; i < MaxRecentFiles; ++i)
    {
        m_recentFileActions[i]->setVisible(false);
    }
    m_recentFileSeparator->setVisible(count > 0);
}

void W3DViewWindow::OnExit()
{
    close();
}

void W3DViewWindow::OnAbout()
{
    QMessageBox about(this);
    about.setWindowTitle(tr("About W3DView"));
    about.setIconPixmap(QIcon(":/icons/W3DView.ico").pixmap(48, 48));
    about.setTextFormat(Qt::RichText);
    about.setText(tr("<h3>Westwood 3D View v%1</h3>"
                     "<p>A tool to view and inspect Westwood 3D (.w3d) files.</p>"
                     "<p>Copyright &copy; 1998 Westwood Studios<br>"
                     "Copyright &copy; 2025 Electronic Arts Inc.<br>"
                     "Copyright &copy; 2026 Stephan Vedder</p>"
                     "<p>Written by Patrick Smith, Greg Hjelstrom.<br>"
                     "Qt port by Stephan Vedder.</p>")
                      .arg(QApplication::applicationVersion()));
    about.exec();
}

void W3DViewWindow::CreateStatusBar()
{
    // Panes mirroring Renegade's m_wndStatusBar (right-aligned), plus the
    // default message area on the left.
    m_statusPolys      = new QLabel(this);
    m_statusParticles  = new QLabel(this);
    m_statusCamera     = new QLabel(this);
    m_statusFrame      = new QLabel(this);
    m_statusFps        = new QLabel(this);
    m_statusResolution = new QLabel(this);

    m_statusFrame->setMinimumWidth(150);
    m_statusCamera->setMinimumWidth(90);

    statusBar()->addPermanentWidget(m_statusPolys);
    statusBar()->addPermanentWidget(m_statusParticles);
    statusBar()->addPermanentWidget(m_statusCamera);
    statusBar()->addPermanentWidget(m_statusFrame);
    statusBar()->addPermanentWidget(m_statusFps);
    statusBar()->addPermanentWidget(m_statusResolution);
    statusBar()->showMessage(tr("Ready"));
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