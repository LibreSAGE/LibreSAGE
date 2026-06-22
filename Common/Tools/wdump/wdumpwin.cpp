#include "wdumpwin.h"
#include "ui_wdumpwin.h"
#include <QAction>
#include <QFileDialog>
#include <QIcon>
#include <QFileInfo>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QSettings>
#include <QCoreApplication>
#include <QStatusBar>
#include <QStyle>

WDumpWindow::WDumpWindow(QWidget *parent) : QMainWindow(parent)
{
    m_ui = new Ui::WDumpWindow();
    m_ui->setupUi(this);

    setWindowIcon(QIcon(":/wdump.ico"));

    // Give the toolbar actions icons (the MFC port used a toolbar bitmap; we
    // borrow the platform's standard icons here).
    m_ui->actionOpen->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    m_ui->actionAbout->setIcon(style()->standardIcon(QStyle::SP_MessageBoxInformation));

    connect(m_ui->actionOpen, &QAction::triggered, this, &WDumpWindow::OnFileOpen);
    connect(m_ui->actionAbout, &QAction::triggered, this, &WDumpWindow::OnAbout);
    connect(m_ui->actionExit, &QAction::triggered, this, &WDumpWindow::OnExit);

    statusBar()->showMessage(tr("Ready"));

    // Insert the recent-files entries into the File menu, ahead of the Exit item.
    m_recentFileSeparator = m_ui->menuFile->insertSeparator(m_ui->actionExit);
    for (int i = 0; i < MaxRecentFiles; ++i)
    {
        m_recentFileActions[i] = new QAction(this);
        m_recentFileActions[i]->setVisible(false);
        connect(m_recentFileActions[i], &QAction::triggered, this, &WDumpWindow::OnOpenRecentFile);
        m_ui->menuFile->insertAction(m_recentFileSeparator, m_recentFileActions[i]);
    }
    UpdateRecentFileActions();
}

WDumpWindow::~WDumpWindow()
{
    delete m_ui;
}

void WDumpWindow::OnFileOpen()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open File"), "",
                                                    tr("Westwood 3D Files (*.w3d, *.W3D);;All Files (*)"));
    LoadFile(fileName);
}

void WDumpWindow::OnOpenRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action != nullptr)
    {
        LoadFile(action->data().toString());
    }
}

void WDumpWindow::LoadFile(const QString &fileName)
{
    if (fileName.isEmpty())
    {
        return;
    }

    if(!m_chunkData.Load(fileName.toStdString().c_str()))
    {
        fprintf(stderr,"Failed to load file %s\n", fileName.toStdString().c_str());
        statusBar()->showMessage(tr("Failed to load %1").arg(QFileInfo(fileName).fileName()));
        return;
    }
    m_ui->treeView->setModel(new ChunkDataModel(&m_chunkData, this));
    if (m_ui->treeView->selectionModel() != nullptr)
    {
        connect(m_ui->treeView->selectionModel(),
                &QItemSelectionModel::selectionChanged,
                this,
                &WDumpWindow::OnTreeSelectionChanged);
    }
    AddToRecentFiles(fileName);
    statusBar()->showMessage(tr("Loaded %1").arg(QFileInfo(fileName).fileName()));
}

void WDumpWindow::AddToRecentFiles(const QString &fileName)
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
    UpdateRecentFileActions();
}

void WDumpWindow::UpdateRecentFileActions()
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

void WDumpWindow::OnAbout()
{
    QMessageBox about(this);
    about.setWindowTitle(tr("About wdump"));
    about.setIconPixmap(QIcon(":/wdump.ico").pixmap(48, 48));
    about.setTextFormat(Qt::RichText);
    about.setText(tr("<h3>Westwood 3D File Viewer v%1</h3>"
                     "<p>A tool to inspect Westwood 3D (.w3d) files.</p>"
                     "<p>Copyright &copy; 1997 Westwood Studios<br>"
                     "Copyright &copy; 2025 Electronic Arts Inc.<br>"
                     "Copyright &copy; 2026 Stephan Vedder</p>"
                     "<p>Written by Eric Cosky, Greg Hjelstrom.<br>"
                     "Qt port by Stephan Vedder.</p>")
                      .arg(QCoreApplication::applicationVersion()));
    about.exec();
}

void WDumpWindow::OnExit()
{
    close();
}

void WDumpWindow::OnTreeSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);

    if (selected.indexes().isEmpty())
    {
        return;
    }

    // Get the selected ChunkItem from the model
    QModelIndex index = selected.indexes().first();
    ChunkItem *item = static_cast<ChunkItem *>(index.internalPointer());
    if (item == nullptr || item->Type == nullptr || item->Type->Callback == nullptr)
    {
        return;
    }
    ChunkModel *model = new ChunkModel(this);
    item->Type->Callback(item, model);
    m_ui->tableView->setModel(model);
}
