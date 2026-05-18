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

// W3DView.cpp : Defines the class behaviors for the application.
//

#include "w3dview.h"
#include "globals.h"
#include "viewerassetmgr.h"
#include "w3dviewwin.h"
#include "w3dviewdoc.h"
#include "wwmath.h"
#include <QCommandLineParser>

W3DViewApp::W3DViewApp(int &argc, char **argv) : QApplication(argc, argv)
{
    setOrganizationName("WWVegas");
    setApplicationName("W3DView");
    setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("A UI tool to inspect Westwood 3D files");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.process(*this);

    //
    //  Create our document
    //
    m_currentDocument = new W3DViewDoc;

    //
    //	Initialize the libraries
    //
    WWMath::Init ();

    //
    //	Allocate an asset manager
    //
    _TheAssetMgr = new ViewerAssetMgrClass;

}

int main(int argc, char **argv)
{
    W3DViewApp app(argc, argv);

    W3DViewWindow window;
    window.show();

    return app.exec();
}