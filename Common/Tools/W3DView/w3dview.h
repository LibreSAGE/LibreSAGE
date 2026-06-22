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

// W3DView.h : main header file for the W3DVIEW application
//

#pragma once
#include <QApplication>

class GraphicView;
class W3DViewDoc;
class W3DViewWindow;
class W3DViewApp : public QApplication
{
    friend class GraphicView;
    friend class W3DViewWindow;
    Q_OBJECT
public:
    W3DViewApp(int &argc, char **argv);
    W3DViewDoc *GetCurrentDocument() { return m_currentDocument; }
    GraphicView *GetGraphicView() { return m_graphicView; }
    W3DViewWindow *GetMainWindow() { return m_mainWindow; }
protected:
    W3DViewDoc *m_currentDocument = NULL;
    W3DViewWindow *m_mainWindow = NULL;
    GraphicView *m_graphicView = NULL;
};