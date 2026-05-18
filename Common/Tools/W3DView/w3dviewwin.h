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
#include "datatreemodel.h"

#include <QMainWindow>
#include <QItemSelection>

namespace Ui
{
class W3DViewWindow;
}

class W3DViewDoc;
class W3DViewWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit W3DViewWindow(QWidget *parent = nullptr);
    ~W3DViewWindow() override;
    
private slots:
    void OnFileOpen();
    void OnExit();
    void OnTreeSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

public:
	
	//////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////
	void	Select_Device (bool show_dlg = true);

private:
    void InitializeRenderer();

    Ui::W3DViewWindow *m_ui = NULL;
	W3DViewDoc *m_doc = NULL;
	DataTreeModel m_dataTreeModel;
    bool m_rendererInitialized = false;
};
